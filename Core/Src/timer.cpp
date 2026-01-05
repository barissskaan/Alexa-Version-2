#include "timer.h"

extern "C" {

void delayMicroseconds(int i){
	__HAL_TIM_SET_COUNTER(&htim4,0);
	// int temp;
	// int counter = 0;
	while(__HAL_TIM_GET_COUNTER(&htim4)<i){
		// if(counter % 100 == 0 ) temp = __HAL_TIM_GET_COUNTER(&htim4);
		// counter++;
	}
}

} // extern "C"
