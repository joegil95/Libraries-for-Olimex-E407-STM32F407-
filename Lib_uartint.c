/**
 * Copyright (C) Tilen Majerle 2014, and JG 2016
 */

#include "lib_uartint.h"
#include "misc.h"

// Internal USART struct
typedef struct
    {
	uint8_t *Buffer;
	uint16_t Size;
	uint16_t Num;
	uint16_t In;
	uint16_t Out;
	uint8_t Initialized;
    } LibUartInt_t;


// Internal buffers
#define LIBUARTINT_BUFFER_SIZE 				32
#define LIBUSARTINT_BUFFER_SIZE 			32

uint8_t LibUsartInt1_Buffer[LIBUARTINT_BUFFER_SIZE];
uint8_t LibUsartInt2_Buffer[LIBUARTINT_BUFFER_SIZE];
uint8_t LibUsartInt3_Buffer[LIBUARTINT_BUFFER_SIZE];
uint8_t LibUsartInt4_Buffer[LIBUSARTINT_BUFFER_SIZE];
uint8_t LibUsartInt5_Buffer[LIBUSARTINT_BUFFER_SIZE];
uint8_t LibUsartInt6_Buffer[LIBUARTINT_BUFFER_SIZE];

LibUartInt_t LibUsartInt1 = {LibUsartInt1_Buffer, LIBUARTINT_BUFFER_SIZE, 0, 0, 0, 0};
LibUartInt_t LibUsartInt2 = {LibUsartInt2_Buffer, LIBUARTINT_BUFFER_SIZE, 0, 0, 0, 0};
LibUartInt_t LibUsartInt3 = {LibUsartInt3_Buffer, LIBUARTINT_BUFFER_SIZE, 0, 0, 0, 0};
LibUartInt_t LibUsartInt4 = {LibUsartInt4_Buffer, LIBUSARTINT_BUFFER_SIZE, 0, 0, 0, 0};
LibUartInt_t LibUsartInt5 = {LibUsartInt5_Buffer, LIBUSARTINT_BUFFER_SIZE, 0, 0, 0, 0};
LibUartInt_t LibUsartInt6 = {LibUsartInt6_Buffer, LIBUARTINT_BUFFER_SIZE, 0, 0, 0, 0};

LibUartInt_t* LibUartInt_GetUsart(USART_TypeDef* USARTx);


/**
 * @brief  Initialize USART with interrupts
 * @param  USARTx = USART1, USART2, USART3, UART4, UART5 or USART6
 * @param  baudrate = standard baudrate value from 1200 to 115200
 * @param  datalen = USART_WordLength_8b or USART_WordLength_9b
 * @param  parity = USART_Parity_No or USART_Parity_Even or USART_Parity_Odd
 * @param  stops = USART_StopBits_1 or USART_StopBits_2
 * @retval None
 */
void LibUartInt_Init(USART_TypeDef* USARTx, uint32_t baudrate, uint16_t datalen, uint16_t parity, uint16_t stops)
    {
	USART_InitTypeDef   USART_InitStruct;
    NVIC_InitTypeDef	NVIC_InitStruct;

	USART_InitStruct.USART_BaudRate = baudrate;

    USART_InitStruct.USART_HardwareFlowControl = LibUartInt_HardwareFlowControl_None;  // no flow control
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                    // full-duplex TX + RX
	USART_InitStruct.USART_Parity = parity;
	USART_InitStruct.USART_StopBits = stops;
	USART_InitStruct.USART_WordLength = datalen;

	if (USARTx == USART1)
        {
		LibUsartInt1_InitPins(LibUartInt_PinsPack_1);
		NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
        }
	if (USARTx == USART2)
        {
		LibUsartInt2_InitPins(LibUartInt_PinsPack_1);
		NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
        }
	if (USARTx == USART3)
        {
		LibUsartInt3_InitPins(LibUartInt_PinsPack_1);
		NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
        }
	if (USARTx == UART4)
        {
		LibUsartInt4_InitPins(LibUartInt_PinsPack_1);
		NVIC_InitStruct.NVIC_IRQChannel = UART4_IRQn;
        }
	if (USARTx == UART5)
        {
		LibUsartInt5_InitPins(LibUartInt_PinsPack_1);
		NVIC_InitStruct.NVIC_IRQChannel = UART5_IRQn;
		}
	if (USARTx == USART6)
        {
		LibUsartInt6_InitPins(LibUartInt_PinsPack_1);
		NVIC_InitStruct.NVIC_IRQChannel = USART6_IRQn;
        }

    // We are initialized
    LibUartInt_t* u = LibUartInt_GetUsart(USARTx);
    u->Initialized = 1;

	// Disable and Deinit
	USART_Cmd(USARTx, DISABLE);
	USART_DeInit(USARTx);

	// ReInit and Enable
	USART_Init(USARTx, &USART_InitStruct);
	USART_Cmd(USARTx, ENABLE);

	// Enable RX interrupt
	USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);

	// Initialize NVIC
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = LibUartInt_NVIC_PRIORITY;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = LibUartInt_GetSubPriority(USARTx);
	NVIC_Init(&NVIC_InitStruct);
    }


