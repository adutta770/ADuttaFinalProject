/*
 * Button_Driver.c
 *
 *  Created on: Sep 24, 2024
 *      Author: dutai
 */

#include "Button_Driver.h"

void Button_Init(){
	GPIO_InitTypeDef button_Init;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	button_Init.Pin = BUTTON_PIN_NUM;
	button_Init.Mode = GPIO_MODE_IT_RISING;
	button_Init.Pull = GPIO_NOPULL;
	button_Init.Speed = GPIO_SPEED_FREQ_HIGH; // initially GPIO_SPEED_FREQ_MEDIUM
	// button_Init.Alternate not set
	HAL_GPIO_Init(GPIOA, &button_Init);
}

void Button_Clock_Enable(){
	__HAL_RCC_GPIOA_CLK_ENABLE();
}

bool Button_State(){
	GPIO_PinState on_off;
	on_off = HAL_GPIO_ReadPin(GPIOA, BUTTON_PIN_NUM);
	if(on_off == GPIO_PIN_SET)
		return true;
	else
		return false;
}
