/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32h417_usbfs_device.c
 * Description        : USBFS stubs — IAP uses USBHS CDC only
 *******************************************************************************/
#include "ch32h417_usbfs_device.h"

const uint8_t *pUSBFS_Descr;
volatile uint8_t USBFS_SetupReqCode;
volatile uint8_t USBFS_SetupReqType;
volatile uint16_t USBFS_SetupReqValue;
volatile uint16_t USBFS_SetupReqIndex;
volatile uint16_t USBFS_SetupReqLen;
volatile uint8_t USBFS_DevConfig;
volatile uint8_t USBFS_DevAddr;
volatile uint8_t USBFS_DevSleepStatus;
volatile uint8_t USBFS_DevEnumStatus;

__attribute__((aligned(4))) uint8_t USBFS_EP0_Buf[64];
__attribute__((aligned(4))) uint8_t USBFS_EP2_Buf[128];

void USBFS_RCC_Init(void) {}
void USBFS_Device_Endp_Init(void) {}
void USBFS_Device_Init(FunctionalState sta) { (void)sta; }
void USBFS_Send_Resume(void) {}
