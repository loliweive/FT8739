/*******************************************************************************
* Copyright (C) 2013-2014, FocalTech Systems (R)��All Rights Reserved.
*
* File Name: Test_WDT.c
*
*    Author: xinkunpeng
*
*   Created: 2014-04-21
*
*  Abstract: ���ʱ��12.096ms~25.2S,����2us/32us
*
* Reference: test WDT introduction:

/*******************************************************************************
* 1.Included header files
*******************************************************************************/
#include "Test_Counter.h"

#if _TEST_CNT_EN
/*******************************************************************************
* 2.Private constant and macro definitions using #define
*******************************************************************************/
//time0
#define T0MODE                            0x02    //����λΪT1,����λΪT0
#define IRQTIME0_VALUE                    1       //(MS)
#define IRQTIME0_LOAD()                   TMOD|=T0MODE;TH0=IRQTIME0_VALUE;TL0=0

#define INT1_TRI_MODE                     0// 0:LOW level triggered 1:falling edg
#define TEST_CNT_WK                       1//0:������CNT WK   1:����CNT WK
#define COUNT_MS                          100//COUNT���ʱ�䣬ms 

//��������ģʽ���ֻ�ܽ���һ��
#define IDLE_MODE                        0//0:������idle     1:����idle
#define STANDBY_MODE                     1//0:������standy 1:����standy
#define STOP_MODE                        0// 0:������stop   1:����stop



/*******************************************************************************
* 3.Private enumerations, structures and unions using typedef
*******************************************************************************/

/*******************************************************************************
* 4.Static variables
*******************************************************************************/
UINT16 m_usCnt  = 0;
UINT32 Counter  = 0;
UINT8 g_nWKsrc  = 0x00;
BOOLEAN g_bWKsrcOut = 1;
BOOLEAN g_bCntFlag = 0;
/*******************************************************************************
* 5.Global variable or extern global variabls/functions
*******************************************************************************/

/*******************************************************************************
* 6.Static function prototypes
*******************************************************************************/
/*******************************************************************************
* Name: DrvSysTimerInit
* Brief:  ϵͳ��ʱ����ʼ��
* Input:
* Output:
* Return:
*******************************************************************************/
void DrvSysTimer0_Init(void)
{
    TR0 = 0;
    IRQTIME0_LOAD();
    TR0 = 1;
    ET0 = 1;
}

/*******************************************************************************
*   Name:
*  Brief:
*  Input:
* Output:
* Return: INT0�жϳ�ʼ������
*******************************************************************************/
void ExternInt0Init(void)
{
    IT0   = 0;   // 0:LOW level triggered 1:falling edge

    IPL0  = 0;
    IPH0  = 0;

    IPLX0 = 1;
    IPHX0 = 1;  //�ж�0���ȼ��������

    EX0   = 1;
}

/*******************************************************************************
*   Name:
*  Brief:
*  Input:
* Output:
* Return: INT1�жϳ�ʼ������
*******************************************************************************/
void ExternInt1Init(void)
{
    IT1 = INT1_TRI_MODE;   // 0:LOW level triggered 1:falling edge
    EX1 = 1;

    IPLX1 = 1;
    IPLX1 = 1;

    EA = 1;
}

/*******************************************************************************
* Name: Timer_IRQHandler
* Brief:   Timer �ж���Ӧ����
* Input:
* Output:
* Return:
*******************************************************************************/
void Timer0_IRQHandler(void) interrupt  1
{
    m_usCnt++;
}

/*******************************************************************************
*   Name:
*  Brief:
*  Input:
* Output:
* Return: INT0�ж���Ӧ����
*******************************************************************************/
void Exint0_IRQHandler(void) interrupt 0
{
    if (g_bWKsrcOut)
    {
        g_nWKsrc = IWKSTA;
        g_bWKsrcOut = FALSE;
    }

    if(RTC_CNT_INT)
    {
        DBG_FLOW("\n\rcount int0");
        RTC_CNT_WKEN= 0;
    }
}

