/**
 * Copyright (C) IAR Systems 2009, and JG 2016
 */


#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"

#include "stm32_eth.h"
#include "lib_ethernet.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

typedef union _TranDesc0_t
    {
    uint32_t Data;
    struct
        {
        uint32_t  DB            : 1;
        uint32_t  UF            : 1;
        uint32_t  ED            : 1;
        uint32_t  CC            : 4;
        uint32_t  VF            : 1;
        uint32_t  EC            : 1;
        uint32_t  LC            : 1;
        uint32_t  NC            : 1;
        uint32_t  LSC           : 1;
        uint32_t  IPE           : 1;
        uint32_t  FF            : 1;
        uint32_t  JT            : 1;
        uint32_t  ES            : 1;
        uint32_t  IHE           : 1;
        uint32_t                : 3;
        uint32_t  TCH           : 1;
        uint32_t  TER           : 1;
        uint32_t  CIC           : 2;
        uint32_t                : 2;
        uint32_t  DP            : 1;
        uint32_t  DC            : 1;
        uint32_t  FS            : 1;
        uint32_t  LSEG          : 1;
        uint32_t  IC            : 1;
        uint32_t  OWN           : 1;
        };
    } TranDesc0_t, * pTranDesc0_t;

typedef union _TranDesc1_t
    {
    uint32_t Data;
    struct
        {
        uint32_t  TBS1          :13;
        uint32_t                : 3;
        uint32_t  TBS2          :12;
        uint32_t                : 3;
        };
    } TranDesc1_t, * pTranDesc1_t;

typedef union _RecDesc0_t
    {
    uint32_t Data;
    struct
        {
        uint32_t  RMAM_PCE      : 1;
        uint32_t  CE            : 1;
        uint32_t  DE            : 1;
        uint32_t  RE            : 1;
        uint32_t  RWT           : 1;
        uint32_t  FT            : 1;
        uint32_t  LC            : 1;
        uint32_t  IPHCE         : 1;
        uint32_t  LS            : 1;
        uint32_t  FS            : 1;
        uint32_t  VLAN          : 1;
        uint32_t  OE            : 1;
        uint32_t  LE            : 1;
        uint32_t  SAF           : 1;
        uint32_t  DERR          : 1;
        uint32_t  ES            : 1;
        uint32_t  FL            :14;
        uint32_t  AFM           : 1;
        uint32_t  OWN           : 1;
        };
    } RecDesc0_t, * pRecDesc0_t;

typedef union _recDesc1_t
    {
    uint32_t Data;
    struct
        {
        uint32_t  RBS1          :13;
        uint32_t                : 1;
        uint32_t  RCH           : 1;
        uint32_t  RER           : 1;
        uint32_t  RBS2          :14;
        uint32_t  DIC           : 1;
        };
    } RecDesc1_t, * pRecDesc1_t;

typedef union _EnetDmaDesc_t
    {
    uint32_t Data[4];
    // Rx DMA descriptor
    struct
        {
        RecDesc0_t                RxDesc0;
        RecDesc1_t                RxDesc1;
        uint32_t *                   pBuffer;
        union
            {
            uint32_t *                 pBuffer2;
            union _EnetDmaDesc_t *  pEnetDmaNextDesc;
            };
        } Rx;
    // Tx DMA descriptor
    struct
        {
        TranDesc0_t               TxDesc0;
        TranDesc1_t               TxDesc1;
        uint32_t *                   pBuffer1;
        union
            {
            uint32_t *                 pBuffer2;
            union _EnetDmaDesc_t *  pEnetDmaNextDesc;
            };
        } Tx;
    } EnetDmaDesc_t, * pEnetDmaDesc_t;

#pragma data_alignment=4
uint8_t RxBuff[EMAC_MAX_PACKET_SIZE];
#pragma data_alignment=4
uint8_t TxBuff[EMAC_MAX_PACKET_SIZE];

#pragma data_alignment=128
EnetDmaDesc_t EnetDmaRx;

#pragma data_alignment=128
EnetDmaDesc_t EnetDmaTx;

unsigned int PhyAddr;

/*******************************************************************************
* Function Name  : ENET_RxDscrInit
* Description    : Initializes the Rx ENET descriptor chain. Single Descriptor
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ENET_RxDscrInit(void)
    {
    /* Initialization */
    /* Assign temp Rx array to the ENET buffer */
    EnetDmaRx.Rx.pBuffer = (uint32_t *)RxBuff;

    /* Initialize RX ENET Status and control */
    EnetDmaRx.Rx.RxDesc0.Data = 0;

    /* Initialize the next descriptor- In our case its single descriptor */
    EnetDmaRx.Rx.pEnetDmaNextDesc = &EnetDmaRx;

    EnetDmaRx.Rx.RxDesc1.Data = 0;
    EnetDmaRx.Rx.RxDesc1.RER  = 0; // end of ring
    EnetDmaRx.Rx.RxDesc1.RCH  = 1; // end of ring

    /* Set the max packet size  */
    EnetDmaRx.Rx.RxDesc1.RBS1 = EMAC_MAX_PACKET_SIZE;

    /* Setting the VALID bit */
    EnetDmaRx.Rx.RxDesc0.OWN = 1;
    /* Setting the RX NEXT Descriptor Register inside the ENET */
    ETH->DMARDLAR = (uint32_t)&EnetDmaRx;
    /* Setting the RX NEXT Descriptor Register inside the ENET */
    // ETH_DMARDLAR = (uint32_t)&EnetDmaRx;
    }

