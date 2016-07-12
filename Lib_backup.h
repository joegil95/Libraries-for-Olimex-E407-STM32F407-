/**
 * Backup Domain Library for STM32F4xx devices
 */


#include "stm32f4xx.h"

#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"

void Libbackup_Init(void);

uint8_t Libbackup_Read8(uint16_t address);
uint16_t Libbackup_Read16(uint16_t address);
uint32_t Libbackup_Read32(uint16_t address);

void Libbackup_Write8(uint16_t address, uint8_t data);
void Libbackup_Write16(uint16_t address, uint16_t data);
void Libbackup_Write32(uint16_t address, uint32_t data);
