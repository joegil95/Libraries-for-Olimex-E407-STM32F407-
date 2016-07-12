/**
 * Copyright (C) JG 2016
 */


#include "stm32f4xx.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"

#include "misc.h"

#include "lib_ps2.h"


int16_t fullcode[10]= {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};     // buffer clavier
GPIO_TypeDef * GPIO_port= GPIOC;
uint16_t GPIO_pin= (1 << 7);
int lock= 0;


 /**
  * @brief  Initialize external interrupt on PG9 (clock input)
  * @param  GPIOx = Data RX GPIO port (GPIOC or GPIOD)
  * @param  PINx = Data RX GPIO pin (0 to 15)
  * @retval None
  */
void LibPs2_Init(GPIO_TypeDef * GPIOx, uint16_t PINx)
    {
	// Set variables
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

    // Data port and pin
    GPIO_port= GPIOx;
    GPIO_pin= (1 << PINx);

    // Enable clock for GPIOC or GPIOD
    if (GPIO_port == GPIOC)
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    else
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    // RX set as input with pull-up
    GPIO_Init(GPIOx, &GPIO_InitStruct);

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
	// ELibPsénable interrupts
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


 /**
  * @brief  Get last received Ps2 data
  * @param  None
  * @retval Ps2 data (8 bits) or -1 if none
  */
int16_t LibPs2_Get()
    {
    int try= 0;
    static int p= 0;
    int16_t code= -1;

    do
        {
        if (!lock)
            {
            code= fullcode[p];
            fullcode[p]= -1;   // data lue
            p++;
            if (p >= 10) p= 0;
            try++;
            }
        }
    while ((code == -1) && (try < 3));

    return code;
    }


// modifie fullcode si caractère complet recu
void LibPs2_Receive(void)
    {
    static int state= START;
    static int16_t recu= 0;
    static int16_t code= -1;
    static int nb= 0, parity= 0;
    static int pos= 0;

    uint16_t bit= 0;

    // lecture data (sur PC7 par defaut)
    bit= GPIO_ReadInputData(GPIO_port) & (1 << 7);
    code= -1;

    switch(state)
        {
        case START:             // bit de start (0)
            recu= 0; nb= 0; parity= 0;
            if (bit == 0)  // ok
                state= DATA;
            else                // erreur => RAZ
                state= START;

            break;

        case DATA:              //  bit de donnee
            if (bit != 0)
                {
                recu |= (1 << nb);
                parity++;
                }
            nb++;
            if (nb >= 8) state= PARITY;
            break;

        case PARITY:            // verification de la parite
            if (bit != 0)
                parity++;

            if (parity & (1 << 0))  // impair ?
                state= STOP;        // ok
            else
                state= START;       // erreur => RAZ
            break;

        case STOP:                  // bit de stop (1)
            if (bit != 0)      // ok
                code= recu;

            state= START;
            break;
        }

    if (code != -1)     // caractere complet si != -1
        {
        lock= 1;    // verrouillage lecture

        fullcode[pos]= code;
        pos++;
        if (pos >= 10) pos= 0;

        lock= 0;
        }
    }


// routine d'interruption pour reception
void EXTI9_5_Handler(void)
    {
    static int state= START;
    static int16_t recu= 0;
    static int16_t code= -1;
    static int nb= 0, parity= 0;

    uint16_t bit= 0;

    if (EXTI_GetITStatus(EXTI_Line9) != RESET)
        {
        LibPs2_Receive();

        // Clear interrupt flag
        EXTI_ClearITPendingBit(EXTI_Line9);
        }
    }
