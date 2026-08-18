/********************************** (C) COPYRIGHT *******************************
 * File Name          : iap.c
 * Description        : Flash program/verify for CDC serial IAP
 *
 * Command buffer layout (from CDC frame):
 *   IAP_Deal_Buf[0] = Cmd
 *   IAP_Deal_Buf[1] = Len
 *   IAP_Deal_Buf[2..] = payload data (PROM / VERIFY)
 *
 * APP layout (dual-core combined bin starts at FLASH_Base 0x08006000):
 *   V3F @ 0x00006000, V5F @ 0x00020000  (same image sequential from 0x6000)
 *******************************************************************************/
#include "iap.h"
#include "string.h"
#include "flash.h"
#include "core_riscv.h"

vu32 Program_addr = FLASH_Base;
vu32 Verify_addr = FLASH_Base;
vu8 Verify_Star_flag = 0;
vu8 Fast_Program_Buf[1024 * 8];
vu32 CodeLen = 0;
vu8 End_Flag = 0;
u8 IAP_Deal_Buf[USBD_DATA_SIZE + 4];

#define Size_256B   0x100
#define Size_4KB    0x1000
#define Size_8KB    0x2000

static vu32 Flash_Erase_Page_Size = Size_8KB;

static void IAP_Detect_Page_Size(void)
{
    FLASH_Unlock_Fast();
    if (((*(vu32 *)FLASH_CFGR0_BASE) & (1 << 28)) != 0)
        Flash_Erase_Page_Size = Size_8KB;
    else
        Flash_Erase_Page_Size = Size_4KB;
}

/* 把未满一页的缓冲写入 Flash（跳过 VERIFY 时必须在 END 调用） */
static void IAP_Flush_Remaining(void)
{
    uint32_t i, temp;
    vu8 *PBuf;

    if (CodeLen == 0)
        return;

    IAP_Detect_Page_Size();
    FLASH_Unlock_Fast();

    for (i = 0; ((CodeLen + i) % Size_256B) != 0; i++)
        Fast_Program_Buf[CodeLen + i] = 0xff;

    temp = CodeLen;
    PBuf = Fast_Program_Buf;
    FLASH_ErasePage(Program_addr & (~(Flash_Erase_Page_Size - 1)));
    for (i = 0; i < ((temp + Size_256B - 1) / Size_256B); i++)
    {
        CH32_IAP_Program(Program_addr, (u32 *)PBuf);
        PBuf += Size_256B;
        Program_addr += Size_256B;
    }
    CodeLen = 0;
}

/* 清除下载标志，否则上电一直停在 IAP */
static void IAP_Clear_Flag(void)
{
    IAP_Detect_Page_Size();
    FLASH_Unlock_Fast();
    FLASH_ErasePage(CalAddr & (~(Flash_Erase_Page_Size - 1)));
    /* 确认已擦成 0xFFFFFFFF；若仍异常再写 0 */
    if (*(vu32 *)CalAddr == CheckNum)
    {
        FLASH_Unlock_Fast();
        FLASH_ProgramWord(CalAddr, 0x00000000);
    }
    FLASH->CTLR |= ((uint32_t)0x00008000);
    FLASH->CTLR |= ((uint32_t)0x00000080);
}

u8 RecData_Deal(void)
{
    uint32_t i, temp;
    uint8_t s;
    uint8_t cmd = IAP_Deal_Buf[0];
    uint8_t Lenth = IAP_Deal_Buf[1];
    uint8_t *data = &IAP_Deal_Buf[2];
    vu8 *PBuf = NULL;

    switch (cmd)
    {
    case CMD_IAP_ERASE:
        IAP_Detect_Page_Size();
        Program_addr = FLASH_Base;
        Verify_addr = FLASH_Base;
        CodeLen = 0;
        Verify_Star_flag = 0;
        s = ERR_SUCCESS;
        break;

    case CMD_IAP_PROM:
        for (i = 0; i < Lenth; i++)
            Fast_Program_Buf[CodeLen + i] = data[i];
        CodeLen += Lenth;
        if (CodeLen >= Flash_Erase_Page_Size)
        {
            FLASH_Unlock_Fast();
            FLASH_ErasePage(Program_addr);
            PBuf = Fast_Program_Buf;
            for (uint32_t j = 0; j < (Flash_Erase_Page_Size / 256); j++)
            {
                CH32_IAP_Program(Program_addr, (u32 *)PBuf);
                CodeLen -= Size_256B;
                PBuf += Size_256B;
                Program_addr += Size_256B;
            }
            for (i = 0; i < CodeLen; i++)
            {
                Fast_Program_Buf[i] = *(PBuf);
                PBuf++;
            }
        }
        s = ERR_SUCCESS;
        break;

    case CMD_IAP_VERIFY:
        if (Verify_Star_flag == 0)
        {
            Verify_Star_flag = 1;
            IAP_Flush_Remaining();
        }

        s = ERR_SUCCESS;
        for (i = 0; i < Lenth; i++)
        {
            if (data[i] != *(u8 *)(Verify_addr + i))
            {
                s = ERR_ERROR;
                break;
            }
        }
        Verify_addr += Lenth;
        break;

    case CMD_IAP_END:
        /* 关键 VERIFY 时也必须落盘剩余数据 + 清 flag，否则重插仍进 IAP */
        IAP_Flush_Remaining();
        IAP_Clear_Flag();
        Verify_Star_flag = 0;
        Program_addr = FLASH_Base;
        Verify_addr = FLASH_Base;
        End_Flag = 1;
        s = ERR_End;
        break;

    case CMD_JUMP_IAP:
        s = ERR_SUCCESS;
        break;

    default:
        s = ERR_ERROR;
        break;
    }

    (void)temp;
    return s;
}

/* APP 向量粗检：非空、非全 F、非 IAP 空标志 */
u8 IAP_App_Is_Valid(void)
{
    vu32 w0 = *(vu32 *)FLASH_Base;
    vu32 w1 = *(vu32 *)(FLASH_Base + 4);

    if (w0 == 0xFFFFFFFFu || w0 == 0x00000000u || w0 == 0xe339e339u)
        return 0;
    if (w1 == 0xFFFFFFFFu)
        return 0;
    return 1;
}

void GPIO_Cfg_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

u8 PA0_Check(void)
{
    u8 i, cnt = 0;
    GPIO_Cfg_init();
    for (i = 0; i < 10; i++)
    {
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
            cnt++;
        Delay_Ms(5);
    }
    if (cnt > 6)
        return 0;
    return 1;
}