/*******************************************************************************
*   Name:
*  Brief:
*  Input:
* Output:
* Return: INT1�ж���Ӧ����
*******************************************************************************/
void Exint1_IRQHandler(void) interrupt 2
{

    // ��cnt
    CNT_CLEAR     = 1;
    //while(RTC_CNT_INT);//�ȴ�RTC_CNT_INTΪ0�����Ϊһ��clk��ʱ��,���ڲ���Ҫ�ˡ�Ӳ���Ĺ�����
    CNT_START     = 0;          
    DBG_FLOW("\n\rcnt int1");
    g_bCntFlag = TRUE;


}

/*******************************************************************************
* Name:Test_Counter
* Input:
* Output:
* function:
*********************************************************************************/
void Test_Counter(void)
{
    UINT8 i = 0;
    UINT8 Temp[3];
    UINT32 cnt;
    // cnt clk enable
    CLKEN_500K = 1;    
    CNTCKEN = 1;

    //DrvSysTimer0_Init();
    

#if 0
    // ����CNT����ʱ��
    CNTIVH = 0x00;
    CNTIVM = 0x00;
    CNTIVL = 0x02;
#else
    // ����CNT����ʱ��
    cnt = COUNT_MS*500;//COUNT_MS = cnt *2us/1000
    CNTIVH = cnt>>16;
    CNTIVM = cnt>>8;
    CNTIVL = cnt;

    
#endif

    CNT_WKEN  = 1; //enable CNT function
    
#if TEST_CNT_WK
    RTC_CNT_WKEN  = 1; //enable wake up (RTC & CNT use same reg)
#endif

    // clear cnt
    //CNT_CLEAR       = 1;
    //CNT_START       = 1;
    
    
#if TEST_CNT_WK  
    ExternInt0Init();
#endif

    ExternInt1Init();
    DBG_MODE("\n\rCNT test is start");

    //P0_6 = 0;
    //CNT_START = 1;
    while (1)
    {
        CNT_START = 1;    
        m_usCnt = 0;
        g_bCntFlag = 0;
        
#if TEST_CNT_WK        
       RTC_CNT_WKEN  = 1; //enable wake up (RTC & CNT use same reg)
#endif


    #if 0   //��ӡ����ֵ,��CNT_START = 0��ʱ�����CNT CLEAR�ܷ������

        DelayMs(6);
        CNT_START     = 0;
        Temp[0] = CNTIVH;
        Temp[1] = CNTIVM;
        Temp[2] = CNTIVL;
        DBG_CNT("\n\rCNTIVH=%d",Temp[0]);   
        DBG_CNT("\n\rCNTIVM=%d",Temp[1]);          
        DBG_CNT("\n\rCNTIVL=%d",Temp[2]);
        CNT_CLEAR     = 1;
        DelayMs(7);
        Temp[0] = CNTIVH;
        Temp[1] = CNTIVM;
        Temp[2] = CNTIVL;
        DBG_CNT("\n\rCNTIVH=%d",Temp[0]);   
        DBG_CNT("\n\rCNTIVM=%d",Temp[1]);   
        DBG_CNT("\n\rCNTIVL=%d",Temp[2]);
        DelayMs(7);
        Temp[0] = CNTIVH;
        Temp[1] = CNTIVM;
        Temp[2] = CNTIVL;
        DBG_CNT("\n\rCNTIVH=%d",Temp[0]);   
        DBG_CNT("\n\rCNTIVM=%d",Temp[1]);   
        DBG_CNT("\n\rCNTIVL=%d",Temp[2]);        
    #endif
        
    
#if IDLE_MODE
        DBG_MODE("\njoin idle mode");
        DelayMs(10);
        DrvSysPowerIdle();
        DBG_MODE("\ncount wakeup..ok");
#endif

#if STANDBY_MODE
        DBG_MODE("\njoin standy mode");
        DelayMs(50);
        DrvSysPowerStandby();
        DBG_MODE("\ncount wakeup..ok");
#endif

#if STOP_MODE
        DBG_MODE("\njoin stop mode");
        DelayMs(50);
        DrvSysPowerStop();
        DBG_MODE("\ncount wakeup..ok");
#endif


        while (!g_bCntFlag);
        if (g_bCntFlag)
        {
            DBG_CNT("\n\rWK src:%x",g_nWKsrc);
            DBG_CNT("\n\rCNT time=%dms",m_usCnt);
        }
        DelayMs(1000);
    }
}
#endif
