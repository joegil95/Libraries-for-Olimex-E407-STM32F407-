/**
 * UART Library for STM32F4 with interrupts
 */


#ifndef LIBUARTINT_H
#define LIBUARTINT_H 250

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

/*
                PINSPACK 1      PINSPACK 2      PINSPACK 3
    U(S)ARTX    TX     RX       TX     RX       TX     RX

    USART1      PA9    PA10     PB6    PB7      -      -
    USART2      PA2    PA3      PD5    PD6      -      -
    USART3      PB10   PB11     PC10   PC11     PD8    PD9
    UART4       PA0    PA1      PC10   PC11     -      -
    UART5       PC12   PD2      -      -        -      -
    USART6       PC6    PC7     PG14   PG9      -      -
*/

#include "stm32f4xx.h"

#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"

#include "lib_gpio.h"
#include "misc.h"


typedef enum
    {
	LibUartInt_PinsPack_1,
	LibUartInt_PinsPack_2,
	LibUartInt_PinsPack_3,
    } LibUartInt_PinsPack_t;

typedef enum
    {
	LibUartInt_HardwareFlowControl_None = 0x0000,     // No flow control
	LibUartInt_HardwareFlowControl_RTS = 0x0100,      // RTS flow control
	LibUartInt_HardwareFlowControl_CTS = 0x0200,      // CTS flow control
	LibUartInt_HardwareFlowControl_RTS_CTS = 0x0300   // RTS and CTS flow controls
    } LibUartInt_HardwareFlowControl_t;


// NVIC Global Priority
#define LibUartInt_NVIC_PRIORITY	0x06


void LibUartInt_Init(USART_TypeDef* USARTx, uint32_t baudrate, uint16_t datalen, uint16_t parity, uint16_t stops);

void LibUartInt_Putc(USART_TypeDef* USARTx, volatile char c);
int16_t LibUartInt_Getc(USART_TypeDef* USARTx);

uint8_t LibUartInt_BufferEmpty(USART_TypeDef* USARTx);
uint8_t LibUartInt_BufferFull(USART_TypeDef* USARTx);
void LibUartInt_ClearBuffer(USART_TypeDef* USARTx);

void LibUartInt_InsertBuffer(USART_TypeDef* USARTx, uint8_t c);

void LibUsartInt1_InitPins(LibUartInt_PinsPack_t pinspack);
void LibUsartInt2_InitPins(LibUartInt_PinsPack_t pinspack);
void LibUsartInt3_InitPins(LibUartInt_PinsPack_t pinspack);
void LibUsartInt4_InitPins(LibUartInt_PinsPack_t pinspack);
void LibUsartInt5_InitPins(LibUartInt_PinsPack_t pinspack);
void LibUsartInt6_InitPins(LibUartInt_PinsPack_t pinspack);

uint8_t LibUartInt_GetSubPriority(USART_TypeDef* USARTx);


/**
 * interrupt handler on receive to be defined by user
 */
void USART1_Handler(void);
void USART2_Handler(void);
void USART3_Handler(void);
void UART4_Handler(void);
void UART5_Handler(void);
void USART6_Handler(void);

/**
 * interrupt handler template
 */

/*
void USART6_Handler(void)
    {
	// Check if interrupt on receive
	if (USART_GetITStatus(USART1, USART_IT_RXNE))
        {
        //Put received data into internal buffer
		LibUartInt_INT_InsertToBuffer(USART1, USART1->DR);
		}
    }
 */


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