/*******************************************************************************
* Function Name  : ENET_TxDscrInit
* Description    : Initializes the Tx ENET descriptor chain with single descriptor
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ENET_TxDscrInit(void)
    {
    /* ENET Start Address */
    EnetDmaTx.Tx.pBuffer1 = (uint32_t *)TxBuff;

    /* Next Descriptor Address */
    EnetDmaTx.Tx.pEnetDmaNextDesc = &EnetDmaTx;

    /* Initialize ENET status and control */
    EnetDmaTx.Tx.TxDesc0.TCH  = 1;
    EnetDmaTx.Tx.TxDesc0.Data = 0;
    EnetDmaTx.Tx.TxDesc1.Data = 0;
    /* Tx next set to Tx descriptor base */
    ETH->DMATDLAR = (uint32_t) & EnetDmaTx;
    }

/*************************************************************************
* Function Name  : LibEthernet_Init
* Description    : Initialise Controller
* Parameters     :
* Return         :
*************************************************************************/
uint16_t LibEthernet_Init(void)
    {
    /** ETHERNET pins configuration
    ETH_RMII_REF_CLK: PA1
    ETH_RMII_MDIO: PA2
    ETH_RMII_MDINT: PA3
    ETH_RMII_CRS_DV: PA7
    ETH_RMII_TX_EN: PG11
    ETH_RMII_MDC: PC1
    ETH_RMII_RXD0: PC4
    ETH_RMII_RXD1: PC5
    ETH_RMII_TXD0: PG13
    ETH_RMII_TXD1: PG14
    */

    GPIO_InitTypeDef GPIO_InitStructure;
    ETH_InitTypeDef ETH_InitStructure;

    // Enable ETHERNET clocks
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |
        RCC_AHB1Periph_ETH_MAC_Rx | RCC_AHB1Periph_ETH_MAC_PTP, ENABLE);

    // Enable GPIOs clocks
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
        RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOG, ENABLE);

    // Enable SYSCFG clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // Select RMII Interface
    SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);

    // Configure PA1, PA2, PA3 and PA7
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Connect PA1, PA2, PA3 and PA7 to ethernet module
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

    // Configure PG11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    // Connect PG11 to ethernet module
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_ETH);

    // Configure PC1, PC4 and PC5
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Connect PC1, PC4 and PC5 to ethernet module
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);

    // Configure PG13 and PG14*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    // Connect PG13 and PG14 to ethernet module
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);

    // Reset ETHERNET on AHB Bus
    ETH_DeInit();

    // Software reset
    ETH_SoftwareReset();

    // Wait for software reset
    while(ETH_GetSoftwareResetStatus() == SET);

    // ETHERNET Configuration
    // Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter
    ETH_StructInit(&ETH_InitStructure);

    // Fill ETH_InitStructure custom parameters
    ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable  ;
    //  ETH_InitStructure.ETH_Speed = ETH_Speed_100M;
    ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
    //  ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;
    ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
    ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
    ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Enable;
    ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Disable;
    ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
    ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
    ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
    ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;
    ETH_InitStructure.ETH_Speed = ETH_Speed_100M;

    for(PhyAddr = 0; 32 > PhyAddr; PhyAddr++)
        {
        // for the ks8721bl ethernet controller
        // PHY Identifier 1 and 2
        PHYID.HI = ETH_ReadPHYRegister(PhyAddr,2);  // 0x0022
        PHYID.LO = ETH_ReadPHYRegister(PhyAddr,3);  // 0x1619
        if ((PHYID.HI_LO == 0x00221619 ) || (PHYID.HI_LO == 0x0007C0F1))
        break;
        }

    if (PhyAddr > 32)
        {
        // Non supported Ethernet Physical layer
        return 1;
    }

    // Configure Ethernet
    if(ETH_Init(&ETH_InitStructure, PhyAddr) == 0)
        {
        // Ethernet Initialization Failure
        return 1;
        }

    /*
    uint32_t i;
    // Put the PHY in reset mode
    PhyWrite(PHY_ADDR,0, 0x8000);
    // Delay to assure PHY reset
    for(i=0; i<0xFFFFF; i++);
    // Set PHY operation mode
    PhyWrite(PHY_ADDR,28, 0);
    // Set PHY operation mode
    PhyWrite(PHY_ADDR,0, PHY_OPR_MODE);
    // DMA Init
    ETH->DMABMR |= (1<<8); // PBL =
    ETH->DMAIER = 0;
    ETH->DMASR = 0x0001E7FF;
    ETH->DMAOMR = (1<<20); // flush TX FIFO
    __no_operation();
    __no_operation();
    while(ETH->DMAOMR & (1<<20));
    // MAC init
    ETH->MMCCR |= (1<<3); // freeze statitistic
    // ETH->MACCR_bit.APCS = 1; // automatily padding
    // ETH->MACCR_bit.IPCO = 1; // calculate CRC on RX data
    ETH->MACCR = (1<<11) |   // Full duplex
               (1<<14);   // 100 Mb

    ETH->MACFFR = (1<<4) |  // Pass all multicast
                (3<<6);   // Cotrol packets

    // ETH->MACFFR_bit.RA  = 1; // Pass all
    // ETH->MACFFR_bit.PM  = 1; // Pass all

    ETH->MACFCR = 0;     // Flow control (pause)
    ETH->MACVLANTR = 0;  // VLAN
    ETH->MACPMTCSR = 0;  // Power control
    ETH->MACIMR    = 0;  // disable interrupts

    ETH->MACA0HR   = (UIP_ETHADDR5 << 8) + UIP_ETHADDR4;
    ETH->MACA0LR   = (uint32_t)(UIP_ETHADDR3 << 24) +\
                          (UIP_ETHADDR2 << 16) +\
                          (UIP_ETHADDR1 << 8)  +\
                           UIP_ETHADDR0;
    ETH->MACA1HR = 0x0000FFFF;  // disable seccond mac address
    ETH->MACA2HR = 0x0000FFFF;
    ETH->MACA3HR = 0x0000FFFF;
    */

    ENET_TxDscrInit();
    ENET_RxDscrInit();

    ETH_Start();
    /*
    ETH->DMARPDR = 1;
    ETH->MACCR |= (1<<2) | (1<<3);

    ETH->DMAOMR |= (1<<1) | (1<<13);
    */

    return 0;   // OK
    }

