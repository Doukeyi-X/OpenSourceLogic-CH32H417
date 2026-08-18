/********************************** (C) COPYRIGHT *******************************
 * File Name          : hardware.c
 * Description        : IAP entry — USB CDC serial upgrade
 *
 * Boot decision (command mode):
 *   - CalAddr == CheckNum  → stay IAP (APP requested upgrade via 0xAE)
 *   - APP valid && flag clear → jump APP @ 0x6000
 *   - APP invalid → stay IAP
 *
 * END command must: flush last program page + erase CalAddr flag, then jump.
 *******************************************************************************/
#include "hardware.h"
#include "ch32h417_usbhs_device.h"
#include "ch32h417_gpio.h"
#include "iap.h"
#include "usb_inf.h"

#define UPGRADE_MODE_COMMAND   0
#define UPGRADE_MODE_IO        1
#define UPGRADE_MODE           UPGRADE_MODE_COMMAND

void IAP_2_APP(void)
{
    USB_Init(DISABLE);
    Delay_Ms(30);
    DBG_PRINT("jump APP app0=%08lx\r\n",
              (unsigned long)(*(vu32 *)FLASH_Base));
    Delay_Ms(10);
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOA, DISABLE);
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOB, DISABLE);
    Delay_Ms(5);

    RCC_DeInit();
    NVIC_EnableIRQ(Software_IRQn);
    NVIC_SetPendingIRQ(Software_IRQn);
}

void Hardware(void)
{
    vu32 flag = *(vu32 *)CalAddr;
    vu32 app0 = *(vu32 *)FLASH_Base;

    DBG_PRINT("IAP boot app0=%08lx flag=%08lx valid=%d\r\n",
              (unsigned long)app0, (unsigned long)flag,
              (int)IAP_App_Is_Valid());

#if UPGRADE_MODE == UPGRADE_MODE_COMMAND
    /*
     * 与原厂逻辑一致：
     *   APP 非空 且  flag != CheckNum  → 跳 APP
     *   flag == CheckNum               → 停 IAP 等升级
     *   APP 空                         → 停 IAP
     */
    if (IAP_App_Is_Valid())
    {
        if (flag != CheckNum)
        {
            IAP_2_APP();
            while (1)
                ;
        }
        DBG_PRINT("IAP flag set, stay for upgrade\r\n");
    }
    else
    {
        DBG_PRINT("APP invalid, stay IAP\r\n");
    }
#elif UPGRADE_MODE == UPGRADE_MODE_IO
    if (PA0_Check() == 0)
    {
        IAP_2_APP();
        while (1)
            ;
    }
#endif

    USB_Init(ENABLE);
    DBG_PRINT("IAP CDC ready (1A86:5539)\r\n");

    while (1)
    {
        CDC_IAP_Poll();
#if UPGRADE_MODE == UPGRADE_MODE_COMMAND
        if (End_Flag)
        {
            Delay_Ms(30);
            IAP_2_APP();
            while (1)
                ;
        }
#endif
    }
}
