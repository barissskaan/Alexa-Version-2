#include "my_main.h"
#include "main.h"
#include "led_array.h"
#include "timer.h"
#include "transmit.h"
#include "audio_processing.h"
#include "keyword_spotting.h"
#include <stdio.h>
#include <cmath>
#include <cstring>
#include <string>

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
    printf("========================================\r\n");
    printf("   DIY Alexa - Keyword Spotting System\r\n");
    printf("========================================\r\n");

    // Initialize audio processing module
    AudioProcessing_Init();
    
    // Initialize keyword spotting system (MFCC + Neural Network)
    KeywordSpotting_Init();

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
    printf("\r\n>>> Listening for keywords (ON/OFF)...\r\n\r\n");

    // Main loop
    bool socketState = false;  // false = OFF, true = ON

    while (1) {
        // Check if recording just completed (flag set by ISR)
        if (AudioProcessing_IsRecordingComplete()) {
            // Print recording info (safe to do in main loop)
            printf("\r\n>>> Aufnahme beendet. Dauer: %lu ms\r\n",
                   HAL_GetTick() - AudioProcessing_GetRecordingStartTime());
            printf("    Samples: 16000\r\n");
            
            // Get recorded audio data
            int32_t* audioData = AudioProcessing_GetRecordedData();
            
            // Process audio with keyword spotting (MFCC + Neural Network)
            std::string detectedWord = KeywordSpotting_ProcessAudio(audioData);
            
            // Control socket based on detected keyword
            if (detectedWord == "ON") {
                if (!socketState) {
                    printf("\r\n>>> [ACTION] Turning Socket ON\r\n");
                    sendSequence(on);
                    socketState = true;
                    // Visual feedback
                    for (int i = 0; i < 2; i++) {
                        led_func(10);
                        HAL_Delay(100);
                        led_func(0);
                        HAL_Delay(100);
                    }
                } else {
                    printf("\r\n>>> Socket is already ON\r\n");
                }
            } 
            else if (detectedWord == "OFF") {
                if (socketState) {
                    printf("\r\n>>> [ACTION] Turning Socket OFF\r\n");
                    sendSequence(off);
                    socketState = false;
                    // Visual feedback
                    led_func(10);
                    HAL_Delay(500);
                    led_func(0);
                } else {
                    printf("\r\n>>> Socket is already OFF\r\n");
                }
            }
            else {
                printf("\r\n>>> No action taken for: %s\r\n", detectedWord.c_str());
            }
            
            // Reset recording state
            AudioProcessing_ResetRecording();
            AudioProcessing_ClearRecordingComplete();

            printf("\r\n>>> Listening for keywords (ON/OFF)...\r\n\r\n");
        }

        // Volume display
        LautstaerkeZeigen();
    }
}
