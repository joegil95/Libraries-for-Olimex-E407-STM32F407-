/**
 * Copyright (C) JG 2016
 */

#include "lib_extint.h"

/// Configuration d'interruptions externes
/// 16 interruptions possibles sur GPIOx / ligne PINk
/// avec x= A a F et k= 0 a 15
/// une seule interruption possible sur chaque ligne

/**
 * @brief  Initialize External Interrupt
 * @param  GPIOx = GPIOA tp GPIOG port
 * @param  Pin_k = 0 to 15
 * @param  trigger = EXTI_Trigger_Falling or EXTI_Trigger_Rising or EXTI_Trigger_Rising_Falling
 * @retval None
 */
void ExtInt_Init(GPIO_TypeDef* GPIOx, uint16_t Pin_k, EXTITrigger_TypeDef trigger)
    {
	// Set variables used
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	uint16_t GPIO_Pin_k= (1 << Pin_k);

	// Enable clock for GPIOx
	if (GPIOx == GPIOA)	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	if (GPIOx == GPIOB)	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	if (GPIOx == GPIOC)	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	if (GPIOx == GPIOD)	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	if (GPIOx == GPIOE)	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	if (GPIOx == GPIOF)	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	if (GPIOx == GPIOG)	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

	// Enable clock for SYSCFG
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// Set pin as input
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_k;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOx, &GPIO_InitStruct);

	// Use Pin k on port GPIOx
    // SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOx, EXTI_PinSourcek);
    if (GPIOx == GPIOA)	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, Pin_k);
    if (GPIOx == GPIOB)	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, Pin_k);
    if (GPIOx == GPIOC)	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, Pin_k);
    if (GPIOx == GPIOD)	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, Pin_k);
    if (GPIOx == GPIOE)	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, Pin_k);
    if (GPIOx == GPIOF)	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, Pin_k);
    if (GPIOx == GPIOG)	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, Pin_k);

	// Connect to Line k
    // EXTI_InitStruct.EXTI_Line = EXTI_Linek;
    EXTI_InitStruct.EXTI_Line = GPIO_Pin_k;
	// Enable interrupts
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	// Interrupt mode
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	// Trigger mode
	EXTI_InitStruct.EXTI_Trigger = trigger;
	// Add to EXTI
	EXTI_Init(&EXTI_InitStruct);

	// Add IRQ vector to NVIC / EXTIk_IRQn vector
	if (GPIO_Pin_k == GPIO_Pin_0) NVIC_InitStruct.NVIC_IRQChannel= EXTI0_IRQn;
	if (GPIO_Pin_k == GPIO_Pin_1) NVIC_InitStruct.NVIC_IRQChannel= EXTI1_IRQn;
	if (GPIO_Pin_k == GPIO_Pin_2) NVIC_InitStruct.NVIC_IRQChannel= EXTI2_IRQn;
	if (GPIO_Pin_k == GPIO_Pin_3) NVIC_InitStruct.NVIC_IRQChannel= EXTI3_IRQn;
	if (GPIO_Pin_k == GPIO_Pin_4) NVIC_InitStruct.NVIC_IRQChannel= EXTI4_IRQn;
	if ((GPIO_Pin_k == GPIO_Pin_5) || (GPIO_Pin_k == GPIO_Pin_6) || (GPIO_Pin_k == GPIO_Pin_7) ||
        (GPIO_Pin_k == GPIO_Pin_8) ||( GPIO_Pin_k == GPIO_Pin_9))
        NVIC_InitStruct.NVIC_IRQChannel= EXTI9_5_IRQn;
	if ((GPIO_Pin_k == GPIO_Pin_10) || (GPIO_Pin_k == GPIO_Pin_11) || (GPIO_Pin_k == GPIO_Pin_12) ||
        (GPIO_Pin_k == GPIO_Pin_13) || (GPIO_Pin_k == GPIO_Pin_14) || (GPIO_Pin_k == GPIO_Pin_15))
        NVIC_InitStruct.NVIC_IRQChannel= EXTI15_10_IRQn;

	// Set priority
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;   /// a adapter ???
	// Set sub priority
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;      /// a adapter si plusieurs lignes ???
	// Enable interrupt
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	// Add to NVIC
	NVIC_Init(&NVIC_InitStruct);
    }
