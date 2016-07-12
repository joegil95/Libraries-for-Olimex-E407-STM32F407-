/**
 * External interrupts Library for STM32F4
*/


#ifndef LIBEXTINT_H
#define LIBEXTINT_H 250

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif


#include "stm32f4xx.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"

#include "misc.h"


void ExtInt_Init(GPIO_TypeDef* GPIOx, uint16_t Pin_k, EXTITrigger_TypeDef trigger);

/**
 * interrupt handlers to be defined by user
 */
void EXTI0_Handler(void);
void EXTI1_Handler(void);
void EXTI2_Handler(void);
void EXTI3_Handler(void);
void EXTI4_Handler(void);
void EXTI9_5_Handler(void);
void EXTI15_10Handler(void);


/// template

/*
void EXTI0_Handler(void)
    {
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
        {


        // Clear interrupt flag
        EXTI_ClearITPendingBit(EXTI_Line0);
        }
    }
*/

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
