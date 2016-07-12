/**
 * Copyright (C) JG 2016
 */


#include "stm32f4xx.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"

#include "misc.h"

#include "lib_ps2int.h"


 /**
  * @brief  Initialize external interrupt on PG9 (clock input)
  * @param  None
  * @retval None
  */
void LibPs2_Init(void)
    {
	// Set variables
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	// Enable clock for GPIOG
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	// Enable clock for SYSCFG
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// Set pin as input
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;

	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOG, &GPIO_InitStruct);

	// use PG7 for EXTI_Line9
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource9);

	// PG7 is connected to EXTI_Line9
    EXTI_InitStruct.EXTI_Line = EXTI_Line9;
	// Enable interrupts
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	// Interrupt mode
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	// Triggers on falling edges
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	// Add to EXTI
	EXTI_Init(&EXTI_InitStruct);

	// Add IRQ vector to NVIC / EXTI9_5_IRQn vector
	// vector is common to EXTI5 -> EXTI9
	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	// Set priority
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	// Set sub priority
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	// Enable interrupt
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	// Add to NVIC
	NVIC_Init(&NVIC_InitStruct);
    }
