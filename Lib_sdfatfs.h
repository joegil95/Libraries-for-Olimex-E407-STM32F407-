/**
 * SD card Fat Library for STM32F4xx devices using Fatfs
 */

/**
 * NB: configure Fatfs options in ffconf.h
 */

#include "stm32f4xx.h"

#include "fatfs/integer.h"
#include "fatfs/diskio.h"
#include "fatfs/ff.h"

#define FAT16   16
#define FAT32   32

typedef struct       		// Entree table de partition 16 octets
	{
	uint8_t	    PT_State;		        // etat de la partition 0x00 = inactive 0x80 = active
	uint8_t     PT_StartHead;	        // tete de debut de la partition
	uint16_t    PT_StartSecCyl;	        // cylindre et secteur de debut de la partition
	uint8_t     PT_Type;		        // type de Partition
   	uint8_t     PT_EndHead;		        // tete de fin de la partition
	uint16_t    PT_EndSecCyl; 	        // cylindre et secteur de fin de la partition
	uint32_t    PT_StartOffset;	        // offset de debut de la partition en secteurs
	uint32_t    PT_TotalSec;		    // nb de secteurs de la partition
 	}
__attribute__ ((packed)) PartTable;     // pas d'alignement automatique (32 bits)

typedef struct				// Master Boot Record 512 octets
	{
	uint8_t     MBR_BootSequence[446];	    // Code de demarrage
	PartTable 	MBR_Part1;       	        // table 1ere partition (offset 0x1BE)
	PartTable 	MBR_Part2;       	        // table 2nde partition (offset 0x1CE)
  	PartTable 	MBR_Part3;       	        // table 3eme partition (offset 0x1DE)
  	PartTable 	MBR_Part4;       	        // table 4eme partition (offset 0x1EE)
	uint16_t    MBR_BootRecSig;		 	        // signature =	0xAA55
   	}
__attribute__ ((packed)) MasterBootRecord;  // pas d'alignement automatique (32 bits)

typedef struct     			// Boot Sector 512 octets FAT16 ou FAT32
	{
	uint8_t	    BS_JmpBoot[3];	   	// jmp vers boot code
   	uint8_t	    BS_OEMName[8];		// par convention = "MSWIN4.1"
   	uint16_t 	BPB_BytsPerSec;		// nb d'octets par secteur = 512 (ou multiple)
   	uint8_t	    BPB_SecPerClus;   	// nb de secteurs par cluster = 1,2,4, 8 ... 128
   	uint16_t	BPB_RsvdSecCnt;		// nb de secteurs reserves = 32 en general
   	uint8_t	    BPB_NumFATs;		// nb de FATS = 2
   	uint16_t	BPB_RootEntCnt;		// nb d'entrees root = 0 en FAT32
   	uint16_t	BPB_TotSec16;		// nb total de secteurs = 0 en FAT32 (cf BPB_TotSec32)
   	uint8_t	    BPB_Media;			// type de media = 0xF8 (fixe) ou 0xF0 (amovible)
   	uint16_t	BPB_FATSz16;		// taille d'une FAT = 0 en FAT32 (cf BPB_FATSz32)
   	uint16_t	BPB_SecPerTrk;		// nb de secteurs par piste du disque (Int13h)
   	uint16_t	BPB_NumHeads;		// nb de tetes du disque (Int13h)
   	uint32_t	BPB_HiddSec;		// nb de secteurs precedant cette partition sur le disque (offset)
   	uint32_t 	BPB_TotSec32;		// nb total de secteurs du volume
   	union
   		{
      	struct
      	{
      	uint8_t	    BS_DrvNum16;	   // numero du disque = 0x80 ... (Int13h)
      	uint8_t	    BS_Reserved16;	   // reserve = 0
      	uint8_t	    BS_BootSig;		   // signature = 0x29 indiquant la presence des 3 champs suivants
      	uint32_t	BS_VolID;          // serial number du volume
      	uint8_t	    BS_VolLab[11];     // label du volume = celui indique dans la racine
      	uint8_t	    BS_FilSysType[8];  // type de FAT = "FAT16   " ou "FAT     " ...
      	} __attribute__ ((packed)) Fat16;
      	struct
      	{
	   	uint32_t	BPB_FATSz32;	    // taille d'une FAT en FAT32
	   	uint16_t	BPB_ExtFlags;	    // nb de FAT32 et utilisation d'icelles (champ de bits)
	   	uint16_t	BPB_FSVer;		    // version de FAT32 utilisee = 0
	   	uint32_t	BPB_RootClus;	    // numero du premier cluster de la racine = 2
	   	uint16_t	BPB_FSInfo;		    // numero du secteur = 1 de la structure FSINFO dans la zone reservee
	   	uint16_t	BPB_BkBootSec;	    // numero du secteur = 6 de la copie du boot record dans la zone reservee
	   	uint8_t     BPB_Reserved[12];	// champ reserve pour usage ulterieur = 0
	   	uint8_t     BS_DrvNum32;  		// numero du disque = 0x80 ... (Int13h)
	   	uint8_t     BS_Reserved32;		// reserve = 0
	   	uint8_t     BS_BootSig;		    // signature = 0x29 indiquant la presence des 3 champs suivants
	   	uint32_t	BS_VolID;		    // serial number du volume
	   	uint8_t     BS_VolLab[11];		// label du volume = celui indique dans la racine
	   	uint8_t     BS_FilSysType[8];	// type de FAT = "FAT32   "
   		uint8_t     BS_Code[420];		// code de boot
      	} __attribute__ ((packed)) Fat32;
   	} u;
   	uint16_t    BS_BootSecSig;		// identifieur de boot secteur = 0xAA55
   	}
__attribute__ ((packed)) BootSector;    // pas d'alignement automatique (32 bits)


// fonctions by JG
SD_Error open_disk(unsigned part);
SD_Error open_part(void);


// fonctions by JG à implémenter pour Fatfs selon options
DSTATUS SD_disk_initialize();
DSTATUS SD_disk_status();
DRESULT SD_disk_read(BYTE* buff, DWORD sector, UINT cnt);
DRESULT SD_disk_write(BYTE* buff, DWORD sector, UINT cnt);
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);
DWORD get_fattime();


