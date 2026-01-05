#include "my_main.h"
#include "main.h"
#include "led_array.h"
#include "timer.h"
#include "transmit.h"
#include "audio_processing.h"
#include <stdio.h>
#include <cmath>
#include <cstring>

// EXTERNAL HAL HANDLES
extern TIM_HandleTypeDef htim4;    // Timer for microsecond delays
extern I2S_HandleTypeDef hi2s2;    // I2S for microphone input
extern UART_HandleTypeDef huart3;  // UART for printf output

 // PRINTF RETARGET
extern "C" int _write(int file, char* ptr, int len) {
    (void)file;
    HAL_UART_Transmit(&huart3, reinterpret_cast<uint8_t*>(ptr), len, HAL_MAX_DELAY);
    return len;
}

extern "C" void my_main(void) {
    // Visual confirmation that my_main is running
    uint16_t on  =  0b111111000010;
    uint16_t off = 0b111111000001;
    sendSequence(off);

    for(int i = 0; i < 3; i++) {
        led_func(10);  // All LEDs ON
        HAL_Delay(200);
        led_func(0);   // All LEDs OFF
        HAL_Delay(200);
    }
    
    printf("\r\n");
    printf("   DIY Alexa - Aufgabe 1\r\n");

    // Initialize audio processing module
    AudioProcessing_Init();

    // Start I2S DMA reception
    HAL_StatusTypeDef status = HAL_I2S_Receive_DMA(&hi2s2, AudioProcessing_GetInputBuffer(), I2S_BUF_SIZE);
    if (status != HAL_OK) {
        printf("[ERROR] I2S DMA start failed! Error: %d\r\n", status);
        while (1) {
        	led_func(10);
            HAL_Delay(200);
            led_func(0);
            HAL_Delay(200);
        }
    }
    printf("[INIT] I2S DMA started\r\n");

    // Warmup: Wait 2 seconds for microphone stabilization
    printf("\r\n[WARMUP] Waiting 2 seconds for microphone...\r\n");
    uint32_t warmupStart = HAL_GetTick();
    while (HAL_GetTick() - warmupStart < 2000) {
        // LED animation during warmup
        int pos = ((HAL_GetTick() / 100) % 20);
        if (pos >= 10) pos = 19 - pos;
        led_func(pos + 1);
    }
    led_func(0);
    printf("[WARMUP] Complete!\r\n");

    // Enable data processing
    AudioProcessing_Enable(true);
    printf("\r\n>>> Listening for audio...\r\n\r\n");

    // Main loop
    bool flag = false;

    while (1) {
        // Check if recording just completed (flag set by ISR)
        if (AudioProcessing_IsRecordingComplete()) {
            // Print recording info (safe to do in main loop)
            printf("\r\n>>> Aufnahme beendet. Dauer: %lu ms\r\n",
                   HAL_GetTick() - AudioProcessing_GetRecordingStartTime());
            printf("    Samples: 16000\r\n");
            
            // Toggle steckdose: ein/aus bei jedem Schwellwert-Überschreiten
            if(flag == false) {
                printf(">>> Steckdose EIN\r\n");
                sendSequence(on);
                flag = true;
            } else {
                printf(">>> Steckdose AUS\r\n");
                sendSequence(off);
                flag = false;
            }
            
            // Reset recording state
            AudioProcessing_ResetRecording();
            AudioProcessing_ClearRecordingComplete();

            printf("\r\n>>> Listening for audio...\r\n\r\n");
        }

        // Volume display
        LautstaerkeZeigen();
    }
}
