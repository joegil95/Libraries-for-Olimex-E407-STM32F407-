/**********************************************************************/
/*        Pilote du Graphic LCD 128x64 piloté par STM32F4             */
/*		  Fonctionne avec afficheurs a base de KS0107/0108            */
/**********************************************************************/


#include "lib_glcd.h"

#include "font5x8.h"

// Tempo pour le timing du LCD
void GLCD_Delay(uint32_t duree)
    {
    uint32_t delay;

    for(delay = 0 ; delay < (duree * 2) ; delay++);
    }

// Selection du demi-LCD  side = LEFT or RIGHT
void GLCD_SelectSide(uint8_t side)
    {
    if(side==LEFT)
    	{
        reset(GLCD_CMD_PORT, Pin_CS1);
        asset(GLCD_CMD_PORT, Pin_CS2);
        }
    if(side==RIGHT)
    	{
        asset(GLCD_CMD_PORT, Pin_CS1);
        reset(GLCD_CMD_PORT, Pin_CS2);
        }

   if(side==BOTH)
    	{
        asset(GLCD_CMD_PORT, Pin_CS1);
        asset(GLCD_CMD_PORT, Pin_CS2);
        }

    GLCD_Command(COL_0);   	   	 // Colonne 0
    }

// Envoi d'une commande au LCD
void GLCD_Command(uint8_t cmd)
    {
    uint16_t output= 0;

    GLCD_WaitBusy();                    // LCD busy ?
    reset(GLCD_CMD_PORT, Pin_DI);       // Instruction mode
    reset(GLCD_CMD_PORT, Pin_RW);       // Write mode

    //  GLCD_DAT_OUT = cmd;   	    // outbyte
    output= GPIO_ReadOutputData(GLCD_DAT_PORT) & 0xFF00;
    GPIO_Write(GLCD_DAT_PORT, output | cmd);

    asset(GLCD_CMD_PORT, Pin_EN);       // Strobe
    GLCD_Delay(1);                      // 1.5µs mini
    reset(GLCD_CMD_PORT, Pin_EN);
    }

// Envoi d'une donnée au LCD
void GLCD_DataWrite(uint8_t data)
    {
    uint16_t output= 0;

    GLCD_WaitBusy();                    // LCD busy ?
    asset(GLCD_CMD_PORT, Pin_DI);       // Data mode
    reset(GLCD_CMD_PORT, Pin_RW);       // Write mode

    //  GLCD_DAT_OUT = data;            // outbyte
    output= GPIO_ReadOutputData(GLCD_DAT_PORT) & 0xFF00;
    GPIO_Write(GLCD_DAT_PORT, output | data);

    asset(GLCD_CMD_PORT, Pin_EN);       // Strobe
    GLCD_Delay(1);                      // 1.5µs mini
    reset(GLCD_CMD_PORT, Pin_EN);
    }

/*
// Lecture du registre de données du LCD
uint8_t GLCD_DataRead()
    {
    uint8_t data;
    GLCD_DAT_Pin_DIR = INPUT;           // LCD_DATA port in input mode

    asset(GLCD_CMD_PORT, Pin_RW);       // Read mode
    asset(GLCD_CMD_PORT, Pin_DI);       // Data mode

    asset(GLCD_CMD_PORT, Pin_EN);       // strobe
    GLCD_Delay(1);                      // 1.5µs mini
    reset(GLCD_CMD_PORT, Pin_EN);
    GLCD_Delay(1);                      // 1.5µs mini
    data = GLCD_DAT_INP;
    GLCD_DAT_Pin_DIR = OUTPUT;

    return(data);                       // return the data read
    }
*/

// Attente Busy Flag (Repasse à 0 qd LCD prêt)
void GLCD_WaitBusy()
    {
    GLCD_Delay(20);     // a rallonger en cas de bugs aléatoires
    return;

// Cette méthode pose parfois des problèmes, même en limitant à GPIO_Pin_7
// et même en ajoutant une pull-Up sur cette entrée
// Cf nombreux problèmes signalés sur le Net avec le busy flag des KS0108

/*
    uint16_t input= 0;
    GPIO_InitTypeDef GPIO_InitStructure;

    //  GLCD_DAT_DIR = INPUT;         	 	            // LCD_DATA port in input mode
    GPIO_InitStructure.GPIO_Pin = Pin_DATA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;

    GPIO_Init(GLCD_DAT_PORT, &GPIO_InitStructure);

    reset(GLCD_CMD_PORT, Pin_DI);       // Instruction mode
    asset(GLCD_CMD_PORT, Pin_RW);       // Read mode
    asset(GLCD_CMD_PORT, Pin_EN);       // Strobe

    do
        {
        input= GPIO_ReadInputData(GLCD_DAT_PORT);
        }
    while (input & LCD_BUSY);       // LCD busy ?

    reset(GLCD_CMD_PORT, Pin_EN);

    //    GLCD_DAT_DIR = OUTPUT;
    GPIO_InitStructure.GPIO_Pin = Pin_DATA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;

    GPIO_Init(GLCD_DAT_PORT, &GPIO_InitStructure);
*/
    }

