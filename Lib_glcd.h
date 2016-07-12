/**********************************************************************/
/*        Header du Graphic LCD 128x64 piloté par STM32F4             */
/**********************************************************************/

#include "stm32f4xx.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

// Données sur Port F
#define GLCD_DAT_PORT  	GPIOF   // PF0 a PF7
#define GLCD_DAT_CLK    RCC_AHB1Periph_GPIOF

// Commandes sur Port F
#define GLCD_CMD_PORT  	GPIOF
#define GLCD_CMD_CLK    RCC_AHB1Periph_GPIOF

// 8 Pins de data
#define Pin_DATA   (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7)

// 6 Pins de commandes
#define Pin_DI		GPIO_Pin_10     //  Alias RS
#define Pin_RW		GPIO_Pin_11     //
#define Pin_EN		GPIO_Pin_12     //

#define Pin_CS1		GPIO_Pin_14     //  a inverser avec CS2 selon afficheurs
#define Pin_CS2		GPIO_Pin_13     //  a inverser avec CS1 selon afficheurs
#define Pin_RST		GPIO_Pin_15     //

#define Pin_CMD    (Pin_DI | Pin_RW | Pin_EN | Pin_CS1 | Pin_CS2 | Pin_RST)

// Macros
#define asset(p, bit)	GPIO_SetBits(p, bit)
#define reset(p, bit)	GPIO_ResetBits(p, bit)

// Divers
#define PAGE_0           0xB8       // Adress base for Page 0
#define COL_0            0x40       // Adress base for Col 0

#define START_LINE       0xC0       // Adress base for line 0 a adapter si necessaire

#define DISPLAY_ON       0x3F       // Turn display on
#define DISPLAY_OFF      0x3E       // Turn display off

#define LCD_BUSY         0x80       // GLCD occupé

#define LEFT        0               // Pour la sélection du demi afficheur
#define RIGHT       1
#define BOTH        2


// Prototypes des fonctions du module
void GLCD_Delay(uint32_t);        	  	// Tempo de fonctionnement du LCD
void GLCD_SelectSide(uint8_t);          	// Sélection d'un demi afficheur
void GLCD_Command(uint8_t);    	  	  	    // Envoi d'une commande
void GLCD_DataWrite(uint8_t);           	// Ecriture d'une colonne de 8 pixels
uint8_t GLCD_DataRead(void);             	// Lecture de données de l'afficheur
void GLCD_WaitBusy(void);             	    // Attente LCD prêt
void GLCD_Move(uint8_t, uint8_t , uint8_t); // Déplacement
void GLCD_WriteChar(uint8_t);      	 	    // Ecriture d'un caractère
void GLCD_Write(uint8_t *); 				// Ecriture d'une chaine AsciiZ
void GLCD_ShowInt(int);					    // Affichage d'un entier
void GLCD_InitPorts(void);    	   	 	    // Initialisation des Ports du LCD
void GLCD_InitDisplay(void);       	  	    // Initialisation de l'afficheur
void GLCD_ClearScreen(void);      		    // Effacement de l'écran

