#ifndef KWS_H
#define KWS_H

#include <stdio.h>
#include <string>

#include "ai_datatypes_defines.h"
#include "ai_platform.h"
#include "network.h"
#include "network_data.h"

class KWS {
  private:
    float mOutput[9];

    AI_ALIGNED(4) ai_float out_data[AI_NETWORK_OUT_1_SIZE];
    ai_handle network;
    AI_ALIGNED(4) ai_u8 activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];
    AI_ALIGNED(4) ai_buffer ai_input[AI_NETWORK_IN_NUM];
    AI_ALIGNED(4) ai_buffer ai_output[AI_NETWORK_OUT_NUM];

    int aiInit();
  public:
    AI_ALIGNED(4) ai_float mMFCC[AI_NETWORK_IN_1_HEIGHT][AI_NETWORK_IN_1_WIDTH];

    KWS();
    void printReport();
    int runInference(int batch_size = 1);
    std::string indexToWord(int index);
};

#endif
