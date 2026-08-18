/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32h417_usbhs_device.c
 * Description        : USBHS CDC ACM device for CH32 IAP serial upgrade
 *******************************************************************************/
#include "ch32h417_usbhs_device.h"
#include "usb_desc.h"
#include "usb_inf.h"

const uint8_t *pUSBHS_Descr;

volatile uint8_t USBHS_SetupReqCode;
volatile uint8_t USBHS_SetupReqType;
volatile uint16_t USBHS_SetupReqValue;
volatile uint16_t USBHS_SetupReqIndex;
volatile uint16_t USBHS_SetupReqLen;

volatile uint8_t USBHS_DevConfig;
volatile uint8_t USBHS_DevAddr;
volatile uint8_t USBHS_DevSleepStatus;
volatile uint8_t USBHS_DevEnumStatus;

/* CDC line coding (dummy, baud ignored over USB) */
static uint8_t CDC_LineCoding[7] = {0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08}; /* 115200 8N1 */

__attribute__((aligned(4))) uint8_t USBHS_EP0_Buf[DEF_USBD_UEP0_SIZE];
__attribute__((aligned(4))) uint8_t USBHS_EP1_Tx_Buf[8];
__attribute__((aligned(4))) uint8_t USBHS_EP2_Rx_Buf[DEF_CDC_PACK_SIZE];
/* EP3 TX 复用 USB_CDC_Tx_Buf，不再单独占一份 RAM */

extern volatile uint8_t cdc_ep3_busy;
extern __attribute__((aligned(4))) uint8_t USB_CDC_Tx_Buf[];

void USBHS_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void USBHS_Device_Endp_Init(void)
{
    USBHSD->UEP_TX_EN = USBHS_UEP0_T_EN | USBHS_UEP1_T_EN | USBHS_UEP3_T_EN;
    USBHSD->UEP_RX_EN = USBHS_UEP0_R_EN | USBHS_UEP2_R_EN;

    USBHSD->UEP0_MAX_LEN = DEF_USBD_UEP0_SIZE;
    USBHSD->UEP1_MAX_LEN = 8;
    USBHSD->UEP2_MAX_LEN = DEF_CDC_PACK_SIZE;
    USBHSD->UEP3_MAX_LEN = DEF_CDC_PACK_SIZE;

    USBHSD->UEP0_DMA = (uint32_t)(uint8_t *)USBHS_EP0_Buf;
    USBHSD->UEP1_TX_DMA = (uint32_t)(uint8_t *)USBHS_EP1_Tx_Buf;
    USBHSD->UEP2_RX_DMA = (uint32_t)(uint8_t *)USBHS_EP2_Rx_Buf;
    USBHSD->UEP3_TX_DMA = (uint32_t)(uint8_t *)USB_CDC_Tx_Buf;

    USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_RES_NAK;
    USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_RES_ACK;
    USBHSD->UEP1_TX_CTRL = USBHS_UEP_T_RES_NAK;
    USBHSD->UEP2_RX_CTRL = USBHS_UEP_R_RES_ACK;
    USBHSD->UEP3_TX_CTRL = USBHS_UEP_T_RES_NAK;

    cdc_ep3_busy = 0;
}

void USBHS_RCC_Init(FunctionalState sta)
{
    if (sta)
    {
        if ((RCC->PLLCFGR & RCC_SYSPLL_SEL) != RCC_SYSPLL_USBHS)
        {
            RCC_USBHS_PLLCmd(DISABLE);
            RCC_USBHSPLLCLKConfig(RCC_USBHSPLLSource_HSE);
            RCC_USBHSPLLReferConfig(RCC_USBHSPLLRefer_25M);
            RCC_USBHSPLLClockSourceDivConfig(RCC_USBHSPLL_IN_Div1);
            RCC_USBHS_PLLCmd(ENABLE);
            while (!(RCC->CTLR & RCC_USBHS_PLLRDY))
                ;
        }
        RCC_UTMIcmd(ENABLE);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS, ENABLE);
    }
    else
    {
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS, DISABLE);
        RCC_UTMIcmd(DISABLE);
        if ((RCC->PLLCFGR & RCC_SYSPLL_SEL) != RCC_SYSPLL_USBHS)
            RCC_USBHS_PLLCmd(DISABLE);
    }
}

