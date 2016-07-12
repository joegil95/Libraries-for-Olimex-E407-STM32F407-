/**
 * Copyright (C) JG 2016
 */

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

#include "Lib_lcd16x2.h"			 // Fichier header librairie LCD

//--------------------------------------------------------------------------

void LCD_SendCommand(char);
void LCD_SendChar(char);
void LCD_Send(char,int);
void LCD_Send4msb(char);
void LCD_Enable(void);

// Temporisations
void LCD_delay_us(unsigned us)	  // delais en +/- �s
	{
	unsigned d= us /5;

    for ( ; d > 0 ; d--);
	}

void LCD_delay_ms(unsigned ms) 	  // delais en +/- ms
	 {
	 unsigned long d= 1000*ms;    //*10

	 for ( ; d > 0 ; d--);
	 }


// Envoi commande
void LCD_SendCommand(char commande)
{
    LCD_Send(commande,0);
}

// Envoi caractere
void LCD_SendChar(char caractere)
{
    LCD_Send(caractere,1);
}

/// G�re l'�criture d'un octet vers le LCD
void LCD_Send(char donnee,int type)
{
    // Activation de RS si type = 1 (donnee)
    if(type==1)
        GPIO_SetBits( GPIO_LCD, GPIO_Pin_2 );

    // Ecriture des 4 MSB
    LCD_Send4msb(donnee);

    // Ecriture des 4 LSB
    LCD_Send4msb(donnee << 4);

    // Desactivation RS
    GPIO_ResetBits( GPIO_LCD, GPIO_Pin_2 );

    // Attente execution
    LCD_delay_ms(6);
}

// G�re l'enable du LCD
void LCD_Enable(void)
{
    GPIO_SetBits(GPIO_LCD, GPIO_Pin_3 );

    // petite tempo
    LCD_delay_us(10);

    GPIO_ResetBits(GPIO_LCD, GPIO_Pin_3);
}


// G�re l'�criture des 4 MSB vesr le LCD
void LCD_Send4msb(char donnee)
{
    // Reset bits de donn�es
    GPIO_ResetBits(GPIO_LCD, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 );

    // Ecriture des 4 MSB
    if (donnee & (1 << 4))
        GPIO_SetBits( GPIO_LCD, GPIO_Pin_4 );

    if (donnee & (1 << 5))
        GPIO_SetBits( GPIO_LCD, GPIO_Pin_5 );

    if (donnee & (1 << 6))
        GPIO_SetBits( GPIO_LCD, GPIO_Pin_6 );

    if (donnee & (1 << 7))
        GPIO_SetBits( GPIO_LCD, GPIO_Pin_7 );


    // Validation donnee par basculement de l'enable
    LCD_Enable();
}

// Initialisation du LCD
void LCD_Init(void)
{
    // Port LCD en sortie
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitTypeDef gpioInit;
    gpioInit.GPIO_Mode = GPIO_Mode_OUT;
    gpioInit.GPIO_OType = GPIO_OType_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_2 | GPIO_Pin_3;
    gpioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpioInit.GPIO_Speed = GPIO_Speed_2MHz;      // doit etre < frequence du quartz
    GPIO_Init(GPIO_LCD, &gpioInit);

    // Attente de l'initialisation interne du circuit LCD
    LCD_delay_ms(20);

    GPIO_ResetBits( GPIO_LCD, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 );

    // RS � 0
    GPIO_ResetBits( GPIO_LCD, GPIO_Pin_2 );

    LCD_delay_ms(10);

    // Configuration initiale : etape 1
    LCD_Send4msb(0x30);

    LCD_delay_ms(5);

    // Configuration initiale : etape 2
    LCD_Send4msb(0x30);
    LCD_delay_us(160);

    // Configuration initiale : etape 3
    LCD_Send4msb(0x30);
    LCD_delay_us(160);

    // Passage en mode 4 bits
    LCD_Send4msb(0x20);
    LCD_delay_ms(5);

    // Envoi des commandes de param�trage
    LCD_SendCommand(0x28);		// 4 bits - 2 lignes - Police 5x7
    LCD_Config(0,0,0);   	// Display off - Cursor off - Blinking off
    LCD_Erase();
    LCD_InsertMode(1,0);   	// Cursor increase - Display not shifted
    LCD_Config(1,0,0);   	//  Display on - Cursor off - Blinking off
}

// Efface le contenu de l'ecran LCD
void LCD_Erase(void)
{
    LCD_SendCommand(LCD_CMD_EFF);
}

// Renvoie le curseur � la position initiale
void LCD_Home(void)
{
    LCD_SendCommand(LCD_CMD_HOME);
}

// S�lection du mode d'insertion des caract�res sur le LCD
void LCD_InsertMode(int direction_curseur,int inversion_affichage)
{
    LCD_SendCommand(LCD_CMD_ENTRY_MODE | (inversion_affichage << LCD_BIT_ENTRY_SHIFT) | (direction_curseur << LCD_BIT_ENTRY_INC));
}

// Configuration affichage LCD
void LCD_Config(int affichage_actif,int curseur_actif,int clignotement_actif)
{
    LCD_SendCommand(
        LCD_CMD_DISPLAY
        | (affichage_actif << LCD_BIT_DISP_DISP)
        | (curseur_actif << LCD_BIT_DISP_CURS)
        | (clignotement_actif << LCD_BIT_DISP_BLINK)
    );
}

