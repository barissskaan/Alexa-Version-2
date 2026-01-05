#include "led_array.h"

static GPIO_TypeDef* port_arr[10] = {GPIOG,GPIOG,GPIOF,GPIOE,GPIOF,GPIOE,GPIOE,GPIOF,GPIOF,GPIOD};
static uint16_t pin_arr[10] = {GPIO_PIN_9,GPIO_PIN_14,GPIO_PIN_15,GPIO_PIN_13,GPIO_PIN_14,GPIO_PIN_11,GPIO_PIN_9,GPIO_PIN_13,GPIO_PIN_12,GPIO_PIN_15};

void led_func(int led_count){
	led_count = led_count > 10 ? 10 : led_count < 0 ? 0 : led_count;

	for(int i = 0; i<10; i++){
		HAL_GPIO_WritePin(port_arr[i],pin_arr[i],GPIO_PIN_RESET);
		}
	for(int i = 0; i<led_count;i++){
		HAL_GPIO_WritePin(port_arr[i],pin_arr[i],GPIO_PIN_SET);
	}

}



