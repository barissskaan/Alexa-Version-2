/**
 * @file    keyword_spotting.h
 * @brief   Keyword Spotting Module - Integrates MFCC and Neural Network
 *
 * This module provides high-level functions to process audio data
 * and recognize keywords using MFCC feature extraction and neural network inference.
 */

#ifndef KEYWORD_SPOTTING_H
#define KEYWORD_SPOTTING_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#include <string>

/**
 * @brief  Initialize the keyword spotting system
 * @note   Initializes MFCC processor and neural network
 */
void KeywordSpotting_Init(void);

/**
 * @brief  Process audio data and recognize keyword
 * @param  audioData: Pointer to 16000 samples of audio data (1 second @ 16kHz)
 * @return Detected keyword as string ("ON", "OFF", "YES", "NO", etc.)
 */
std::string KeywordSpotting_ProcessAudio(int32_t* audioData);

/**
 * @brief  Get the last detected keyword index
 * @return Index of the detected keyword (see kws.cpp for mapping)
 */
int KeywordSpotting_GetLastDetectionIndex(void);

/**
 * @brief  Print neural network report
 * @note   Useful for debugging and understanding the network
 */
void KeywordSpotting_PrintReport(void);

#endif /* __cplusplus */

#endif /* KEYWORD_SPOTTING_H */
