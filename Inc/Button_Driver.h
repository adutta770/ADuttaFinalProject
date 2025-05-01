/*
 * Button_Driver.h
 *
 *  Created on: Apr 20, 2025
 *      Author: dutai
 */

#include <stdbool.h>
#include "stm32f4xx_hal.h"

#ifndef BUTTON_DRIVER_H_
#define BUTTON_DRIVER_H_

#define BUTTON_PIN_NUM GPIO_PIN_0
#define BUTTON_PRESSED 1
#define BUTTON_NOT_PRESSED 0

void Button_Init();
void Button_Clock_Enable();
bool Button_State();


#endif /* BUTTON_DRIVER_H_ */
