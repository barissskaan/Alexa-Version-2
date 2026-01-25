/**
 * @file    keyword_spotting.cpp
 * @brief   Keyword Spotting Module Implementation
 *
 * Integrates MFCC feature extraction and neural network inference
 * for real-time keyword recognition.
 */

#include "keyword_spotting.h"
#include "audio_filter.h"
#include "mfcc.h"
#include "kws.h"
#include "network.h"
#include <stdio.h>

// MFCC Parameters (optimized for the provided neural network)
#define NUM_MFCC_COEFFS 10       // Number of MFCC coefficients per frame
#define FRAME_LEN 320            // Frame length in samples (20ms @ 16kHz)
#define MFCC_DEC_BITS 1          // Quantization bits
#define NUM_FRAMES 48            // Number of frames (48 frames * 20ms = 960ms)
#define FRAME_SHIFT 320          // Frame shift (no overlap)
#define AUDIO_SAMPLE_RATE 16000  // Sample rate in Hz

// Static instances (initialized once)
static MFCC* mfccProcessor = nullptr;
static KWS* kwsSystem = nullptr;
static int lastDetectionIndex = -1;

// Temporary buffers
static float audioFloat[FRAME_LEN];
static q7_t mfccOutput[NUM_MFCC_COEFFS];

/**
 * @brief Initialize the keyword spotting system
 */
void KeywordSpotting_Init(void) {
    printf("\r\n[KWS] Initializing Keyword Spotting System...\r\n");

    // Initialize audio filters
    printf("[KWS] Initializing audio filters...\r\n");
    AudioFilter_Init();
    printf("[KWS] Audio filters initialized\r\n");

    // Create MFCC processor
    printf("[KWS] Creating MFCC processor...\r\n");
    mfccProcessor = new MFCC(NUM_MFCC_COEFFS, FRAME_LEN, MFCC_DEC_BITS);
    printf("[KWS] MFCC processor created\r\n");

    // Create KWS system (neural network)
    printf("[KWS] Creating Neural Network...\r\n");
    kwsSystem = new KWS();
    printf("[KWS] Keyword Spotting System initialized successfully!\r\n\r\n");
}

/**
 * @brief Process audio data and recognize keyword
 */
std::string KeywordSpotting_ProcessAudio(int32_t* audioData) {
    if (mfccProcessor == nullptr || kwsSystem == nullptr) {
        printf("[KWS ERROR] System not initialized!\r\n");
        return "_ERROR_";
    }

    printf("[KWS] Processing audio data...\r\n");
    
    // STEP 1: Apply audio filters for noise reduction and enhancement
    printf("[KWS] Applying audio filters...\r\n");
    AudioFilter_Reset();  // Reset filter states for new recording
    AudioFilter_ApplyAll(audioData, 16000);  // Filter all 16000 samples
    printf("[KWS] Audio filtering complete\r\n");

    // STEP 2: Extract MFCCs for each frame
    printf("[KWS] Extracting MFCC features...\r\n");
    for (int frame = 0; frame < NUM_FRAMES; frame++) {
        int startIdx = frame * FRAME_SHIFT;

        // Convert int32_t audio to float for MFCC computation
        for (int i = 0; i < FRAME_LEN; i++) {
            audioFloat[i] = (float)audioData[startIdx + i];
        }

        // Compute MFCC for this frame
        mfccProcessor->mfcc_compute(audioFloat, mfccOutput);

        // Copy MFCCs to neural network input buffer
        // Network expects: [48 frames][10 coefficients]
        for (int coeff = 0; coeff < NUM_MFCC_COEFFS; coeff++) {
            kwsSystem->mMFCC[frame][coeff] = (float)mfccOutput[coeff];
        }
    }

    printf("[KWS] MFCC extraction complete\r\n");
    
    // STEP 3: Run neural network inference
    printf("[KWS] Running neural network inference...\r\n");
    lastDetectionIndex = kwsSystem->runInference(1);

    if (lastDetectionIndex < 0) {
        printf("[KWS ERROR] Inference failed!\r\n");
        return "_ERROR_";
    }

    // Convert index to word
    std::string detectedWord = kwsSystem->indexToWord(lastDetectionIndex);

    printf("[KWS] *** DETECTED: %s (index: %d) ***\r\n", detectedWord.c_str(), lastDetectionIndex);

    return detectedWord;
}

/**
 * @brief Get the last detection index
 */
int KeywordSpotting_GetLastDetectionIndex(void) {
    return lastDetectionIndex;
}

/**
 * @brief Print neural network report
 */
void KeywordSpotting_PrintReport(void) {
    if (kwsSystem != nullptr) {
        kwsSystem->printReport();
    } else {
        printf("[KWS ERROR] System not initialized!\r\n");
    }
}