// Déplacement dans l'écran
void GLCD_Move(uint8_t side, uint8_t page, uint8_t col)
	{
    GLCD_SelectSide(side);              // Sélection coté

    GLCD_Command(PAGE_0 | page);     	// Sélection page
    GLCD_Command(COL_0 | col);          // Sélection colonne
	}

// Ecriture d'un caractère en FontSystem5x8
void GLCD_WriteChar(uint8_t c)
    {
    uint8_t i, j;
    i = (c - 0x20);
          for (j = 0; j < 5; j++)
          {
          GLCD_DataWrite(FontSystem5x8[i][j]);
		  // Incrémentation auto de la colonne à chaque écriture
          }
    }

// Ecriture d'une chaine en FontSystem5x8
void GLCD_Write(uint8_t * chaine)
    {
    int n;

	for (n= 0 ; chaine[n] != 0 ; n++)
		GLCD_WriteChar(chaine[n]);
    }


// Affichage d'une variale de type entier
void GLCD_ShowInt(int entier)
	{
   	int compteur = 0;
   	uint8_t caracteres[10];    // Un entier comporte un maximum de 10 digits

   	// Si l'entier vaut 0, on l'affiche tout de suite
   	if(entier == 0)
   		{
      	GLCD_WriteChar('0');
   		}
   	else
   		{
      	// Gestion de l'affichage du signe moins dans le cas d'un nombre négatif
      	if(entier < 0)
      		{
        	GLCD_WriteChar('-');
        	entier = -entier;       // Repassage du nombre en positif
      		}

      	// Construction de la chaîne de caractères correspondante
      	// Principe: divisions successives par 10 pour récupérer chaque digit
      	// jusqu'à arriver à 0.
      	while(entier>0 && compteur <= 10)
      		{
        	caracteres[compteur] = entier % 10;  // Récupération du digit actuel
        	entier /= 10;   // Passage au digit suivant
        	compteur++;
      		}

      	// Affichage de la chaîne (on commence par les derniers digit
      	// puisque la construction s'est faites dans le sens inverse)
      	for(;compteur>0; compteur--)
      		{
        	GLCD_WriteChar('0' + caracteres[compteur-1]);
      		}
   		}
	}


// Effacement écran GLCD
void GLCD_ClearScreen(void)
    {
    uint8_t page;
    uint8_t colonne;

    GLCD_SelectSide(BOTH);                         // Les deux en même temps
    // Effacement des 8 pages d'un demi LCD
    for (page = 0; page < 8; page++)
    	{
        GLCD_Command(PAGE_0 | page);       		    // Sélection page
        GLCD_Command(COL_0);             		    // Sélection colonne 0
        for (colonne = 0; colonne <64; colonne++)
        	{
			GLCD_DataWrite (0x00);
			// Incrémentation auto de la colonne à chaque écriture
            }
        }
    }

// Initialisation des Ports du GLCD
void GLCD_InitPorts(void)
    {
    // Port de données en sorties
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(GLCD_DAT_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(GLCD_CMD_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = Pin_DATA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;

    GPIO_Init(GLCD_DAT_PORT, &GPIO_InitStructure);

    // Port de commande en sorties
    GPIO_InitStructure.GPIO_Pin = Pin_DI | Pin_RW | Pin_EN | Pin_CS1 | Pin_CS2 | Pin_RST;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;

    GPIO_Init(GLCD_CMD_PORT, &GPIO_InitStructure);

    // Data niveau 0, Cmd niveau 1
    GPIO_SetBits(GLCD_CMD_PORT,Pin_CMD);
    GPIO_ResetBits(GLCD_DAT_PORT, Pin_DATA);
    }

// Initialisation du GLCD
void GLCD_InitDisplay(void)
    {
    GLCD_InitPorts();

    GPIO_ResetBits(GLCD_DAT_PORT, Pin_DATA);
    GPIO_ResetBits(GLCD_CMD_PORT, Pin_CMD);

    reset(GLCD_CMD_PORT, Pin_RST);
    GLCD_Delay(150);
    asset(GLCD_CMD_PORT, Pin_RST);

    GLCD_SelectSide(RIGHT);
    GLCD_Command(DISPLAY_OFF);      // Display OFF
    GLCD_Command(START_LINE);
    GLCD_Command(PAGE_0);
    GLCD_Command(COL_0);
    GLCD_Command(DISPLAY_ON);       // Display ON

    GLCD_SelectSide(LEFT);
    GLCD_Command(DISPLAY_OFF);      // Display OFF
    GLCD_Command(START_LINE);
    GLCD_Command(PAGE_0);
    GLCD_Command(COL_0);
    GLCD_Command(DISPLAY_ON);       // Display ON
    }


