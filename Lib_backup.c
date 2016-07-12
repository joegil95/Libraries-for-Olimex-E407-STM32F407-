/**
 * Copyright (C) JG 2016
 */


/// Backup Domain is a 4 KB sram memory at address 0x4002400
/// which is not erased on reboot
/// and which is saved via VBat when power is down


#include "stm32f4xx.h"

#include "lib_backup.h"


 /**
  * @brief  Initialize Backup domain
  * @param  None
  * @retval None
  */
void Libbackup_Init(void)
    {
    // activer PWR clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    // autoriser acces au Backup Domain
    PWR_BackupAccessCmd(ENABLE);
    // activer Backup Domain clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
    // activer regulateur (si VBat presente)
    PWR_BackupRegulatorCmd(ENABLE);
    // attendre activation regulateur
    while(PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET);
    }


/**
  * @brief  Read byte from Backup Domain
  * @param  addresse = offset from Backup Domain start address
  * @retval byte read at this address or 0 if error
  */
uint8_t Libbackup_Read8(uint16_t address)
    {
    uint8_t data= 0;

    if (address < 4096)
        data= *(uint8_t *) (BKPSRAM_BASE + address);
    return data;
    }

/**
  * @brief  Read word from Backup Domain
  * @param  addresse = offset from Backup Domain start address
  * @retval word read at this address or 0 if error
  */
uint16_t Libbackup_Read16(uint16_t address)
    {
    uint16_t data= 0;

    if (address < 4096)
        data= *(uint16_t *) (BKPSRAM_BASE + address);
    return data;
    }

/**
  * @brief  Read double word from Backup Domain
  * @param  addresse = offset from Backup Domain start address
  * @retval double word read at this address or 0 if error
  */
uint32_t Libbackup_Read32(uint16_t address)
    {
    uint32_t data= 0;

    if (address < 4096)
        data= *(uint32_t *) (BKPSRAM_BASE + address);
    return data;
    }


 /**
  * @brief  Write byte to Backup Domain
  * @param  addresse = offset from Backup Domain start address
  * @param  data = byte to write at this address
  * @retval None
  */
void Libbackup_Write8(uint16_t address, uint8_t data)
    {
    if (address < 4096)
        *(uint8_t *) (BKPSRAM_BASE + address)= data;
    }

/**
  * @brief  Write word to Backup Domain
  * @param  addresse = offset from Backup Domain start address
  * @param  data = word to write at this address
  * @retval None
  */
void Libbackup_Write16(uint16_t address, uint16_t data)
    {
    if (address < 4096)
        *(uint16_t *) (BKPSRAM_BASE + address)= data;
    }

/**
  * @brief  Write double word to Backup Domain
  * @param  addresse = offset from Backup Domain start address
  * @param  data = double word to write at this address
  * @retval None
  */
void Libbackup_Write32(uint16_t address, uint32_t data)
    {
    if (address < 4096)
        *(uint32_t *) (BKPSRAM_BASE + address)= data;
    }
