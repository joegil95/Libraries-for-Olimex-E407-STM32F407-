/**
 * Copyright (C) JG 2016
 */

/// Connecter les lignes sur les pins P8-11 du port D ou E
/// Connecter les colonnes sur les pins P12-14 du meme port


#include "lib_keypad.h"

int8_t Clavier[3][4]= {{1, 4, 7, 10}, {2,5,8, 0}, {3, 6, 9, 11}};


/**
  * @brief  Initialize keypad ports
  * @param  GPIOx = GPIOD or GPIOE
  * @retval None
  */
void LibKeypad_Init(GPIO_TypeDef* GPIOx)
    {
    if (GPIOx == GPIOD)
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    if (GPIOx == GPIOE)
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitTypeDef gpioInit;

    // colonnes en sorties
    gpioInit.GPIO_Mode = GPIO_Mode_OUT;
    gpioInit.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_Init(GPIOx, &gpioInit);

    // lignes en entrees avec pull-up
    gpioInit.GPIO_Mode = GPIO_Mode_IN;
    gpioInit.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    gpioInit.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOx, &gpioInit);

    // mise a 1 de toutes les colonnes
    GPIO_Write(GPIOx, 0xF000);
    }


/**
  * @brief  Get keypad touch
  * @param  GPIOx = GPIOD or GPIOE
  * @retval Pressed touch if any, or -1 if none
  */
int8_t LibKeypad_Get(GPIO_TypeDef* GPIOx)
    {
    uint16_t col, lig, input, mask= 0x0F00;
    int8_t touche= -1;

    for(col= 0 ; col < 3 ; col++)
        {
        // mise a 0 d'une des 3 colonnes
        GPIO_ResetBits(GPIOx, (1 << (col + 12)));

        do
          	{
            // lecture  des lignes (logique inverse)
            input= ~GPIO_ReadInputData(GPIOx) & mask;
            for (long tempo= 0 ; tempo < 100000 ; tempo++);
            }
        while ((~GPIO_ReadInputData(GPIOx) & mask) != input);

		// determination de la ligne
		for (lig= 0 ; lig < 4 ; lig++)
            {
            if (input & (1 << (lig + 8)))
                touche = Clavier[col][lig];
            }

        // remise a 1 de la colonne
        GPIO_SetBits(GPIOx, (1 << (col + 12)));

		if (touche != -1) break;
        }

    return (touche);
    }
