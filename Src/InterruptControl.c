/*
 * InterruptControl.c
 *
 *  Created on: Oct 1, 2024
 *      Author: dutai
 */

#include "InterruptControl.h"

// !!!!! Replace with HAL functions?

void IRQ_Enable_Interrupt(IRQn_Type IRQ_num){
	HAL_NVIC_EnableIRQ(IRQ_num);
}

void IRQ_Disable_Interrupt(IRQn_Type IRQ_num){
	HAL_NVIC_DisableIRQ(IRQ_num);
}

void IRQ_Clear_Interrupt_Pending(IRQn_Type IRQ_num){
	HAL_NVIC_ClearPendingIRQ(IRQ_num);
}

void IRQ_Set_Interrupt_Pending(IRQn_Type IRQ_num){
	HAL_NVIC_SetPendingIRQ(IRQ_num);
}
