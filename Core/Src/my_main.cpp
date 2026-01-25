#include "my_main.h"
#include "main.h"
#include "led_array.h"
#include "timer.h"
#include "transmit.h"
#include "audio_processing.h"
#include "keyword_spotting.h"
#include "i2c_lcd.h"
#include <stdio.h>
#include <cmath>
#include <cstring>
#include <string>

// EXTERNAL HAL HANDLES
extern TIM_HandleTypeDef htim4;    // Timer for microsecond delays
extern I2S_HandleTypeDef hi2s2;    // I2S for microphone input
extern UART_HandleTypeDef huart3;  // UART for printf output
extern I2C_HandleTypeDef hi2c1;    // I2C for LCD display

// Menu states
enum MenuState {
    MENU_IDLE,           // Waiting for "ON" command
    MENU_DURATION,       // Showing duration options
    MENU_SOCKET_ON       // Socket is ON
};

// Timer mode
enum TimerMode {
    TIMER_FOREVER,       // Socket stays on forever
    TIMER_10MIN          // Socket turns off after 10 minutes
};

// Global state
static MenuState currentMenu = MENU_IDLE;
static TimerMode timerMode = TIMER_FOREVER;
static uint32_t socketOnTime = 0;
static bool socketState = false;

 // PRINTF RETARGET
extern "C" int _write(int file, char* ptr, int len) {
    (void)file;
    HAL_UART_Transmit(&huart3, reinterpret_cast<uint8_t*>(ptr), len, HAL_MAX_DELAY);
    return len;
}

// Helper functions
void ShowIdleScreen(void) {
    LCD_Clear();
    LCD_PrintAt(0, 0, "Say: ON or OFF");
    LCD_PrintAt(1, 0, "Ready...");
}

void ShowDurationMenu(void) {
    LCD_Clear();
    LCD_PrintAt(0, 0, "1:Forever");
    LCD_PrintAt(1, 0, "2:10 Minutes");
}

void ShowSocketOn(TimerMode mode) {
    LCD_Clear();
    LCD_PrintAt(0, 0, "Socket: ON");
    if (mode == TIMER_FOREVER) {
        LCD_PrintAt(1, 0, "Mode: Forever");
    } else {
        LCD_PrintAt(1, 0, "Mode: 10 min");
    }
}

void ShowSocketOff(void) {
    LCD_Clear();
    LCD_PrintAt(0, 0, "Socket: OFF");
    LCD_PrintAt(1, 0, "Say: ON");
}

