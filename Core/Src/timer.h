#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
extern TIM_HandleTypeDef htim4;
void delayMicroseconds(int i);

#ifdef __cplusplus
}
#endif

#endif
