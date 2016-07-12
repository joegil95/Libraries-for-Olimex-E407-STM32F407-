/**
 * Copyright (C) JG 2016 and Chan (Fatfs)
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

#ifdef SD_DMA_MODE
#include "misc.h"
#include "interrupts.h"
#endif

#include "stm32f4x7_sdlow.h"
#include "stm32f4x7_sdcard.h"

#include "Lib_sdfatfs.h"

#include <string.h>

#ifdef SD_DMA_MODE
#include "misc.h"
#include "interrupts.h"

void SD_SetInterrupts(void);
#endif

#define FAT16	16
#define FAT32	32

MasterBootRecord mbr;
BootSector bootsec;

uint16_t PartType;			// type de partition= FAT16 ou FAT32
uint32_t PartOffset;		// offset de la partition par rapport au MBR
uint32_t PartSectorsNb;	    // taille de la partition en secteurs

uint32_t RootCluster; 		// numero du premier cluster de la racine
uint32_t RootSectorsNb;	    // nb de secteurs de la racine
uint32_t FirstDataSector;	// 1er secteur utile de la partition
uint32_t DataSectorsNb;	    // nb de secteurs utiles de la partition
uint32_t DataClustersNb;	// nb de clusters utiles de la partition

// structure qui contient le CSD, CID, capacité, type... apres SD_Init() OK
extern SD_CardInfo SDCardInfo;

SD_Error open_disk(unsigned part)     // ouverture du disque et selection d'une partition
	{
   	int p;
   	SD_Error res;
    PartTable * table[4]= {&mbr.MBR_Part1, &mbr.MBR_Part2, &mbr.MBR_Part3, &mbr.MBR_Part4};
    uint8_t nbused= 0, used[4]= {0, 0, 0, 0};
    char FatType[20]= "UNKNOWN";

    // lecture du secteur absolu 0 (MBR)
	res = SD_ReadBlock((uint8_t *) &mbr, 0, 512);
	if (res != SD_OK) return res;

    // Check if the Transfer is finished (DMA mode only)
    #ifdef SD_DMA_MODE
    res = SD_WaitReadOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
    #endif

    for (p= 0 ; p < 4 ; p++)	// recherche des partitions
    	{
		if (table[p]->PT_Type != 0)
        	{
            switch(table[p]->PT_Type)
				{
       			case 0x01:  strcpy(FatType, "FAT 12"); break;
				case 0x03:  strcpy(FatType, "FAT 16 +32 MO");
                            used[p]= 1;
                            nbused ++;
                            break;
        		case 0x05:  strcpy(FatType, "ETENDUE"); break;
				case 0x06:  strcpy(FatType, "FAT 16 -32 MO");
                            used[p]= 1;
                            nbused ++;
                            break;
        		case 0x07:  strcpy(FatType, "NTFS"); break;
        		case 0x0B:  strcpy(FatType, "FAT 32");
                            used[p]= 1;
                            nbused ++;
                            break;
				case 0x0C:  strcpy(FatType, "FAT 32 LBA");
                            used[p]= 1;
                            nbused ++;
                            break;
        		case 0x0E:  strcpy(FatType, "FAT 16 LBA");
                            used[p]= 1;
                            nbused ++;
                            break;
				case 0x0F:  strcpy(FatType, "ETENDUE LBA"); break;
        		}
            }
        }

    if (nbused == 0)                        // pas de partition valide 16 ou 32 bits
        return SD_UNSUPPORTED_HW;

    if ((part >= 4) || (used[part] != 1))   // numero de partition non valide
        return SD_INVALID_PARAMETER;

    PartOffset= table[part]->PT_StartOffset;        // offset du debut de la partition
    PartSectorsNb= table[part]->PT_TotalSec;        // nb total de secteurs

    return SD_OK;
    }


// ouverture de la partition prealablement selectionnée par open_disk()
SD_Error open_part()
	{
   	uint32_t totsec;
	SD_Error res;

	PartType= 0;

    // lecture du secteur relatif 0 de la partition (BOOT)
    // toujours multiplier les n° de secteurs par 512
    res = SD_ReadBlock((uint8_t *) &bootsec, 512*PartOffset, 512);
	if (res != SD_OK) return res;

   	if ((bootsec.BPB_FATSz16 == 0) && (strncmp(bootsec.u.Fat32.BS_FilSysType, "FAT32", 5) == 0))
       	{
		PartType= FAT32;
      	RootSectorsNb= 0;
		RootCluster= bootsec.u.Fat32.BPB_RootClus;

        /*
	   	printf("Lecteur: Partition FAT32, label du volume: %.11s\n", bootsec.u.Fat32.BS_VolLab);
   	   	printf("Taille en secteurs: %lu, secteurs par cluster: %u\n", bootsec.BPB_TotSec32, bootsec.BPB_SecPerClus);
	   	printf("Secteurs reserves: %u, root cluster: %u\n", bootsec.BPB_RsvdSecCnt, RootCluster);
   	   	printf("Nombre de FATs: %u, taille de chaque FAT en secteurs: %lu\n", bootsec.BPB_NumFATs, bootsec.u.Fat32.BPB_FATSz32);
        */

		// initialisation des variables globales utiles
	   	FirstDataSector = bootsec.BPB_RsvdSecCnt + (bootsec.BPB_NumFATs * bootsec.u.Fat32.BPB_FATSz32);
	   	DataSectorsNb = bootsec.BPB_TotSec32 - (bootsec.BPB_RsvdSecCnt + (bootsec.BPB_NumFATs * bootsec.u.Fat32.BPB_FATSz32));
	   	DataClustersNb = DataSectorsNb / bootsec.BPB_SecPerClus;

	   	return SD_OK;
       	}

	if (strncmp(bootsec.u.Fat16.BS_FilSysType, "FAT16", 5) == 0)
   	   	{
		PartType= FAT16;
		RootSectorsNb= ((bootsec.BPB_RootEntCnt * 32) + (bootsec.BPB_BytsPerSec -1)) / bootsec.BPB_BytsPerSec;
      	RootCluster= 1;	// symbolique

      	if (bootsec.BPB_TotSec16 != 0) totsec= bootsec.BPB_TotSec16;
      	else totsec= bootsec.BPB_TotSec32;		// si nb depasse 16 bits

        /*
	   	printf("Lecteur: Partition FAT16, label du volume: %.11s\n", bootsec.u.Fat16.BS_VolLab);
   		printf("Taille en secteurs: %lu, secteurs par cluster: %u\n", totsec, bootsec.BPB_SecPerClus);
	   	printf("Secteurs reserves: %u, root cluster: %u\n", bootsec.BPB_RsvdSecCnt, RootCluster);
   		printf("Nombre de FATs: %u, taille de chaque FAT en secteurs: %lu\n", bootsec.BPB_NumFATs, bootsec.BPB_FATSz16);
        */

		// initialisation des variables globales utiles
      	FirstDataSector = bootsec.BPB_RsvdSecCnt + (bootsec.BPB_NumFATs * bootsec.BPB_FATSz16)
      	+ RootSectorsNb;
		DataSectorsNb = bootsec.BPB_TotSec16 - (bootsec.BPB_RsvdSecCnt + (bootsec.BPB_NumFATs * bootsec.BPB_FATSz16)
      	+ RootSectorsNb);
   		DataClustersNb = DataSectorsNb / bootsec.BPB_SecPerClus;

   		return SD_OK;
      	}

    // sinon cette partition n'est apparemment pas de type FAT16 / FAT32
    return SD_UNSUPPORTED_HW;
   	}