/**
 * @brief  Get character from UART buffer (non blocking function)
 * @param  USARTx = USART1, USART2, USART3, UART4, UART5 or USART6
 * @retval Received character if any, or (int16_t) -1 if none
 */
int16_t  LibUartInt_Getc(USART_TypeDef* USARTx)
    {
	int16_t c = -1;

	LibUartInt_t* u = LibUartInt_GetUsart(USARTx);

	// Check if we have any data in buffer
	if (u->Num > 0)
        {
		if (u->Out == u->Size)
			u->Out = 0;

		c = *(u->Buffer + u->Out);
		u->Out++;
		u->Num--;
        }
	return c;
    }


/**
 * @brief  Send character via UART
 * @param  USARTx = USART1, USART2, USART3, UART4, UART5 or USART6
 * @param  c = character to be send
 * @retval None
 */
void LibUartInt_Putc(USART_TypeDef* USARTx, volatile char c)
    {
	// Wait to be ready to send
	while (!(USARTx->SR & USART_FLAG_TXE));
	// Send data
	USARTx->DR = (uint16_t)(c & 0x01FF);
    }


/**
 * @brief  Check if internal UART buffer is empty
 * @param  USARTx = USART1, USART2, USART3, UART4, UART5 or USART6
 * @retval false (0) if buffer not empty, true (> 0) if fuffer empty
 */
uint8_t LibUartInt_BufferEmpty(USART_TypeDef* USARTx)
    {
	LibUartInt_t* u = LibUartInt_GetUsart(USARTx);
	return (u->Num == 0);
    }


/**
 * @brief  Check if internal UART buffer is full
 * @param  USARTx = USART1, USART2, USART3, UART4, UART5 or USART6
 * @retval false (0) if buffer not full, true (> 0) if fuffer full
 */
uint8_t LibUartInt_BufferFull(USART_TypeDef* USARTx)
    {
	LibUartInt_t* u = LibUartInt_GetUsart(USARTx);
	return (u->Num == u->Size);
    }


/**
 * @brief  Clear internal UART buffer
 * @param  USARTx = USART1, USART2, USART3, UART4, UART5 or USART6
 * @retval None
 */
void LibUartInt_ClearBuffer(USART_TypeDef* USARTx)
    {
	LibUartInt_t* u = LibUartInt_GetUsart(USARTx);

	u->Num = 0;
	u->In = 0;
	u->Out = 0;
    }


/**
 * Internal functions
 */
LibUartInt_t* LibUartInt_GetUsart(USART_TypeDef* USARTx)
    {
	LibUartInt_t* u;

	if (USARTx == USART1) u = &LibUsartInt1;
	if (USARTx == USART2) u = &LibUsartInt2;
	if (USARTx == USART3) u = &LibUsartInt3;
	if (USARTx == UART4) u = &LibUsartInt4;
	if (USARTx == UART5) u = &LibUsartInt5;
	if (USARTx == USART6) u = &LibUsartInt6;

	return u;
    }


