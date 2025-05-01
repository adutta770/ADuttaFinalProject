/*
 * InterruptControl.h
 *
 *  Created on: Apr 20, 2025
 *      Author: dutai
 */

#include <stdint.h>
#include "stm32f4xx_hal.h"

#ifndef INTERRUPTCONTROL_H_
#define INTERRUPTCONTROL_H_

void IRQ_Enable_Interrupt(IRQn_Type IRQ_num);
void IRQ_Disable_Interrupt(IRQn_Type IRQ_num);
void IRQ_Clear_Interrupt_Pending(IRQn_Type IRQ_num);
void IRQ_Set_Interrupt_Pending(IRQn_Type IRQ_num);

#endif /* INTERRUPTCONTROL_H_ */
