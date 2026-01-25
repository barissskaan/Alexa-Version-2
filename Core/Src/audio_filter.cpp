/**
 * @file    audio_filter.cpp
 * @brief   Audio Signal Filtering Implementation
 * 
 * Implements various audio filters to improve speech recognition
 * in noisy environments (traffic, background noise, etc.)
 */

#include "audio_filter.h"
#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Filter state variables
static float hpf_prev_input = 0.0f;
static float hpf_prev_output = 0.0f;
static float lpf_prev_output = 0.0f;
static int32_t pre_emphasis_prev = 0;

// High-pass filter coefficients (1st order Butterworth, 80 Hz @ 16 kHz)
static float hpf_alpha = 0.0f;

// Low-pass filter coefficient (1st order, 4000 Hz @ 16 kHz)
static float lpf_alpha = 0.0f;

/**
 * @brief Initialize audio filters
 */
void AudioFilter_Init(void) {
    // Calculate high-pass filter coefficient
    // RC = 1 / (2 * π * fc)
    // α = RC / (RC + dt), where dt = 1/fs
    float rc_hp = 1.0f / (2.0f * M_PI * HPF_CUTOFF_FREQ);
    float dt = 1.0f / AUDIO_SAMPLE_RATE;
    hpf_alpha = rc_hp / (rc_hp + dt);
    
    // Calculate low-pass filter coefficient
    float rc_lp = 1.0f / (2.0f * M_PI * LPF_CUTOFF_FREQ);
    lpf_alpha = dt / (rc_lp + dt);
    
    AudioFilter_Reset();
}

/**
 * @brief Reset filter states
 */
void AudioFilter_Reset(void) {
    hpf_prev_input = 0.0f;
    hpf_prev_output = 0.0f;
    lpf_prev_output = 0.0f;
    pre_emphasis_prev = 0;
}

/**
 * @brief Remove DC offset from signal
 */
void AudioFilter_RemoveDCOffset(int32_t* buffer, uint32_t length) {
    // Calculate mean (DC offset)
    int64_t sum = 0;
    for (uint32_t i = 0; i < length; i++) {
        sum += buffer[i];
    }
    int32_t dc_offset = sum / length;
    
    // Remove DC offset
    for (uint32_t i = 0; i < length; i++) {
        buffer[i] -= dc_offset;
    }
}

/**
 * @brief Apply pre-emphasis filter
 * @note Boosts high frequencies (consonants) for better recognition
 */
void AudioFilter_PreEmphasis(int32_t* buffer, uint32_t length) {
    if (length == 0) return;
    
    // Apply pre-emphasis: y[n] = x[n] - α * x[n-1]
    // α = 0.97 is standard for speech processing
    int32_t prev = pre_emphasis_prev;
    
    for (uint32_t i = 0; i < length; i++) {
        int32_t current = buffer[i];
        buffer[i] = current - (int32_t)(PRE_EMPHASIS_COEFF * prev);
        prev = current;
    }
    
    // Save last sample for next call
    pre_emphasis_prev = prev;
}

/**
 * @brief Apply high-pass filter (1st order IIR)
 * @note Removes low-frequency noise (traffic, rumble, wind)
 */
void AudioFilter_HighPass(int32_t* buffer, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        float input = (float)buffer[i];
        
        // High-pass IIR filter: y[n] = α * (y[n-1] + x[n] - x[n-1])
        float output = hpf_alpha * (hpf_prev_output + input - hpf_prev_input);
        
        hpf_prev_input = input;
        hpf_prev_output = output;
        
        buffer[i] = (int32_t)output;
    }
}

/**
 * @brief Apply low-pass filter (1st order IIR)
 * @note Removes high-frequency noise above speech range
 */
void AudioFilter_LowPass(int32_t* buffer, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        float input = (float)buffer[i];
        
        // Low-pass IIR filter: y[n] = α * x[n] + (1 - α) * y[n-1]
        float output = lpf_alpha * input + (1.0f - lpf_alpha) * lpf_prev_output;
        
        lpf_prev_output = output;
        
        buffer[i] = (int32_t)output;
    }
}

/**
 * @brief Apply complete filter chain
 * @note Optimal order: DC Removal → High-Pass → Pre-Emphasis → Low-Pass
 */
void AudioFilter_ApplyAll(int32_t* buffer, uint32_t length) {
    // 1. Remove DC offset (biases can affect other filters)
    AudioFilter_RemoveDCOffset(buffer, length);
    
    // 2. High-pass filter (remove low-frequency noise)
    AudioFilter_HighPass(buffer, length);
    
    // 3. Pre-emphasis (boost high frequencies for speech)
    AudioFilter_PreEmphasis(buffer, length);
    
    // 4. Low-pass filter (remove very high frequency noise)
    AudioFilter_LowPass(buffer, length);
}
