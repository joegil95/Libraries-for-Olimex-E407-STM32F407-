/*
 * Keypad 3 x 4 Library for STM32F4xx devices
 */

#include "stm32f4xx.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

// initialisation des ports pour le clavier
void LibKeypad_Init(GPIO_TypeDef* GPIOx);

// lecture d'une touche au clavier
// retourne -1 si RAS
int8_t LibKeypad_Get(GPIO_TypeDef* GPIOx);