void USBHS_Device_Init(FunctionalState sta)
{
    if (sta)
    {
        USBHS_RCC_Init(ENABLE);
        USBHSD->CONTROL = USBHS_UD_RST_LINK | USBHS_UD_PHY_SUSPENDM;
        USBHSD->INT_EN = USBHS_UDIE_BUS_RST | USBHS_UDIE_SUSPEND | USBHS_UDIE_BUS_SLEEP |
                         USBHS_UDIE_LPM_ACT | USBHS_UDIE_TRANSFER | USBHS_UDIE_LINK_RDY;
        USBHS_Device_Endp_Init();
        USBHSD->BASE_MODE = USBHS_UD_SPEED_HIGH;
        USBHSD->CONTROL = USBHS_UD_DEV_EN | USBHS_UD_DMA_EN | USBHS_UD_LPM_EN | USBHS_UD_PHY_SUSPENDM;
        NVIC_EnableIRQ(USBHS_IRQn);
    }
    else
    {
        USBHSD->CONTROL = USBHS_UD_RST_SIE | USBHS_UD_RST_LINK;
        NVIC_DisableIRQ(USBHS_IRQn);
        USBHS_RCC_Init(DISABLE);
    }
}

void USBHS_IRQHandler(void)
{
    uint8_t intflag, intst, errflag;
    uint16_t len;
    uint8_t endp_num;

    intflag = USBHSD->INT_FG;
    intst = USBHSD->INT_ST;

    if (intflag & USBHS_UDIF_TRANSFER)
    {
        endp_num = intst & USBHS_UDIS_EP_ID_MASK;
        if (!(intst & USBHS_UDIS_EP_DIR)) /* SETUP/OUT */
        {
            switch (endp_num)
            {
            case DEF_UEP0:
                USBHSD->UEP0_RX_CTRL &= ~USBHS_UEP_R_DONE;
                if ((USBHSD->UEP0_RX_CTRL & USBHS_UEP_R_SETUP_IS) && !(USBHSD->UEP0_RX_CTRL & USBHS_UEP_R_DONE))
                {
                    USBHS_SetupReqType = pUSBHS_SetupReqPak->bRequestType;
                    USBHS_SetupReqCode = pUSBHS_SetupReqPak->bRequest;
                    USBHS_SetupReqLen = pUSBHS_SetupReqPak->wLength;
                    USBHS_SetupReqValue = pUSBHS_SetupReqPak->wValue;
                    USBHS_SetupReqIndex = pUSBHS_SetupReqPak->wIndex;

                    len = 0;
                    errflag = 0;

                    if ((USBHS_SetupReqType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD)
                    {
                        /* CDC class requests */
                        if ((USBHS_SetupReqType & USB_REQ_TYP_MASK) == USB_REQ_TYP_CLASS)
                        {
                            switch (USBHS_SetupReqCode)
                            {
                            case CDC_GET_LINE_CODING:
                                pUSBHS_Descr = CDC_LineCoding;
                                len = 7;
                                break;
                            case CDC_SET_LINE_CODING:
                                /* data stage on EP0 OUT */
                                break;
                            case CDC_SET_CONTROL_LINE_STATE:
                                break;
                            default:
                                errflag = 0xFF;
                                break;
                            }
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                    }
                    else
                    {
                        switch (USBHS_SetupReqCode)
                        {
                        case USB_GET_DESCRIPTOR:
                            switch ((uint8_t)(USBHS_SetupReqValue >> 8))
                            {
                            case USB_DESCR_TYP_DEVICE:
                                pUSBHS_Descr = MyDevDescr;
                                len = DEF_USBD_DEVICE_DESC_LEN;
                                break;
                            case USB_DESCR_TYP_CONFIG:
                                pUSBHS_Descr = MyCfgDescr;
                                len = DEF_USBD_CONFIG_DESC_LEN;
                                break;
                            case USB_DESCR_TYP_STRING:
                                switch ((uint8_t)(USBHS_SetupReqValue & 0xFF))
                                {
                                case DEF_STRING_DESC_LANG:
                                    pUSBHS_Descr = MyLangDescr;
                                    len = DEF_USBD_LANG_DESC_LEN;
                                    break;
                                case DEF_STRING_DESC_MANU:
                                    pUSBHS_Descr = MyManuInfo;
                                    len = DEF_USBD_MANU_DESC_LEN;
                                    break;
                                case DEF_STRING_DESC_PROD:
                                    pUSBHS_Descr = MyProdInfo;
                                    len = DEF_USBD_PROD_DESC_LEN;
                                    break;
                                case DEF_STRING_DESC_SERN:
                                    pUSBHS_Descr = MySerNumInfo;
                                    len = DEF_USBD_SN_DESC_LEN;
                                    break;
                                default:
                                    errflag = 0xFF;
                                    break;
                                }
                                break;
                            default:
                                errflag = 0xFF;
                                break;
                            }
                            if (USBHS_SetupReqLen > len)
                                USBHS_SetupReqLen = len;
                            len = (USBHS_SetupReqLen >= DEF_USBD_UEP0_SIZE) ? DEF_USBD_UEP0_SIZE : USBHS_SetupReqLen;
                            memcpy(USBHS_EP0_Buf, pUSBHS_Descr, len);
                            pUSBHS_Descr += len;
                            break;

                        case USB_SET_ADDRESS:
                            USBHS_DevAddr = (uint16_t)(USBHS_SetupReqValue & 0xFF);
                            break;

                        case USB_GET_CONFIGURATION:
                            USBHS_EP0_Buf[0] = USBHS_DevConfig;
                            if (USBHS_SetupReqLen > 1)
                                USBHS_SetupReqLen = 1;
                            break;

                        case USB_SET_CONFIGURATION:
                            USBHS_DevConfig = (uint8_t)(USBHS_SetupReqValue & 0xFF);
                            USBHS_DevEnumStatus = 0x01;
                            break;

                        case USB_CLEAR_FEATURE:
                            if ((USBHS_SetupReqType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
                            {
                                if ((uint8_t)(USBHS_SetupReqValue & 0xFF) == USB_REQ_FEAT_ENDP_HALT)
                                {
                                    switch ((uint8_t)(USBHS_SetupReqIndex & 0xFF))
                                    {
                                    case (DEF_UEP1 | DEF_UEP_IN):
                                        USBHSD->UEP1_TX_CTRL = USBHS_UEP_T_RES_NAK;
                                        break;
                                    case (DEF_UEP2 | DEF_UEP_OUT):
                                        USBHSD->UEP2_RX_CTRL = USBHS_UEP_R_RES_ACK;
                                        break;
                                    case (DEF_UEP3 | DEF_UEP_IN):
                                        USBHSD->UEP3_TX_CTRL = USBHS_UEP_T_RES_NAK;
                                        cdc_ep3_busy = 0;
                                        break;
                                    default:
                                        errflag = 0xFF;
                                        break;
                                    }
                                }
                                else
                                    errflag = 0xFF;
                            }
                            else if ((USBHS_SetupReqType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)
                            {
                                if ((uint8_t)(USBHS_SetupReqValue & 0xFF) == 0x01)
                                    USBHS_DevSleepStatus &= ~0x01;
                                else
                                    errflag = 0xFF;
                            }
                            else
                                errflag = 0xFF;
                            break;

                        case USB_SET_FEATURE:
                            if ((USBHS_SetupReqType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
                            {
                                if ((uint8_t)(USBHS_SetupReqValue & 0xFF) == USB_REQ_FEAT_ENDP_HALT)
                                {
                                    switch ((uint8_t)(USBHS_SetupReqIndex & 0xFF))
                                    {
                                    case (DEF_UEP1 | DEF_UEP_IN):
                                        USBHSD->UEP1_TX_CTRL = (USBHSD->UEP1_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_STALL;
                                        break;
                                    case (DEF_UEP2 | DEF_UEP_OUT):
                                        USBHSD->UEP2_RX_CTRL = (USBHSD->UEP2_RX_CTRL & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_STALL;
                                        break;
                                    case (DEF_UEP3 | DEF_UEP_IN):
                                        USBHSD->UEP3_TX_CTRL = (USBHSD->UEP3_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_STALL;
                                        break;
                                    default:
                                        errflag = 0xFF;
                                        break;
                                    }
                                }
                            }
                            else
                                errflag = 0xFF;
                            break;

                        case USB_GET_INTERFACE:
                            USBHS_EP0_Buf[0] = 0x00;
                            if (USBHS_SetupReqLen > 1)
                                USBHS_SetupReqLen = 1;
                            break;

                        case USB_SET_INTERFACE:
                            break;

                        case USB_GET_STATUS:
                            USBHS_EP0_Buf[0] = 0x00;
                            USBHS_EP0_Buf[1] = 0x00;
                            if (USBHS_SetupReqLen > 2)
                                USBHS_SetupReqLen = 2;
                            break;

                        default:
                            errflag = 0xFF;
                            break;
                        }
                    }

                    if (errflag == 0xFF)
                    {
                        USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_STALL;
                        USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_STALL;
                    }
                    else
                    {
                        if (USBHS_SetupReqType & DEF_UEP_IN)
                        {
                            /* special: GET_LINE_CODING already set pUSBHS_Descr */
                            if ((USBHS_SetupReqType & USB_REQ_TYP_MASK) == USB_REQ_TYP_CLASS &&
                                USBHS_SetupReqCode == CDC_GET_LINE_CODING)
                            {
                                len = (USBHS_SetupReqLen > 7) ? 7 : USBHS_SetupReqLen;
                                memcpy(USBHS_EP0_Buf, CDC_LineCoding, len);
                                USBHS_SetupReqLen = 0;
                                USBHSD->UEP0_TX_LEN = len;
                                USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
                            }
                            else
                            {
                                len = (USBHS_SetupReqLen > DEF_USBD_UEP0_SIZE) ? DEF_USBD_UEP0_SIZE : USBHS_SetupReqLen;
                                USBHS_SetupReqLen -= len;
                                USBHSD->UEP0_TX_LEN = len;
                                USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
                            }
                        }
                        else
                        {
                            if (USBHS_SetupReqLen == 0)
                            {
                                USBHSD->UEP0_TX_LEN = 0;
                                USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
                            }
                            else
                            {
                                USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK;
                            }
                        }
                    }
                }
                else
                {
                    /* EP0 data OUT (e.g. SET_LINE_CODING) */
                    USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_RES_NAK;
                    len = USBHSD->UEP0_RX_LEN;
                    if ((USBHS_SetupReqType & USB_REQ_TYP_MASK) == USB_REQ_TYP_CLASS &&
                        USBHS_SetupReqCode == CDC_SET_LINE_CODING)
                    {
                        if (len >= 7)
                            memcpy(CDC_LineCoding, USBHS_EP0_Buf, 7);
                    }
                    USBHS_SetupReqLen = 0;
                    USBHSD->UEP0_TX_LEN = 0;
                    USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
                }
                break;

            case DEF_UEP2: /* CDC data OUT */
                USBHSD->UEP2_RX_CTRL &= ~USBHS_UEP_R_DONE;
                if (USBHSD->UEP2_RX_CTRL & USBHS_UEP_R_TOG_MATCH)
                {
                    len = USBHSD->UEP2_RX_LEN & 0xFF;
                    if (len)
                        CDC_Rx_Push(USBHS_EP2_Rx_Buf, len);
                    USBHSD->UEP2_RX_CTRL ^= USBHS_UEP_R_TOG_DATA1;
                    USBHSD->UEP2_RX_CTRL = ((USBHSD->UEP2_RX_CTRL) & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_ACK;
                }
                else
                {
                    USBHSD->UEP2_RX_CTRL = ((USBHSD->UEP2_RX_CTRL) & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_ACK;
                }
                break;

            default:
                break;
            }
        }
        else /* IN */
        {
            switch (endp_num)
            {
            case DEF_UEP0:
                USBHSD->UEP0_TX_CTRL &= ~USBHS_UEP_T_DONE;
                if (USBHS_SetupReqLen == 0)
                    USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK;

                if ((USBHS_SetupReqType & USB_REQ_TYP_MASK) == USB_REQ_TYP_STANDARD)
                {
                    switch (USBHS_SetupReqCode)
                    {
                    case USB_GET_DESCRIPTOR:
                        len = USBHS_SetupReqLen >= DEF_USBD_UEP0_SIZE ? DEF_USBD_UEP0_SIZE : USBHS_SetupReqLen;
                        memcpy(USBHS_EP0_Buf, pUSBHS_Descr, len);
                        USBHS_SetupReqLen -= len;
                        pUSBHS_Descr += len;
                        USBHSD->UEP0_TX_LEN = len;
                        USBHSD->UEP0_TX_CTRL ^= USBHS_UEP_T_TOG_DATA1;
                        USBHSD->UEP0_TX_CTRL = (USBHSD->UEP0_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_ACK;
                        break;
                    case USB_SET_ADDRESS:
                        USBHSD->DEV_AD = USBHS_DevAddr;
                        break;
                    default:
                        USBHSD->UEP0_TX_LEN = 0;
                        break;
                    }
                }
                break;

            case DEF_UEP1:
                USBHSD->UEP1_TX_CTRL &= ~USBHS_UEP_T_DONE;
                USBHSD->UEP1_TX_CTRL ^= USBHS_UEP_T_TOG_DATA1;
                USBHSD->UEP1_TX_CTRL = (USBHSD->UEP1_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_NAK;
                break;

            case DEF_UEP3: /* CDC data IN complete */
                USBHSD->UEP3_TX_CTRL &= ~USBHS_UEP_T_DONE;
                USBHSD->UEP3_TX_CTRL ^= USBHS_UEP_T_TOG_DATA1;
                USBHSD->UEP3_TX_CTRL = (USBHSD->UEP3_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_NAK;
                cdc_ep3_busy = 0;
                break;

            default:
                break;
            }
        }
    }
    else if (intflag & USBHS_UDIF_LINK_RDY)
    {
        USBHSD->INT_FG = USBHS_UDIF_LINK_RDY;
    }
    else if (intflag & USBHS_UDIF_SUSPEND)
    {
        USBHSD->INT_FG = USBHS_UDIF_SUSPEND;
        if (USBHSD->MIS_ST & USBHS_UDMS_SUSPEND)
            USBHS_DevSleepStatus |= 0x02;
        else
            USBHS_DevSleepStatus &= ~0x02;
    }
    else if (intflag & USBHS_UDIF_BUS_RST)
    {
        USBHS_DevConfig = 0;
        USBHS_DevAddr = 0;
        USBHS_DevSleepStatus = 0;
        USBHS_DevEnumStatus = 0;
        USBHSD->DEV_AD = 0;
        USBHS_Device_Endp_Init();
        USBHSD->INT_FG = USBHS_UDIF_BUS_RST;
    }
    else
    {
        USBHSD->INT_FG = intflag;
    }
}
