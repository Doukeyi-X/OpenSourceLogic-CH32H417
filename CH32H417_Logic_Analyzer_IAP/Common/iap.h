/********************************** (C) COPYRIGHT *******************************
 * File Name          : iap.h
 * Description        : IAP flash programming (CDC serial frames)
 *******************************************************************************/
#ifndef __IAP_H
#define __IAP_H

#include "ch32h417.h"
#include "stdio.h"

/* Cmd+Len+payload；IAP RAM 紧张，payload 最大 120 */
#define USBD_DATA_SIZE    128
#define FLASH_Base        0x08006000

#define Uart_Sync_Head1   0xAA
#define Uart_Sync_Head2   0x55

#define CMD_IAP_PROM      0x80
#define CMD_IAP_ERASE     0x81
#define CMD_IAP_VERIFY    0x82
#define CMD_IAP_END       0x83
#define CMD_JUMP_IAP      0x84

#define ERR_SUCCESS       0x00
#define ERR_ERROR         0x01
#define ERR_End           0x02

#define CalAddr           (0x08078000 - 4)
#define CheckNum          (0x5aa55aa5)

extern u8 IAP_Deal_Buf[USBD_DATA_SIZE + 4];
extern vu8 End_Flag;

u8 RecData_Deal(void);
u8 IAP_App_Is_Valid(void);
void GPIO_Cfg_init(void);
u8 PA0_Check(void);

#endif
