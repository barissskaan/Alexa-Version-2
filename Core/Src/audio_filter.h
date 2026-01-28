/**
 * @file    audio_filter.h
 * @brief   Audio Signal Filtering and Enhancement Module
 * 
 * Provides signal processing filters to improve audio quality
 * and speech recognition accuracy in noisy environments.
 */

#ifndef AUDIO_FILTER_H
#define AUDIO_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "arm_math.h"

// Filter configuration
#define AUDIO_SAMPLE_RATE 16000
#define PRE_EMPHASIS_COEFF 0.97f
#define HPF_CUTOFF_FREQ 80        // High-pass filter cutoff (Hz)
#define LPF_CUTOFF_FREQ 4000      // Low-pass filter cutoff (Hz)

/**
 * @brief  Initialize audio filters
 * @note   Must be called before using filter functions
 */
void AudioFilter_Init(void);

/**
 * @brief  Apply DC offset removal to audio buffer
 * @param  buffer: Audio buffer (in-place processing)
 * @param  length: Buffer length in samples
 * @note   Removes DC bias from signal
 */
void AudioFilter_RemoveDCOffset(int32_t* buffer, uint32_t length);

/**
 * @brief  Apply pre-emphasis filter
 * @param  buffer: Audio buffer (in-place processing)
 * @param  length: Buffer length in samples
 * @note   Boosts high frequencies for better speech recognition
 *         Formula: y[n] = x[n] - α * x[n-1], where α = 0.97
 */
void AudioFilter_PreEmphasis(int32_t* buffer, uint32_t length);

/**
 * @brief  Apply high-pass filter (removes low-frequency noise)
 * @param  buffer: Audio buffer (in-place processing)
 * @param  length: Buffer length in samples
 * @note   Removes rumble, wind noise, traffic noise below 80 Hz
 */
void AudioFilter_HighPass(int32_t* buffer, uint32_t length);

/**
 * @brief  Apply low-pass filter (removes high-frequency noise)
 * @param  buffer: Audio buffer (in-place processing)
 * @param  length: Buffer length in samples
 * @note   Removes high-frequency noise above 4000 Hz
 */
void AudioFilter_LowPass(int32_t* buffer, uint32_t length);

/**
 * @brief  Apply complete filter chain (all filters)
 * @param  buffer: Audio buffer (in-place processing)
 * @param  length: Buffer length in samples
 * @note   Applies: DC Removal → High-Pass → Low-Pass → Pre-Emphasis
 *         Pre-emphasis is applied last to avoid attenuation by low-pass filter
 */
void AudioFilter_ApplyAll(int32_t* buffer, uint32_t length);

/**
 * @brief  Reset filter states
 * @note   Call when starting new recording
 */
void AudioFilter_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_FILTER_H */
