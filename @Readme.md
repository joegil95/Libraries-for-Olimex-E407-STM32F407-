# STM32F407
Librairies pour carte Olimex-E407 à base de STM32F407

Ces librairies et projets sont destinés à la carte Olimex E-407.
Elles fonctionnent sous EmIde 2.20, qui a été utilisé avec un émulateur J-Link.
Le compilateur sous-jacent est GccArm.

Les librairies sont pour certaines dérivées de celles écrite par Tilen Marjele
pour la carte STM32F4-Discovery, d'autres ont été développées spécifiquement.
Elles s'appuient généralement sur les librairies standard fournies par ST-Microelectronics 
pour les STM32F4.

Un fichier startup.s spécifique (ci-joint) est utilisé pour la gestion des interruptions,
à placer dans le dossier Setup du projet EmIde.
Chaque interruption utilisée doit être déclarée par le nom de son gestionnaire
dans Project > Build options > #defines (global)
Ajouter par exemple TIM2_HANDLER pour activer la routine d'interruption du Timer 2,
qui doit se nommer impérativement void TIM2_Handler()

Certaines applications nécéssitant des horloges précises (Ethernet, SD-Card, USB...)
nécessitent de configurer convenablement les PLL dans system_stm32f4xx.c qui doit
alors être inclus dans le projet

Joegil95