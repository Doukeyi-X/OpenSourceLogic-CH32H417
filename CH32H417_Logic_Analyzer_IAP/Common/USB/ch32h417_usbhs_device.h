/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32h417_usbhs_device.h
 * Description        : USBHS CDC device for IAP
 *******************************************************************************/
#ifndef __CH32H417_USBHS_DEVICE_H__
#define __CH32H417_USBHS_DEVICE_H__

#include "ch32h417_conf.h"
#include "string.h"
#include "usb_desc.h"
#include "ch32h417_usb.h"

#ifdef __cplusplus
extern "C" {
#endif

#define pUSBHS_SetupReqPak            ((PUSB_SETUP_REQ)USBHS_EP0_Buf)

extern const uint8_t *pUSBHS_Descr;

extern volatile uint8_t  USBHS_SetupReqCode;
extern volatile uint8_t  USBHS_SetupReqType;
extern volatile uint16_t USBHS_SetupReqValue;
extern volatile uint16_t USBHS_SetupReqIndex;
extern volatile uint16_t USBHS_SetupReqLen;

extern volatile uint8_t  USBHS_DevConfig;
extern volatile uint8_t  USBHS_DevAddr;
extern volatile uint8_t  USBHS_DevSleepStatus;
extern volatile uint8_t  USBHS_DevEnumStatus;

extern __attribute__((aligned(4))) uint8_t USBHS_EP0_Buf[];
extern __attribute__((aligned(4))) uint8_t USBHS_EP2_Rx_Buf[];

void USBHS_Device_Endp_Init(void);
void USBHS_Device_Init(FunctionalState sta);

#ifdef __cplusplus
}
#endif

#endif