/*************************************************************************
* Function Name  : LibEthernet_Recv
* Description    : Read data for MAC/DMA Controller
* Parameters     : Buffer for received frame
* Return         : Frame size or 0 if no frame was received
*************************************************************************/
uint32_t LibEthernet_Recv(void * pPacket)
    {
    uint32_t size;
    /* check for validity */
    if(EnetDmaRx.Rx.RxDesc0.OWN == 0)
        {
        /* Get the size of the packet */
        size = EnetDmaRx.Rx.RxDesc0.FL;
        memcpy(pPacket, RxBuff, size);   // string.h library
        }
    else
        {
        return(ENET_NOK);
        }
    /* Give the buffer back to ENET */
    EnetDmaRx.Rx.RxDesc0.OWN = 1;
    /* Start the receive operation */
    ETH->DMARPDR = 1;

    /* Return packet size */
    return size;
    }

/*************************************************************************
* Function Name  : LibEthernet_Send
* Description    : Send data to MAC/DMA Controller
* Parameters     : Buffer with frame to send, frame size
* Return         : None
*************************************************************************/
void LibEthernet_Send(void *pPacket, uint32_t size)
    {
    while(EnetDmaTx.Tx.TxDesc0.OWN);

    memcpy(TxBuff, pPacket, size);

    /* Assign ENET address to Temp Tx Array */
    EnetDmaTx.Tx.pBuffer1 = (uint32_t *) TxBuff;

    /* Setting the Frame Length */
    EnetDmaTx.Tx.TxDesc0.Data = 0;
    EnetDmaTx.Tx.TxDesc0.TCH  = 1;
    EnetDmaTx.Tx.TxDesc0.LSEG = 1;
    EnetDmaTx.Tx.TxDesc0.FS   = 1;
    EnetDmaTx.Tx.TxDesc0.DC   = 0;
    EnetDmaTx.Tx.TxDesc0.DP   = 0;

    EnetDmaTx.Tx.TxDesc1.Data = 0;
    EnetDmaTx.Tx.TxDesc1.TBS1 = (size & 0xFFF);

    /* Start the ENET by setting the VALID bit in dmaPackStatus of current descr */
    EnetDmaTx.Tx.TxDesc0.OWN = 1;

    /* Start the transmit operation */
    ETH->DMATPDR = 1;
    }

/*
uint16_t PhyRead (uint8_t PhyAddr, uint8_t Reg)
    {
    ETH->MACMIIAR = (1<<0) | (0<<1) | (Reg<<6) | (PhyAddr<<11);
    while(ETH->MACMIIAR & (1<<0));

    return(ETH->MACMIIDR & (0xFFFF));
    }

void PhyWrite (uint8_t PhyAddr, uint8_t Reg, uint16_t Data)
    {
    ETH->MACMIIDR = Data;
    ETH->MACMIIAR = (1<<0) | (1<<1) | (Reg<<6) | (PhyAddr<<11);
    while(ETH->MACMIIAR & (1<<0));
    }
*/
