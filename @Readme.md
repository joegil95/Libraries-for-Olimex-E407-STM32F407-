# STM32F407
Librairies pour carte Olimex-E407 � base de STM32F407

Ces librairies et projets sont destin�s � la carte Olimex E-407.
Elles fonctionnent sous EmIde 2.20, qui a �t� utilis� avec un �mulateur J-Link.
Le compilateur sous-jacent est GccArm.

Les librairies sont pour certaines d�riv�es de celles �crite par Tilen Marjele
pour la carte STM32F4-Discovery, d'autres ont �t� d�velopp�es sp�cifiquement.
Elles s'appuient g�n�ralement sur les librairies standard fournies par ST-Microelectronics 
pour les STM32F4.

Un fichier startup.s sp�cifique (ci-joint) est utilis� pour la gestion des interruptions,
� placer dans le dossier Setup du projet EmIde.
Chaque interruption utilis�e doit �tre d�clar�e par le nom de son gestionnaire
dans Project > Build options > #defines (global)
Ajouter par exemple TIM2_HANDLER pour activer la routine d'interruption du Timer 2,
qui doit se nommer imp�rativement void TIM2_Handler()

Certaines applications n�c�ssitant des horloges pr�cises (Ethernet, SD-Card, USB...)
n�cessitent de configurer convenablement les PLL dans system_stm32f4xx.c qui doit
alors �tre inclus dans le projet

Joegil95