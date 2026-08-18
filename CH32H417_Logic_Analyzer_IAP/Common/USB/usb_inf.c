/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_inf.c
 * Description        : CDC stream + IAP serial frame processing
 *
 * Host frame (same idea as WCH UART IAP):
 *   AA 55 | Cmd | Len | data[Len] | sumL | sumH | 55 AA
 *   sum = Cmd + Len + sum(data)
 *
 * Device reply (END has no reply):
 *   AA 55 | 00 | status | 55 AA     status: 0=OK, 1=ERR
 *******************************************************************************/
#include "debug.h"
#include "usb_inf.h"
#include "usb_desc.h"
#include "iap.h"
#include "string.h"

/* ---------------- RX ring ---------------- */
static volatile uint16_t rx_w = 0;
static volatile uint16_t rx_r = 0;
static uint8_t rx_ring[CDC_RX_RING_SIZE];

/* TX buffer for EP3 */
__attribute__((aligned(4))) uint8_t USB_CDC_Tx_Buf[DEF_CDC_PACK_SIZE];
volatile uint8_t cdc_ep3_busy = 0;

void USB_Init(FunctionalState sta)
{
    if (sta == ENABLE)
    {
#if DEF_USBHS_PORT_ENABLE
        RCC_HB2PeriphClockCmd(RCC_HB2Periph_AFIO | RCC_HB2Periph_GPIOB, ENABLE);
        GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
        USBHS_Device_Init(ENABLE);
#endif
    }
    else
    {
#if DEF_USBHS_PORT_ENABLE
        USBHS_Device_Init(DISABLE);
#endif
    }
}

void CDC_Rx_Push(const uint8_t *data, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        uint16_t next = (uint16_t)((rx_w + 1) % CDC_RX_RING_SIZE);
        if (next == rx_r)
            break; /* overflow drop */
        rx_ring[rx_w] = data[i];
        rx_w = next;
    }
}

static uint16_t cdc_rx_avail(void)
{
    if (rx_w >= rx_r)
        return (uint16_t)(rx_w - rx_r);
    return (uint16_t)(CDC_RX_RING_SIZE - rx_r + rx_w);
}

static int cdc_rx_peek(uint16_t offset)
{
    if (offset >= cdc_rx_avail())
        return -1;
    return rx_ring[(rx_r + offset) % CDC_RX_RING_SIZE];
}

static void cdc_rx_drop(uint16_t n)
{
    rx_r = (uint16_t)((rx_r + n) % CDC_RX_RING_SIZE);
}

static int cdc_rx_get(void)
{
    int v;
    if (rx_r == rx_w)
        return -1;
    v = rx_ring[rx_r];
    rx_r = (uint16_t)((rx_r + 1) % CDC_RX_RING_SIZE);
    return v;
}

uint8_t CDC_EP3_Busy(void)
{
    return cdc_ep3_busy;
}

void CDC_EP3_Start_TX(const uint8_t *data, uint16_t len)
{
    if (len > DEF_CDC_PACK_SIZE)
        len = DEF_CDC_PACK_SIZE;
    memcpy(USB_CDC_Tx_Buf, data, len);
    cdc_ep3_busy = 1;
    USBHSD->UEP3_TX_DMA = (uint32_t)USB_CDC_Tx_Buf;
    USBHSD->UEP3_TX_LEN = len;
    USBHSD->UEP3_TX_CTRL = (USBHSD->UEP3_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_ACK;
}

uint8_t CDC_Tx(const uint8_t *data, uint16_t len)
{
    uint32_t t = 0;
    while (cdc_ep3_busy)
    {
        if (++t > 500000)
            return 1;
    }
    CDC_EP3_Start_TX(data, len);
    t = 0;
    while (cdc_ep3_busy)
    {
        if (++t > 500000)
            return 1;
    }
    return 0;
}

static void cdc_reply(uint8_t status)
{
    uint8_t rsp[6];
    rsp[0] = Uart_Sync_Head1;
    rsp[1] = Uart_Sync_Head2;
    rsp[2] = 0x00;
    rsp[3] = status;
    rsp[4] = Uart_Sync_Head2;
    rsp[5] = Uart_Sync_Head1;
    CDC_Tx(rsp, 6);
}

/*
 * Parse one complete frame if available.
 * Frame: AA 55 Cmd Len data[Len] sumL sumH 55 AA
 * Total length = 8 + Len  (Len up to 248)
 * Drain multiple frames per call to keep up with pipelined host.
 */
void CDC_IAP_Poll(void)
{
    /* 静态缓冲，避免栈溢出；IAP stack 仅 2KB */
    static uint8_t pkt[8 + 128];
    int frames = 0;

    while (frames < 4)
    {
        int b0, b1, len, i;
        uint16_t need, sum;
        uint8_t s;

        /* resync to AA 55 */
        while (cdc_rx_avail() >= 2)
        {
            b0 = cdc_rx_peek(0);
            b1 = cdc_rx_peek(1);
            if (b0 == Uart_Sync_Head1 && b1 == Uart_Sync_Head2)
                break;
            cdc_rx_drop(1);
        }

        if (cdc_rx_avail() < 4)
            return;

        len = cdc_rx_peek(3);
        if (len < 0 || len > 120)
        {
            cdc_rx_drop(1);
            return;
        }

        need = (uint16_t)(8 + len); /* AA 55 Cmd Len data sumL sumH 55 AA */
        if (cdc_rx_avail() < need)
            return;

        for (i = 0; i < (int)need; i++)
            pkt[i] = (uint8_t)cdc_rx_get();

        /* tail check */
        if (pkt[need - 2] != Uart_Sync_Head2 || pkt[need - 1] != Uart_Sync_Head1)
            continue;

        sum = (uint16_t)pkt[2] + (uint16_t)pkt[3];
        for (i = 0; i < len; i++)
            sum = (uint16_t)(sum + pkt[4 + i]);

        if (pkt[4 + len] != (uint8_t)(sum & 0xFF) ||
            pkt[5 + len] != (uint8_t)(sum >> 8))
            continue;

        /* fill IAP_Deal_Buf: Cmd Len data... */
        IAP_Deal_Buf[0] = pkt[2];
        IAP_Deal_Buf[1] = pkt[3];
        if (len)
            memcpy(&IAP_Deal_Buf[2], &pkt[4], (size_t)len);

        s = RecData_Deal();
        if (s != ERR_End)
            cdc_reply(s ? ERR_ERROR : ERR_SUCCESS);
        frames++;
    }
}
