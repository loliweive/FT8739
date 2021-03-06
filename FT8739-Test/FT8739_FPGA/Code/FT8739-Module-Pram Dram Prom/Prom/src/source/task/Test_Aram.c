/*******************************************************************************
* Copyright (C) 2013-2014, FocalTech Systems (R)��All Rights Reserved.
*
* File Name: Test_Aram.c
*
*    Author: xinkunpeng
*
*   Created: 2014-04-22
*
*  Abstract:
*
* Reference:
* Version:
* 0.1:
*
*******************************************************************************/

/*******************************************************************************
* 1.Included header files
*******************************************************************************/
#include "Test_Aram.h"

#if _TEST_ARAM_EN
/*******************************************************************************
* 2.Private constant and macro definitions using #define
*******************************************************************************/
#define ARAM_START_ADDRESS   0xA000
#define ARAM_END_ADDRESS     0xA5A0
#define ARAM_LENGTH          0x5A0

#define TEST_ARAM1         0x5A
#define TEST_ARAM2         0xA5
#define TEST_ARAM3         0x5AA5//0x55AA
#define TEST_ARAM4         0xA55A//0xAA55
#define TEST_ARAM5         0x5AA55AA5//0x12345678//0x55AA55AA
#define TEST_ARAM6         0xA55AA55A//0x11223344//0xAA55AA55

UINT8  XRAM *ucpAsmram;
UINT16 XRAM *uspAsmram;
UINT32 XRAM *ulpAsmram;

UINT8  ucpAsmram1;
UINT8  XRAM ucReadAram;
UINT16 XRAM usReadAram;
UINT32 XRAM ulReadAram;
/*******************************************************************************
* 3.Private enumerations, structures and unions using typedef
*******************************************************************************/

/*******************************************************************************
* 4.Static variables
*******************************************************************************/

/*******************************************************************************
* 5.Global variable or extern global variabls/functions
*******************************************************************************/

/*******************************************************************************
* 6.Static function prototypes
*******************************************************************************/

/*******************************************************************************
*   Name:
*  Brief:
*  Input:
* Output:
* Return:
*******************************************************************************/
void Test_Aram(void)
{
    UINT16 i;
    UINT16 temp_H,temp_L;

    AFEACKEN = 1;
    ARAM_SEL = 1;   /* ARAM_SEL: 1: Mcu write and read; 0: others */

    DBG_MODE("\n\rread and write asmram by byte is start:");
    ucpAsmram = ARAM_START_ADDRESS;
    for (i = 0; i < (ARAM_LENGTH); i++)
    {
        if (!(i&0x01))
            *(ucpAsmram+i) = TEST_ARAM1;
        else
            *(ucpAsmram+i) = TEST_ARAM2;
    }
    for (i = 0; i < (ARAM_LENGTH); i++)
    {
        ucReadAram = *(ucpAsmram+i);
        if (!(i&0x01))
        {
            if (ucReadAram !=  TEST_ARAM1)
            {
                DBG_ERROR("\n\r%02x Error",ucReadAram);
                break;
            }
        }
        else
        {
            if (ucReadAram !=  TEST_ARAM2)
            {
                DBG_ERROR("\n\r%02x Error",ucReadAram);
                break;
            }
        }
    }
    DBG_MODE("\n\rread and write asmram by byte is End!");


    DBG_MODE("\n\rread and write asmram by word is start:");
    uspAsmram = ARAM_START_ADDRESS;
    for (i = 0; i < (ARAM_LENGTH>>1); i++)
    {
        if (!(i&0x01))
            *(uspAsmram+i) = TEST_ARAM3;
        else
            *(uspAsmram+i) = TEST_ARAM4;
    }
    for (i = 0; i < (ARAM_LENGTH>>1); i++)
    {
        usReadAram = *(uspAsmram+i);
        if (!(i&0x01))
        {
            if (usReadAram !=  TEST_ARAM3)
            {
                DBG_ERROR("\n\r%04x Error",usReadAram);
                break;
            }
        }
        else
        {
            if (usReadAram !=  TEST_ARAM4)
            {
                DBG_ERROR("\n\r%04x Error",usReadAram);
                break;
            }
        }
    }
    DBG_MODE("\n\rread and write asmram by word is End!");

    DBG_MODE("\n\rread and write asmram by double word is start:");
    ulpAsmram = ARAM_START_ADDRESS;
    for (i = 0; i < (ARAM_LENGTH>>2); i++)
    {
        if (!(i&0x01))
            *(ulpAsmram+i) = TEST_ARAM5;
        else
            *(ulpAsmram+i) = TEST_ARAM6;
    }
    for (i = 0; i < (ARAM_LENGTH>>2); i++)
    {
        ulReadAram = *(ulpAsmram+i);
        if (!(i&0x01))
        {
            if (ulReadAram !=  TEST_ARAM5)
            {
                temp_H = ulReadAram>>16;
                temp_L = ulReadAram&0x0000FFFF;
                DBG_ERROR("\n\r0x%04x%04x Error",temp_H,temp_L);
                break;
            }
        }
        else
        {
            if (ulReadAram !=  TEST_ARAM6)
            {
                temp_H = ulReadAram>>16;
                temp_L = ulReadAram&0x0000FFFF;
                DBG_ERROR("\n\r0x%04x%04x Error",temp_H,temp_L);
                break;
            }
        }
    }
    ARAM_SEL = 0;
    AFEACKEN = 0;
    DBG_MODE("\n\rread and write asmram by double word is End!\n\r");
}

#endif
