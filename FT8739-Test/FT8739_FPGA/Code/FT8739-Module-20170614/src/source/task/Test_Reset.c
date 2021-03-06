/*******************************************************************************
* Copyright (C) 2012-2014, FocalTech Systems (R)��All Rights Reserved.
*
* File Name: Test_Reset.c
*
*    Author: 
*
*   Created: 2017-4-19
*
*  Abstract:
*
* Reference:
*
*   Version:
*
*******************************************************************************/

/*******************************************************************************
* 1.Included header files
*******************************************************************************/
#include "Test_Reset.h"

#if _TEST_RST_EN
/*******************************************************************************
* 2.Private constant and macro definitions using #define
*******************************************************************************/

// Software reset test
//#define TEST_SW_RESET

// 000:POR Reset test in boot program-���õ�������
//#define TEST_PORRST_CNT

// 001:WDT Reset--need to enable WDT Test EN
//#define TEST_WDTRST_CNT
//extern int WDT_Init();

// 010:EXT Reset-���õ�������
//#define TEST_EXTRST_CNT

// 011: LCD Reset-���õ�������
//#define TEST_LCDRST_CNT

// 100: ESD Reset-���õ�������
//#define TEST_ESDRST_CNT

// 101: LVD Reset
//#define TEST_LVDRST_CNT

// ���Է���ESD��LVDʱ��flagλ
#define TEST_XDP_STATUS

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
void Test_Reset()
{
#ifdef TEST_SW_RESET
    while(1)
   {
        DBG_FLOW("\r\n----------------------");
        DBG_FLOW("\r\nSoft Reset Start:");
        DBG_FLOW("\r\nThis program will be reset in 3 seconds.");
        DelayMs(1000);
        DBG_FLOW("3s");
        DelayMs(1000);
        DBG_FLOW("2s");
        DelayMs(1000);
        DBG_FLOW("1s\r\n");
        DelayMs(1000);
        SFTRST = 0xff;
   }
#endif

// POR reset���ԣ�undone��
#ifdef TEST_PORRST_CNT
    CNT_MASK = 0x00;
    DBG_FLOW("\nPORRST_CNT Test");
    while(1)
    {
        DBG_FLOW("\nCNT_POR = %d",CNT_STA);
        DelayMs(1000);
    }
#endif

// WDT reset���ԣ�done��
#ifdef TEST_WDTRST_CNT
    WDT_Init();
    CNT_MASK = 0x01;
    DBG_FLOW("\nWDTRST_CNT Test");
    while(1)
    {
        DBG_FLOW("\nCNT_WDT = %d",CNT_STA);
        // ����CPU_WDTRST_CNT
        if (CNT_STA == 0x20)
        {
            CNT_MASK |= 0xA0;
        }        
        DelayMs(1000);
    }
#endif

// Ext reset���ԣ�done��
#ifdef TEST_EXTRST_CNT
    CNT_MASK = 0x02;
    DBG_FLOW("\nEXTRST_CNT Test");
    while(1)
    {
        DBG_FLOW("\nCNT_EXT = %d",CNT_STA);
        DelayMs(1000);
    }
#endif

// LCD reset���ԣ�going��
#ifdef TEST_LCDRST_CNT
    CNT_MASK = 0x03;
    DBG_FLOW("\nLCDRST_CNT Test");
    while(1)
    {
        DBG_FLOW("\nCNT_LCD = %d",CNT_STA);       
        DelayMs(1000);
    }
#endif

// ESD reset���ԣ�done��
#ifdef TEST_ESDRST_CNT
    CNT_MASK = 0x04;
    ESD_RST_EN = 1;
    DBG_FLOW("\nESDRST_CNT Test");
    while(1)
    {
        DBG_FLOW("\nCNT_ESD = %d",CNT_STA);
        DelayMs(1000);
    }
#endif

// LVD reset���ԣ�done��
#ifdef TEST_LVDRST_CNT
    CNT_MASK = 0x05;
    LVD_MASK = 1;
    DBG_FLOW("\nLVDRST_CNT Test");
    while(1)
    {
        DBG_FLOW("\nCNT_LVD = %d",CNT_STA);
        DelayMs(1000);
    }
#endif

#ifdef TEST_XDP_STATUS
    DBG_FLOW("\nXDP_STATUS Test");
    while (1)
    {
        #if 1
        DBG_FLOW("\nXDP_STATUS = %x",XDP_STA);
        DelayMs(1000);        
        if (ESD_FLAG == 1)
        {
            DBG_FLOW("\nXDP_STATUS = %x",XDP_STA);            
            DBG_FLOW("\nESD Detected");
            ESD_FLAG = 0;            
            DelayMs(1000);
        }
        if (LVD_FLAG == 1)
        {
            DBG_FLOW("\nXDP_STATUS = %x",XDP_STA);            
            DBG_FLOW("\nLVD Detected");
            LVD_FLAG = 0;
            LVD_INT_CLR = 1;        // ����
            DelayMs(1000);
        }        
        #else

        ESD_RST_EN = 0;
        ESD_FLAG = 0;
        LVD_FLAG = 0;
        
        while(1)
        {
            DBG_FLOW("\nESD_FLAG = %d\nESD_STA = %d",ESD_FLAG,ESD_STA);
            DelayMs(10);
        }

        DBG_FLOW("\nLVD Test");
        LVD_FLAG = 0;
        while (LVD_FLAG == 0);
        while(1)
        {
            DBG_FLOW("\nLVD_FLAG = %d",LVD_FLAG);
            DelayMs(10);
        }       
        #endif      // if 0
    }
#endif    //TEST_XDP_STATUS
}

#endif