uint8_t LibUartInt_GetSubPriority(USART_TypeDef* USARTx)
    {
	uint8_t u;

	if (USARTx == USART1) u = 0;
	if (USARTx == USART2) u = 1;
	if (USARTx == USART3) u = 2;
	if (USARTx == UART4) u = 4;
	if (USARTx == UART5) u = 5;
	if (USARTx == USART6) u = 6;

	return u;
    }


void LibUsartInt1_InitPins(LibUartInt_PinsPack_t pinspack)
    {
	// Enable clock for USART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// Init pins
	if (pinspack == LibUartInt_PinsPack_1)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOA, GPIO_Pin_9 | GPIO_Pin_10, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_USART1);
        }
    if (pinspack == LibUartInt_PinsPack_2)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOB, GPIO_Pin_6 | GPIO_Pin_7, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_USART1);
        }
    }


void LibUsartInt2_InitPins(LibUartInt_PinsPack_t pinspack)
    {
	// Enable clock for USART2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// Init pins
	if (pinspack == LibUartInt_PinsPack_1)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOA, GPIO_Pin_2 | GPIO_Pin_3, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_USART2);
        }
    if (pinspack == LibUartInt_PinsPack_2)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOD, GPIO_Pin_5 | GPIO_Pin_6, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_USART2);
        }
    }


void LibUsartInt3_InitPins(LibUartInt_PinsPack_t pinspack)
    {
	// Enable clock for USART3
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	// Init pins
	if (pinspack == LibUartInt_PinsPack_1)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOB, GPIO_Pin_10 | GPIO_Pin_11, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_USART3);
        }
    if (pinspack == LibUartInt_PinsPack_2)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOC, GPIO_Pin_10 | GPIO_Pin_11, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_USART3);
        }
    if (pinspack == LibUartInt_PinsPack_3)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOD, GPIO_Pin_8 | GPIO_Pin_9, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_USART3);
        }
    }


void LibUsartInt4_InitPins(LibUartInt_PinsPack_t pinspack)
    {
	// Enable clock for UART4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	// Init pins
	if (pinspack == LibUartInt_PinsPack_1)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOA, GPIO_Pin_0 | GPIO_Pin_1, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_UART4);
        }
    if (pinspack == LibUartInt_PinsPack_2)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOC, GPIO_Pin_10 | GPIO_Pin_11, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_UART4);
        }
    }


void LibUsartInt5_InitPins(LibUartInt_PinsPack_t pinspack)
    {
	// Enable clock for UART5
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

	// Init pins
	if (pinspack == LibUartInt_PinsPack_1)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	// by JG
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOC, GPIO_Pin_12, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_UART5);
		LibGPIO_InitAlternate(GPIOD, GPIO_Pin_2, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_UART5);
        }
    }


void LibUsartInt6_InitPins(LibUartInt_PinsPack_t pinspack)
    {
	// Enable clocks for USART6
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	// Init pins
	if (pinspack == LibUartInt_PinsPack_1)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOC, GPIO_Pin_6 | GPIO_Pin_7, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_USART6);
        }
    if (pinspack == LibUartInt_PinsPack_2)
        {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);	// by JG
		LibGPIO_InitAlternate(GPIOG, GPIO_Pin_14 | GPIO_Pin_9, GPIO_OType_PP, GPIO_PuPd_UP, LibGPIO_Speed_High, GPIO_AF_USART6);
        }
    }


void LibUartInt_InsertBuffer(USART_TypeDef* USARTx, uint8_t c)
    {
    LibUartInt_t* u= LibUartInt_GetUsart(USARTx);

	// Still available space in buffer
	if (u->Num < u->Size)
        {
		// Check overflow
		if (u->In == u->Size)
            {
			u->In = 0;
            }
		// Add to buffer
		u->Buffer[u->In] = c;
		u->In++;
		u->Num++;
        }
    }

