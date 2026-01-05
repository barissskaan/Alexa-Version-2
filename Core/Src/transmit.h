#ifndef TRANSMIT_H
#define TRANSMIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include "timer.h"


void transmit(int high_val,int low_val);
void sendSequence(uint16_t bitSequence);

#ifdef __cplusplus
}
#endif

#endif
