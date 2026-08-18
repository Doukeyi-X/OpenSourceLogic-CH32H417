/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_desc.h
 * Description        : USB CDC ACM descriptors for IAP serial upgrade
 *******************************************************************************/
#ifndef __USB_DESC_H
#define __USB_DESC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "ch32h417_conf.h"

/* APP USB2=1A86:5537, APP USB3=1A86:5538, IAP CDC=1A86:5539 */
#define DEF_USB_VID                  0x1A86
#define DEF_USB_PID                  0x5539

#define DEF_UEP_IN                    0x80
#define DEF_UEP_OUT                   0x00

#define DEF_UEP0                      0x00
#define DEF_UEP1                      0x01
#define DEF_UEP2                      0x02
#define DEF_UEP3                      0x03

#define DEF_USBD_UEP0_SIZE            64
/* CDC bulk packet size (USBHS high-speed; 64 is enough for IAP frames) */
#define DEF_CDC_PACK_SIZE             64

/* CDC class requests */
#define CDC_SET_LINE_CODING           0x20
#define CDC_GET_LINE_CODING           0x21
#define CDC_SET_CONTROL_LINE_STATE    0x22

#define DEF_USBD_DEVICE_DESC_LEN      ((uint8_t)MyDevDescr[0])
#define DEF_USBD_CONFIG_DESC_LEN      ((uint16_t)MyCfgDescr[2] + ((uint16_t)MyCfgDescr[3] << 8))
#define DEF_USBD_LANG_DESC_LEN        ((uint16_t)MyLangDescr[0])
#define DEF_USBD_MANU_DESC_LEN        ((uint16_t)MyManuInfo[0])
#define DEF_USBD_PROD_DESC_LEN        ((uint16_t)MyProdInfo[0])
#define DEF_USBD_SN_DESC_LEN          ((uint16_t)MySerNumInfo[0])

extern const uint8_t MyDevDescr[];
extern const uint8_t MyCfgDescr[];
extern const uint8_t MyLangDescr[];
extern const uint8_t MyManuInfo[];
extern const uint8_t MyProdInfo[];
extern const uint8_t MySerNumInfo[];

#ifdef __cplusplus
}
#endif

#endif