void UpdateTimerDisplay(uint32_t remainingSeconds) {
    char buffer[17];
    uint32_t minutes = remainingSeconds / 60;
    uint32_t seconds = remainingSeconds % 60;
    snprintf(buffer, sizeof(buffer), "Time: %02lu:%02lu", minutes, seconds);
    LCD_SetCursor(1, 0);
    LCD_Print("                "); // Clear line
    LCD_PrintAt(1, 0, buffer);
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
    printf("   DIY Alexa - Interactive System\r\n");
    printf("========================================\r\n");

    // Initialize LCD display
    printf("[INIT] Initializing LCD display...\r\n");
    if (LCD_Init(&hi2c1)) {
        printf("[INIT] LCD initialized successfully\r\n");
        LCD_Backlight(true);
        LCD_Clear();
        LCD_PrintAt(0, 0, "DIY Alexa v2.0");
        LCD_PrintAt(1, 0, "Starting...");
        HAL_Delay(2000);
    } else {
        printf("[ERROR] LCD initialization failed!\r\n");
    }

    // Initialize audio processing module
    AudioProcessing_Init();
    
    // Initialize keyword spotting system (MFCC + Neural Network)
    KeywordSpotting_Init();

    // Start I2S DMA reception
    HAL_StatusTypeDef status = HAL_I2S_Receive_DMA(&hi2s2, AudioProcessing_GetInputBuffer(), I2S_BUF_SIZE);
    if (status != HAL_OK) {
        printf("[ERROR] I2S DMA start failed! Error: %d\r\n", status);
        LCD_Clear();
        LCD_PrintAt(0, 0, "ERROR: I2S");
        LCD_PrintAt(1, 0, "Check hardware");
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
    LCD_Clear();
    LCD_PrintAt(0, 0, "Warmup...");
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
    printf("\r\n>>> Listening for keywords...\r\n\r\n");

    // Show idle screen
    ShowIdleScreen();
    currentMenu = MENU_IDLE;

    // Main loop
    uint32_t lastTimerUpdate = 0;

    while (1) {
        // Check timer for 10-minute mode
        if (socketState && timerMode == TIMER_10MIN && currentMenu == MENU_SOCKET_ON) {
            uint32_t elapsed = (HAL_GetTick() - socketOnTime) / 1000; // seconds
            uint32_t remaining = (10 * 60) - elapsed;  // 10 minutes in seconds
            
            // Update display every second
            if (HAL_GetTick() - lastTimerUpdate >= 1000) {
                UpdateTimerDisplay(remaining);
                lastTimerUpdate = HAL_GetTick();
            }
            
            // Check if 10 minutes elapsed
            if (elapsed >= 10 * 60) {
                printf("\r\n>>> [TIMER] 10 minutes elapsed - Turning socket OFF\r\n");
                sendSequence(off);
                socketState = false;
                currentMenu = MENU_IDLE;
                ShowSocketOff();
                HAL_Delay(2000);
                ShowIdleScreen();
            }
        }
        
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
            
            // Handle keywords based on current menu state
            switch (currentMenu) {
                case MENU_IDLE:
                    // Waiting for ON or OFF command
                    if (detectedWord == "ON") {
                        printf("\r\n>>> [MENU] ON detected - Showing duration menu\r\n");
                        currentMenu = MENU_DURATION;
                        ShowDurationMenu();
                        // Visual feedback
                        for (int i = 0; i < 2; i++) {
                            led_func(10);
                            HAL_Delay(100);
                            led_func(0);
                            HAL_Delay(100);
                        }
                    } 
                    else if (detectedWord == "OFF") {
                        if (socketState) {
                            printf("\r\n>>> [ACTION] Turning Socket OFF\r\n");
                            sendSequence(off);
                            socketState = false;
                            ShowSocketOff();
                            HAL_Delay(2000);
                            ShowIdleScreen();
                            // Visual feedback
                            led_func(10);
                            HAL_Delay(500);
                            led_func(0);
                        } else {
                            printf("\r\n>>> Socket is already OFF\r\n");
                            LCD_Clear();
                            LCD_PrintAt(0, 0, "Already OFF!");
                            HAL_Delay(1000);
                            ShowIdleScreen();
                        }
                    }
                    break;
                    
                case MENU_DURATION:
                    // Waiting for ONE or TWO to select duration
                    if (detectedWord == "ONE") {
                        printf("\r\n>>> [MENU] ONE selected - Forever mode\r\n");
                        timerMode = TIMER_FOREVER;
                        sendSequence(on);
                        socketState = true;
                        socketOnTime = HAL_GetTick();
                        currentMenu = MENU_SOCKET_ON;
                        ShowSocketOn(TIMER_FOREVER);
                        // Visual feedback
                        for (int i = 0; i < 3; i++) {
                            led_func(10);
                            HAL_Delay(100);
                            led_func(0);
                            HAL_Delay(100);
                        }
                    }
                    else if (detectedWord == "TWO") {
                        printf("\r\n>>> [MENU] TWO selected - 10 minute mode\r\n");
                        timerMode = TIMER_10MIN;
                        sendSequence(on);
                        socketState = true;
                        socketOnTime = HAL_GetTick();
                        lastTimerUpdate = HAL_GetTick();
                        currentMenu = MENU_SOCKET_ON;
                        ShowSocketOn(TIMER_10MIN);
                        HAL_Delay(1000);
                        UpdateTimerDisplay(10 * 60);
                        // Visual feedback
                        for (int i = 0; i < 3; i++) {
                            led_func(10);
                            HAL_Delay(100);
                            led_func(0);
                            HAL_Delay(100);
                        }
                    }
                    else {
                        // Invalid input, show error
                        printf("\r\n>>> Invalid selection: %s (say ONE or TWO)\r\n", detectedWord.c_str());
                        LCD_Clear();
                        LCD_PrintAt(0, 0, "Say: ONE or TWO");
                        LCD_PrintAt(1, 0, "Try again...");
                        HAL_Delay(1500);
                        ShowDurationMenu();
                    }
                    break;
                    
                case MENU_SOCKET_ON:
                    // Socket is ON, can turn OFF
                    if (detectedWord == "OFF") {
                        printf("\r\n>>> [ACTION] Turning Socket OFF\r\n");
                        sendSequence(off);
                        socketState = false;
                        currentMenu = MENU_IDLE;
                        ShowSocketOff();
                        HAL_Delay(2000);
                        ShowIdleScreen();
                        // Visual feedback
                        led_func(10);
                        HAL_Delay(500);
                        led_func(0);
                    } else {
                        printf("\r\n>>> Socket is ON - Say OFF to turn off\r\n");
                        LCD_Clear();
                        LCD_PrintAt(0, 0, "Say: OFF");
                        LCD_PrintAt(1, 0, "to turn off");
                        HAL_Delay(1500);
                        ShowSocketOn(timerMode);
                        if (timerMode == TIMER_10MIN) {
                            uint32_t elapsed = (HAL_GetTick() - socketOnTime) / 1000;
                            UpdateTimerDisplay((10 * 60) - elapsed);
                        }
                    }
                    break;
            }
            
            // Reset recording state
            AudioProcessing_ResetRecording();
            AudioProcessing_ClearRecordingComplete();

            printf("\r\n>>> Listening for keywords...\r\n\r\n");
        }

        // Volume display
        LautstaerkeZeigen();
    }
}
