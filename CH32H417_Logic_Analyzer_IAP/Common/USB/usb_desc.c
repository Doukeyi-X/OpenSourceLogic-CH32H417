/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_desc.c
 * Description        : USB CDC ACM device descriptors for CH32 IAP
 *******************************************************************************/
#include "usb_desc.h"
#include "hardware.h"

/* Device Descriptor — CDC (IAD composite style) */
const uint8_t MyDevDescr[] =
{
    0x12,                                   /* bLength */
    0x01,                                   /* bDescriptorType */
    0x00, 0x02,                             /* bcdUSB 2.00 */
    0xEF,                                   /* bDeviceClass: Misc */
    0x02,                                   /* bDeviceSubClass: Common */
    0x01,                                   /* bDeviceProtocol: IAD */
    DEF_USBD_UEP0_SIZE,                     /* bMaxPacketSize0 */
    (uint8_t)DEF_USB_VID, (uint8_t)(DEF_USB_VID >> 8),
    (uint8_t)DEF_USB_PID, (uint8_t)(DEF_USB_PID >> 8),
    (uint8_t)DEF_VERSION, (uint8_t)(DEF_VERSION >> 8),
    0x01,                                   /* iManufacturer */
    0x02,                                   /* iProduct */
    0x03,                                   /* iSerialNumber */
    0x01,                                   /* bNumConfigurations */
};

/*
 * Configuration Descriptor
 *   IAD + Comm IF0 (EP1 IN interrupt) + Data IF1 (EP2 OUT + EP3 IN bulk)
 *   total length = 0x4B (75)
 */
const uint8_t MyCfgDescr[] =
{
    /* Configuration */
    0x09, 0x02, 0x4B, 0x00, 0x02, 0x01, 0x00, 0x80, 0x32,

    /* IAD: CDC */
    0x08, 0x0B, 0x00, 0x02, 0x02, 0x02, 0x01, 0x00,

    /* Interface 0: Communication Class */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,

    /* Header Functional Descriptor */
    0x05, 0x24, 0x00, 0x10, 0x01,

    /* Call Management Functional Descriptor */
    0x05, 0x24, 0x01, 0x00, 0x01,

    /* ACM Functional Descriptor */
    0x04, 0x24, 0x02, 0x02,

    /* Union Functional Descriptor */
    0x05, 0x24, 0x06, 0x00, 0x01,

    /* Endpoint 1 IN — interrupt notification */
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x01,

    /* Interface 1: Data Class */
    0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,

    /* Endpoint 2 OUT — bulk data host→device */
    0x07, 0x05, 0x02, 0x02,
    (uint8_t)DEF_CDC_PACK_SIZE, (uint8_t)(DEF_CDC_PACK_SIZE >> 8),
    0x00,

    /* Endpoint 3 IN — bulk data device→host */
    0x07, 0x05, 0x83, 0x02,
    (uint8_t)DEF_CDC_PACK_SIZE, (uint8_t)(DEF_CDC_PACK_SIZE >> 8),
    0x00,
};

const uint8_t MyLangDescr[] =
{
    0x04, 0x03, 0x09, 0x04
};

const uint8_t MyManuInfo[] =
{
    0x0E, 0x03,
    'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0
};

const uint8_t MyProdInfo[] =
{
    28, 0x03,
    'C', 0, 'H', 0, '3', 0, '2', 0, ' ', 0,
    'L', 0, 'A', 0, ' ', 0,
    'I', 0, 'A', 0, 'P', 0, ' ', 0,
    'C', 0, 'D', 0, 'C', 0
};

const uint8_t MySerNumInfo[] =
{
    0x16, 0x03,
    '0', 0, '1', 0, '2', 0, '3', 0, '4', 0,
    '5', 0, '6', 0, '7', 0, '8', 0, '9', 0
};
