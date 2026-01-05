/**
 * @file    audio_processing.h
 * @brief   Audio Processing Module for Microphone Input
 * 
 * Hardware: Adafruit SPH0645LM4H-B Microphone
 * Protocol: I2S2 (PC3=SD, PB10=CK, PB12=WS)
 * Sample Rate: 16 kHz
 */

#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Buffer size for I2S DMA */
#define I2S_BUF_SIZE 1000
/* Noise threshold (upper limit - samples above this are ignored as noise) */
#define NOISE_THRESHOLD 12000
/* Initial adaptive threshold offset */
#define INITIAL_OFFSET 7500

/**
 * @brief  Initialize audio processing module
 * @note   Must be called before starting I2S DMA
 */
void AudioProcessing_Init(void);

/**
 * @brief  Enable/disable audio data processing
 * @param  enable: true to enable, false to disable
 */
void AudioProcessing_Enable(bool enable);

/**
 * @brief  Process audio frame and check for threshold
 * @note   Called from DMA callbacks
 */
void Audio1Sec(void);

/**
 * @brief  Display volume level on LED array
 * @note   Called from main loop
 */
void LautstaerkeZeigen(void);

/**
 * @brief  Calculate average volume of current frame
 * @return Average absolute amplitude
 */
int mittelwert(void);

/**
 * @brief  Check if recording is complete
 * @return true if 1 second recording is complete
 */
bool AudioProcessing_IsRecordingComplete(void);

/**
 * @brief  Get recording start time
 * @return Recording start time in milliseconds
 */
uint32_t AudioProcessing_GetRecordingStartTime(void);

/**
 * @brief  Get recorded audio data (1 second, 16000 samples)
 * @return Pointer to ISecArray buffer
 */
int32_t* AudioProcessing_GetRecordedData(void);

/**
 * @brief  Clear recording complete flag
 * @note   Call after processing recorded data
 */
void AudioProcessing_ClearRecordingComplete(void);

/**
 * @brief  Reset recording state
 * @note   Call after processing recorded data
 */
void AudioProcessing_ResetRecording(void);

/**
 * @brief  Get DMA input buffer pointer
 * @return Pointer to inputBuffer1 (for DMA initialization)
 */
uint16_t* AudioProcessing_GetInputBuffer(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_PROCESSING_H */

