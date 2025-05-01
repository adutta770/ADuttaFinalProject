/*
 * ApplicationCode.h
 *
 *  Created on: Dec 30, 2023
 *      Author: Xavion
 */

#include "stm32f4xx_hal.h"
#include "LCD_Driver.h"
#include "Button_Driver.h"
#include "InterruptControl.h"


#include <stdio.h>


#ifndef INC_APPLICATIONCODE_H_
#define INC_APPLICATIONCODE_H_

//STMPE811_TouchData getTouchData();
void ApplicationInit(void);
void LCD_Visual_Demo(void);

#if (COMPILE_TOUCH_FUNCTIONS == 1)
void LCD_Touch_Polling_Demo(void);
#endif // (COMPILE_TOUCH_FUNCTIONS == 1)

#endif /* INC_APPLICATIONCODE_H_ */
