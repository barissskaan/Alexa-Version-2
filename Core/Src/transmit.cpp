#include "transmit.h"
#include "stm32f4xx_hal.h"

void transmit(int high_val,int low_val){
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_9,GPIO_PIN_RESET);
	delayMicroseconds(high_val*350);
	//HAL_Delay(high_val*1000);
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_9,GPIO_PIN_SET);
	delayMicroseconds(low_val*350);
	//HAL_Delay(low_val*1000);

	}
void sendSequence(uint16_t bitSequence){
	uint16_t ref = 0x0800;
	for(int i = 0; i < 10; i++){
		uint16_t tempSequence = bitSequence;
		for(int j = 0; j<12; j++){
			transmit(1,3);
			if(tempSequence&ref){
				transmit(1,3);
			}
			else{
				transmit(3,1);

			}
			tempSequence = tempSequence << 1;
		}
		transmit(1,31);



	}

}
