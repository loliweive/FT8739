/*******************************************************************************
* Copyright (C) 2013-2014, FocalTech Systems (R)��All Rights Reserved.
*
* File Name: Test_LED.c
*
*    Author: xinkunpeng
*
*   Created: 2014-06-06
*
*  Abstract:
*
* Reference:
*******************************************************************************/

/*******************************************************************************
* 1.Included header files
*******************************************************************************/
#include "Test_LED.h"

#if _TEST_LED_EN
/*******************************************************************************
* 2.Private constant and macro definitions using #define
*******************************************************************************/

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
*   Name: Test_LED
*  Brief:
*  Input:
* Output:
* Return:
*******************************************************************************/
void Test_LED(void)
{
    UINT8 i = 0;
    UINT16 j;
    UINT8 flag;
    UINT8 tmp1,tmp2;
    UINT8 counter;

    flag = 0;
    counter = 5;
    LED_32K_EN = 1;
    LEDCKEN = 0;

    tmp1 = LED_EN;
    tmp2 = LED_H_NUM;
    // ��ӡ�Ĵ���Ĭ��ֵ��LED_H_NUM(0xFF)���³����ʱ��ᱻI2C_DEBUGдΪ0x11
    DBG_MODE("\n\rLED Test is Start:");
    DBG_LED("\n\rLED_EN: %02x", tmp1);      
    DBG_LED("\n\rLED_H_NUM: %02x", tmp2);

    //LED_EN = 0x0F;
    LED_H_NUM = 0x00;
    
    //while (counter--)
    while(1)
    {
        #if 0
        //�Ĵ�����д����
        i++;
        LED_EN    = i;
        LED_H_NUM = i;
        DBG_LED("\n\rLED_EN: %02x", LED_EN); 
        DBG_LED("\n\rLED_H_NUM: %02x", LED_H_NUM);
        DelayMs(200);
        #else 
        //ģ����������
        LED_1_EN = 1;
        if (flag == 0)
        {
            LED_H_NUM = i;
        }
        else
        {
            LED_H_NUM = 43- i;
        }
        DelayMs(50);
        i++;
        j++;

        if (i > 43)
        {
            flag = ~flag;
            i = 0;
        }

        #if 0  
        if (j > 5000)
        {
            LED_EN = 0x00;
            j = 0;
        }
        else if (j > 3000)
        {
            LED_EN = 0x01;
        }
        else if (j > 2000)
        {
            LED_EN = 0x03;
        }
        else if (j > 1000)
        {
            LED_EN = 0x07;
        }
        #endif
        DBG_LED("\n\rLED_EN_read: %02x", LED_EN);
        DBG_LED("\n\rLED_H_NUM_read: %02x", LED_H_NUM);
        #endif
    }
    DBG_MODE("\n\rLED Test is End!\n");
}
#endif

