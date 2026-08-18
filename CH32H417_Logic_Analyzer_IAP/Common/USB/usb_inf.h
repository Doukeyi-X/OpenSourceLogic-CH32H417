/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_inf.h
 * Description        : USB CDC stream interface for IAP
 *******************************************************************************/
#ifndef __USB_INF_H
#define __USB_INF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "ch32h417_conf.h"
#include "ch32h417_usbhs_device.h"

#define DEF_USBFS_PORT_ENABLE       0
#define DEF_USBHS_PORT_ENABLE       1

#define DEF_USB_PORT_FS             0
#define DEF_USB_PORT_HS             1

/* RX ring — IAP RAM 仅约 20KB，不可过大 */
#define CDC_RX_RING_SIZE            1024

void USB_Init(FunctionalState sta);

/* Called from USB ISR when host sends CDC data */
void CDC_Rx_Push(const uint8_t *data, uint16_t len);

/* Send response bytes over CDC (blocking with short timeout) */
uint8_t CDC_Tx(const uint8_t *data, uint16_t len);

/* Main-loop: parse serial IAP frames and program flash */
void CDC_IAP_Poll(void);

/* Low-level EP3 kick (used by USB stack / CDC_Tx) */
void CDC_EP3_Start_TX(const uint8_t *data, uint16_t len);
uint8_t CDC_EP3_Busy(void);

#ifdef __cplusplus
}
#endif

#endif