// D�placement affichage ou curseur
void LCD_Move(int type,int sens)
{
    LCD_SendCommand(
        LCD_CMD_SHIFT
        | (type << LCD_BIT_SHIFT_TYPE)
        | (sens << LCD_BIT_SHIFT_SENS)
    );
}

// D�placement du curseur de n caract�res vers la droite
void LCD_MoveRight(int nombre_caracteres)
{
    int i;

    for(i=0; i<nombre_caracteres; i++)
        LCD_Move(0,1);
}

// D�placement du curseur de n caract�res vers la gauche
void LCD_MoveLeft(int nombre_caracteres)
	{
   	int i;

   	for(i=0;i<nombre_caracteres;i++)
  		LCD_Move(0,0);
	}

// D�placement de n caract�res vers la droite des donn�es � partir du curseur
void LCD_ShiftRight(int nombre_caracteres)
	{
   	int i;

   	for(i=0;i<nombre_caracteres;i++)
      	LCD_Move(1,1);
	}

// D�placement de n caract�res vers la gauche des donn�es � partir du curseur
void LCD_ShiftLeft(int nombre_caracteres)
	{
   	int i;

   	for(i=0;i<nombre_caracteres;i++)
      	LCD_Move(1,0);
	}

// Ecriture d'une cha�ne de caract�re sur le LCD
void LCD_Write(char *chaine)
	{
   	int i;

   	if (chaine == 0)		// Si le pointeur pass� en param�tre n'est pas valide
      	return;      		// => sortie de la fonction

   	for (i=0;i< strlen(chaine);i++)
      	LCD_SendChar(chaine[i]);
	}

// D�placement du curseur
void LCD_MoveCursor(int y,int x)
	{
   	int adresse;

   	if ((y < 1) || (y > 2) || (x < 1) || (x > 16))
   		{
      	return;
		// Si le num�ro de caract�re/ligne n'est pas bon => sortie de la fonction
   		}

   	// Ligne 1
   	if (y == 1)
   		{
      	adresse = LCD_LIGNE_1 + (x - 1);
   		}
   	// Ligne 2
   	else
   		{
      	adresse = LCD_LIGNE_2 + (x - 1);
   		}

   	LCD_SendCommand(LCD_CMD_SET_DDRAM | adresse);
	}

// Affiche une variale de type entier sur l'�cran LCD
void LCD_ShowInt(int entier)
	{
   	int compteur = 0;
   	char caracteres[10];  // Un entier comporte un maximum de 10 digits

   	// Si l'entier vaut 0, on l'affiche tout de suite
   	if(entier == 0)
   		{
      	LCD_SendChar('0');
   		}
   	else
   		{
      	// Gestion de l'affichage du signe moins dans le cas d'un nombre n�gatif
      	if(entier < 0)
      		{
        	LCD_SendChar('-');
        	entier = -entier;   // Repassage du nombre en positif
      		}

      	// Construction de la cha�ne de caract�res correspondante
      	// Principe: divisions successives par 10 pour r�cup�rer chaque digit
      	// jusqu'� arriver � 0.
      	while(entier>0 && compteur <= 10)
      		{
        	caracteres[compteur] = entier % 10;  // R�cup�ration du digit actuel
        	entier /= 10;   // Passage au digit suivant
        	compteur++;
      		}

      	// Affichage de la cha�ne (on commence par les derniers digit
      	// puisque la construction s'est faites dans le sens inverse)
      	for(;compteur>0; compteur--)
      		{
        	LCD_SendChar('0' + caracteres[compteur-1]);
      		}
   		}
	}

// Affiche une variable de type double � l'�cran avec un nombre de d�cimales d�fini
void LCD_ShowDouble(double vdouble, int nb_decimales)
	{
   	unsigned long fraction;
   	unsigned long fraction_successives;
   	unsigned long multiple = 1;
   	int nb_decimales_affichees = nb_decimales - 1;

   	// Etape 1 : Affichage de la valeur enti�re
   	LCD_ShowInt((int) vdouble);

   	// Etape 2 : Affichage de la partie d�cimale
   	// Principe : Multiplication successive par 10 pour construire le nombre entier
   	// correspondant aux d�cimales

   	if (nb_decimales > 0)
   		{
      	// Affichage de la virgule
      	LCD_SendChar(',');

      	// R�cup�ration du premier diviseur
      	while(nb_decimales--)
      		{
        	multiple *= 10;
      		}

      	// Toute la partie d�cimale est transform�e en partie enti�re
      	// Nombre positif
      	if (vdouble >= 0)
      		{
        	fraction = (vdouble - (int) vdouble) * multiple;
      		}
      	// Nombre n�gatif
      	else
      		{
        	fraction = ((int) vdouble - vdouble) * multiple;
      		}

      	fraction_successives = fraction;

      	// V�rification si besoin de rajouter des z�ros apr�s la virgule
      	// avant d'afficher la partie d�cimale
      	while(fraction_successives /= 10)
      		{
        	nb_decimales_affichees--;
      		}

      	// Affichage des z�ros si besoin
      	while(nb_decimales_affichees--)
      		{
        	LCD_SendChar('0');
      		}

      	// Affichage de la partie d�cimale
      	LCD_ShowInt((int) fraction);
   		}
	}
