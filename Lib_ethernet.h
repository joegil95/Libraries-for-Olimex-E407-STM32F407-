/**
 * Ethernet Library for STM32F4xx devices with new Olimex Physical layer
 */

//#include "my_includes.h"

#ifndef  __ETHERNET_H
#define  __ETHERNET_H

#include <stdint.h>

union
    {
    uint32_t HI_LO;
    struct
        {
        uint16_t  LO;
        uint16_t  HI;
        };
    } PHYID;


#define PHY_ADDR  0x15

#define EMAC_MAX_PACKET_SIZE 1520
#define ENET_OK  (1)
#define ENET_NOK (0)
// ENET_OperatingMode
#define PHY_OPR_MODE        0x2100      //  Set the full-duplex mode at 100 Mb/s

uint32_t EthernetPhyInit(void);
uint32_t uIPMain(void);
uint16_t PhyRead (uint8_t PhyAddr, uint8_t Reg);
void PhyWrite (uint8_t PhyAddr, uint8_t Reg, uint16_t Data);

void ENET_RxDscrInit(void);
void ENET_TxDscrInit(void);

uint16_t LibEthernet_Init(void);
uint32_t LibEthernet_Recv(void * pPacket);
void LibEthernet_Send(void *pPacket, uint32_t size);

#endif  /* __ETHERNET_H */
