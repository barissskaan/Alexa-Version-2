// Protocol: I2S2 (PC3=SD, PB10=CK, PB12=WS)
// Sample Rate: 16 kHz

#include "audio_processing.h"
#include "led_array.h"
#include <stdio.h>
#include <cmath>
#include <cstring>

/* External I2S handle (defined in main.c) */
extern I2S_HandleTypeDef hi2s2;

// BUFFERS

// DMA input buffer
static uint16_t inputBuffer1[I2S_BUF_SIZE * 2];

// Merged audio frame (after combining 16-bit pairs)
static int32_t mergedFrame[I2S_BUF_SIZE / 4];
// Dass wir durch 4 teilen, liegt an zwei Faktoren, die gleichzeitig passieren: 
// der Umwandlung von Datengrößen (16-Bit zu 32-Bit) -> durch 2 
// und der Umwandlung von Kanälen (Stereo zu Mono) -> wieder durch 2. TOTAL DURCH 4!

// 1 second recording buffer (16000 samples @ 16kHz)
static int32_t ISecArray[16000];

// Ring buffer to capture audio BEFORE threshold is exceeded
static int32_t RingBuffer[4000];
static int RingBufferSize = 4000;
static int RingBufferIndex = 0;

//STATE VARIABLES
static volatile bool isRecording = false;
static volatile bool datenVerarbeiten = false;  // Data processing enabled flag
static volatile int iZaehler = 0;               // Recording sample counter
static volatile uint32_t recordingStartTime = 0;
static volatile bool recordingComplete = false; // Flag to signal main loop
static volatile uint32_t cooldownEndTime = 0;   // Cooldown timer (no blocking delay)

// THRESHOLD VARIABLES
static int noise = NOISE_THRESHOLD;
static int OFFSET = INITIAL_OFFSET;

// Process audio frame and check for threshold
/**
 * @warning
 * _OFFSET: Das ist das neue Ergebnis. Hier wird der Schwellenwert gespeichert, den wir für den nächsten Aufzeichnungsschritt benutzen wollen. 
 * 0.113: Das ist der Reaktions-Faktor. Er bestimmt, wie stark ein neuer Ton den Schwellenwert verändern darf (hier ca. 11 %).
 * mergedFrame[i]: Das ist der aktuelle Ton-Wert, den wir gerade frisch vom Mikrofon bekommen haben.
 * 0.9: Das ist der Gedächtnis-Faktor. Er sagt: "Behalte 90 % von dem, was du vorher schon über die Lautstärke im Raum wusstest."
 * OFFSET (rechts): Das ist der alte Schwellenwert aus dem vorherigen Schritt.
 * @note Warum 0.113 und 0.9?
 * Durch die 0.9 hat das System ein Gedächtnis. Es bleibt ruhig und ändert sich nur, wenn es im Raum dauerhaft lauter wird.
 * In der Mathematik für Filter gilt oft die Regel: Die beiden Faktoren sollten zusammen ungefähr 1 ergeben ($0.9 + 0.113 \approx 1$).
 */
void Audio1Sec(void) {
    // Das ist ein „Lautstärke-Zähler“. Er zählt, wie viele Aufzeichnungen hintereinander laut waren. Wir starten bei 0.
    int loudSoundCounter = 0;
    // Das ist unser Filter. Ein kurzes „Knacksen“ (çatırtı) dauert vielleicht nur 1 oder 2 Aufzeichnungen. 
    // Wir sagen: Erst wenn es 5 Mal hintereinander laut ist, glauben wir, dass es ein echtes Wort ist.
    const int LOUD_SOUND_DURATION = 5;  // Samples needed to trigger recording

    // Wie ich schon sagte, enthält der Speicher 1000 Plätze, aber wegen des Mono/Stereo-Sprungs (i+=4) 
    // sind nur 250 davon echte Ton-Werte. Also läuft diese Schleife 250 Mal
    for (int i = 0; i < I2S_BUF_SIZE / 4; i++) {
        // Schallwerte können positiv oder negativ sein. Wir nehmen den Betrag (abs) um sie positiv zu machen.
        OFFSET = 0.113 * abs(mergedFrame[i]) + 0.9 * OFFSET;
        // Filter EMA (Exponential Moving Average) -> Adaptiven Schwellwerts -> NICHT STATISCH SONDERN DYNAMISCH! BESSER ZUM FILTERN VON NOISE! UND BESSERE LÖSUNG ALS DIE VORGESCHLAGENE STATISCHE LÖSUNG IM PDF!


        // Check if signal exceeds threshold
        if (abs(mergedFrame[i]) > OFFSET) {
            loudSoundCounter++;
        } else {
            loudSoundCounter = 0;
        }

        // Filter out extreme noise spikes
        if (abs(mergedFrame[i]) > noise) {
            continue;  // Skip this sample
        }

        // Store in ring buffer (captures audio BEFORE trigger)
        // this was actually not really necessary, because we could just use the mergedFrame array directly
        // But i saw that in Aufgabe 2 they wanted us to save the audio before the trigger to give it to AI to not miss the trigger word!
        RingBuffer[RingBufferIndex] = mergedFrame[i];
        RingBufferIndex = (RingBufferIndex + 1) % RingBufferSize;

        if (loudSoundCounter >= LOUD_SOUND_DURATION && 
            abs(mergedFrame[i]) > OFFSET && 
            !isRecording &&
            HAL_GetTick() > cooldownEndTime) {
            
            isRecording = true;
            recordingStartTime = HAL_GetTick();
            printf(">>> Aufnahme beginnt\r\n");

            // Copy ring buffer content (audio before trigger)
            int copyIndex = RingBufferIndex;
            for (int k = 0; k < RingBufferSize && iZaehler < 16000; k++) {
                ISecArray[iZaehler++] = RingBuffer[copyIndex];
                copyIndex = (copyIndex + 1) % RingBufferSize;
            }
        }

        // Continue recording
        if (isRecording) {
            ISecArray[iZaehler++] = mergedFrame[i];

            if (iZaehler >= 16000) {
                // Set flag for main loop to handle (NO blocking delay in ISR!)
                recordingComplete = true;
                isRecording = false;
                
                // Set cooldown time (1 second from now)
                cooldownEndTime = HAL_GetTick() + 1000;
                
                break;
            }
        }
    }
}

