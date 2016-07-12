/**
 * Ps2 (Mouse, keyboard...) Library for STM32F4xx devices
 */
 

#ifndef __PS2_H__
#define __PS2_H__

#include "stm32f4xx.h"


typedef enum {START, DATA, PARITY, STOP, FINISHED } PS2_State;

void LibPs2_Init(GPIO_TypeDef * GPIOx, uint16_t PINx);

int16_t LibPs2_Get(void);

/**
 * routine d'interruption a definir
 * commune aux interruptions externes 5 a 9
 */
void EXTI9_5_Handler(void);


/// template

/*
void EXTI9_5_Handler(void)
    {
    // Check if interrupt on EXTI_Line9
    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
        {


        // Clear interrupt flag
        EXTI_ClearITPendingBit(EXTI_Line9);
        }
    }
*/

#endif