/**
  * @brief  Initialize SD Card and read MBR
  * @param  None
  * @retval Status
  */
DSTATUS SD_disk_initialize()
    {
    SD_Error Status;

    Status= SD_Init();
    if(Status != SD_OK)
        return STA_NOINIT;

    Status= open_disk(0);   // verification et selection partition 0
    if (Status != SD_OK)
        return STA_NOINIT;

    return STA_READY;
    }


/**
  * @brief  Get SD Card status
  * @param  None
  * @retval Status
  */
DSTATUS SD_disk_status ()
    {
    return STA_READY;   // not really implemented
    }


/**
  * @brief  Read sectors from SD Card
  * @param  buff = Pointer to the read data buffer
  * @param  sector = Start sector number
  * @param  cnt = Number of sectors to read
  * @retval Status
  */
DRESULT SD_disk_read (BYTE* buff, DWORD sector, UINT cnt)
    {
    SD_Error Status= SD_OK;

    for (UINT n= 0 ; n < cnt ; n++)
        {
        // sector 0 must point on 1st sector of fat partition
        // so we need to skip MBR
        // sector numbers must be * 512 for sdcard library
        Status= SD_ReadBlock(buff + 512*n, 512*(sector+PartOffset), 512);
        if (Status != SD_OK)
            return RES_ERROR;
        }

    return RES_OK;
    }


/**
  * @brief  Write sectors to SD Card
  * @param  buff = Pointer to the write data buffer
  * @param  sector = Start sector number
  * @param  cnt = Number of sectors to write
  * @retval Status
  */
DRESULT SD_disk_write (BYTE* buff, DWORD sector, UINT cnt)
    {
    SD_Error Status= SD_OK;

    for (UINT n= 0 ; n < cnt ; n++)
        {
        // sector 0 must point on 1st sector of fat partition
        // so we need to skip MBR
        // sector numbers must be * 512 for sdcard library
        Status= SD_WriteBlock(buff + 512*n, 512*(sector+PartOffset), 512);
        if (Status != SD_OK)
            return RES_ERROR;
        }

    return RES_OK;
    }


/**
  * @brief  Get System date and time for file operations
  * @param  None
  * @retval Date and Time
  */
DWORD get_fattime ()
    {
    /*
    Fatfs format:
    bits 31:25 = Year from 1980 (0..127)
    bits 24:21 = Month (1..12)
    bits 20:16 = Day of the month(1..31)
    bits 15:11 = Hour (0..23)
    bits 10:05 = Minute (0..59)
    bits 04:00 = Second / 2 (0..29)
    */

    return 0;   // not implemented
    }


#ifdef SD_DMA_MODE
/* Interrupts functions -----------------------------------------------------*/
/**
  * @brief  Configures SDIO IRQ channel.
  * @param  None
  * @retval None
  */
void SD_SetInterrupts(void)
    {
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = SD_SDIO_DMA_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
    }

/**
  * @brief  This function handles SDIO interrupts (DMA Mode only)
  * @param  None
  * @retval None
  */
void SDIO_Handler(void)
    {
    // Process All SDIO Interrupt Sources
    SD_ProcessIRQSrc();
    }

/**
  * @brief  This function handles DMA2 Stream3 interrupts (DMA Mode only)
  * @param  None
  * @retval None
  */
void DMA2_S3_Handler(void)
    {
    // Process DMA2 Stream3 Interrupt Sources
    SD_ProcessDMAIRQ();
    }
#endif