/* 
 * PUBLIC FUNCTIONS
 */

void AudioProcessing_Init(void) {
    // Clear all buffers
    memset(inputBuffer1, 0, sizeof(inputBuffer1));
    memset(mergedFrame, 0, sizeof(mergedFrame));
    memset(ISecArray, 0, sizeof(ISecArray));
    memset(RingBuffer, 0, sizeof(RingBuffer));
    
    // Reset state
    isRecording = false;
    datenVerarbeiten = false;
    iZaehler = 0;
    recordingStartTime = 0;
    recordingComplete = false;
    cooldownEndTime = 0;
    
    // Reset thresholds
    noise = NOISE_THRESHOLD;
    OFFSET = INITIAL_OFFSET;
}

void AudioProcessing_Enable(bool enable) {
    datenVerarbeiten = enable;
}

void LautstaerkeZeigen(void) {
    for (int i = 0; i < I2S_BUF_SIZE / 4; i++) {
        // Scale sample to LED count (0-10)
        // the divisor is adjusted based on our microphone sensitivity
        // I had to try out different values to get the best result
        int anzahl = abs(mergedFrame[i] / 550) - 12;
        if (anzahl < 0) anzahl = 0;
        if (anzahl > 10) anzahl = 10;
        led_func(anzahl);
    }
}

int mittelwert(void) {
    int volume = 0;

    // Calculate average absolute amplitude
    for (int i = 0; i < I2S_BUF_SIZE / 4; i++) {
        volume += abs(mergedFrame[i]);
    }
    volume /= (I2S_BUF_SIZE / 4);
    
    return volume;
}

bool AudioProcessing_IsRecordingComplete(void) {
    return recordingComplete;
}

uint32_t AudioProcessing_GetRecordingStartTime(void) {
    return recordingStartTime;
}

int32_t* AudioProcessing_GetRecordedData(void) {
    return ISecArray;
}

void AudioProcessing_ClearRecordingComplete(void) {
    recordingComplete = false;
}

void AudioProcessing_ResetRecording(void) {
    memset(RingBuffer, 0, sizeof(RingBuffer));
    RingBufferIndex = 0;
    iZaehler = 0;
}

uint16_t* AudioProcessing_GetInputBuffer(void) {
    return inputBuffer1;
}

/**
 * DMA CALLBACKS (Direct Memory Access): This callback function is called by the HAL Library when the half of DMA transfer is complete.
 * @brief  Process first half of buffer while the second half is being transferred. This is done in parallel.
 * that is why the audio processing is done without any interruption or data loss!
 * @warning Wir verwenden ein Mono-Mikrofon. Aber das Protokoll ist Stereo. (Sterep-Protokoll)
 * index 0: Linker Kanal (Teil 1): Die ersten 16 Bit deiner Stimme.
 * index 1: Linker Kanal (Teil 2): Die zweiten 16 Bit deiner Stimme.
 * index 2: Rechter Kanal (Teil 1): Leer / 0 (weil das Mikrofon Mono ist).
 * index 3: Rechter Kanal (Teil 2): Leer / 0 (weil das Mikrofon Mono ist).
 */
extern "C" void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef* hi2s) {
    // if (hi2s->Instance != SPI2) return;
    
    if (!datenVerarbeiten) {
        // Warmup phase - discard data
        // THE PDF SAYS: "There may be problematic and wrong data at the beginning of the recording!"
        // SO WE DISCARD THE DATA!
        return;
    }

    // Data goes into the RAM 16 bit values, we need to merge them to 32 bit values!
    // Because a real voice value is 18 bit! So it can be represented by 32 bit!
    // After merging we shift right 14 bits to get the 18-bit sample!  
    for (int i = 0, j = 0; i < I2S_BUF_SIZE; i += 4, j++) {
        // Merge two 16-bit values into 32-bit
        int32_t mergedValue = ((int32_t)inputBuffer1[i] << 16) | inputBuffer1[i + 1];
        // Shift right 14 bits to get 18-bit sample
        mergedValue = mergedValue >> 14;
        mergedFrame[j] = mergedValue;
    }

    Audio1Sec();
}
// called when the whole DMA transfer is complete
extern "C" void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef* hi2s) {
    // if (hi2s->Instance != SPI2) return;
    
    if (!datenVerarbeiten) {
        return;
    }

    for (int i = I2S_BUF_SIZE, j = 0; i < I2S_BUF_SIZE * 2; i += 4, j++) {
        int32_t mergedValue = ((int32_t)inputBuffer1[i] << 16) | inputBuffer1[i + 1];
        mergedValue = mergedValue >> 14;
        mergedFrame[j] = mergedValue;
    }

    Audio1Sec();
}

