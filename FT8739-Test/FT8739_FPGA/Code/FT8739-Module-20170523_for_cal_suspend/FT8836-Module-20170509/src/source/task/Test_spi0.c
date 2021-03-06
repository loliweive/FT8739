/*******************************************************************************
* Copyright (C) 2016-2018, FocalTech Systems (R)，All Rights Reserved.
*
* File Name: Test_spi0.c
*
*    Author: tangsujun
*
*   Created: 2017-02-06
*
*  Abstract: 
*
* Reference:
*
*******************************************************************************/
#include "Test_SPI0.h"
#include "Test_SPI0_dma.h"

#if _TEST_SPI0_EN

/*******************************************************************************
* 2.Private constant and macro definitions using #define
*******************************************************************************/
#define SPI0_AUTO_CLR       0 // 
#define SPI_VECTOR 10




//位功能


#define CHIP_ID_H     0x88
#define CHIP_ID_L     0xA6

typedef enum
{
    HCMD_NOF          = 0x00,
    HCMD_TOUCH        = 0x01,
    HCMD_RST          = 0x07,
    HCMD_APP_START    = 0x08,
    HCMD_START1       = 0x55,
    HCMD_DATA_R       = 0x85,
    HCMD_DATA_RN      = 0x86,
    HCMD_ID           = 0x90,
    HCMD_VENDOR_ID    = 0xA8,
    HCMD_DATA_W       = 0xAE,
    HCMD_ECC          = 0xCC,
    HCMD_BOOT_STATE   = 0xD0,
    HCMD_ROM_VERSION  = 0xDB,

    HCMD_RST_STA       = 0xE0,
    HCMD_XRST_STA      = 0xE1,
    HCMD_Upgrade_Flg   = 0xE2,
    HCMD_POR_CLR       = 0xE3,
    HCMD_LCDReLoad_STA = 0xE4,
} ENUM_RomCmd;


/*******************************************************************************
* 3.Private enumerations, structures and unions using typedef
*******************************************************************************/

/*******************************************************************************
* 4.Static variables
*******************************************************************************/
UINT8 g_nDummyData = 0;
UINT16 g_ucHostIdx     = 0;          // Host每次通讯RW的字节序号
UINT8 g_ucHostCmdNow  = 0;          // Host--发来的第一个字节，即命令头
volatile UINT32 g_usAppStartAddr =0;// 数据包开始地址
UINT8 *pAppAddr = (UINT8 *)&g_usAppStartAddr;

UINT16 g_ucAppLen       = 0;        // 接收数据包大小，这里只接收不处理

UINT8 g_ucChipId[2] = {CHIP_ID_H,CHIP_ID_L};
UINT8  g_ucDataBuf[16];             // 长命令缓冲区
UINT8  temp_i=0;



/*******************************************************************************
* 5.Global variable or extern global variabls/functions
*******************************************************************************/

/*******************************************************************************
* 6.Static function prototypes
*******************************************************************************/


/*******************************************************************************
*   Name: SPI0_Init
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI0_Init(void)
{

    SPI0CKEN = 1;
    SPI0CON5 =0xff;
    SPI0CON2 =0x00;
    SPI0CON3 =0;
    SPI0CON4 =0X00;
    /* IO配置: P05-->P02  MOSI MISO CLK CS */
    P0MOD |= BITn(5) + BITn(4)+BITn(3) + BITn(2);
    P0SEL |= BITn(5) + BITn(4)+BITn(3) + BITn(2);
    
    P0DIR |= BITn(4)+BITn(3) + BITn(2);
    P0DIR &= ~ BITn(5);
    
    SPI0_EI = 0;
    




    //中断标志使能
    SPI0RI_EN = 1;
    SPI0TI_EN = 0;



     
#if SPI0_AUTO_CLR
    SPI0RX_AUTO_CLR =1;
    SPI0TX_AUTO_CLR =1;

#endif

    DBG_SPI("\n SPI0CON1=%02x ",SPI0CON);
    DBG_SPI("\n SPI0CON2=%02x ",SPI0CON2);
    DBG_SPI("\n SPI0CON3=%02x ",SPI0CON3);
    DBG_SPI("\n SPI0CON4=%02x ",SPI0CON4);

    
}
/*******************************************************************************
*   Name: SPI0_Get_Byte_Num
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
UINT32 SPI0_Get_Byte_Num(void)
{    
    UINT16 byte_num_h,byte_num_m,byte_num_l;
    UINT32 byte_num;
    
    SPI0CON4 =0x00<<6;
    byte_num_h  =SPI0CON4&0x3f;
    SPI0CON4 =0x01<<6;
    byte_num_m  =SPI0CON4&0x3f;
    SPI0CON4 =0x02<<6;
    byte_num_l  =SPI0CON4&0x3f;

    byte_num    =(byte_num_h<<12)|(byte_num_m<<6)|(byte_num_l);

    return  byte_num;
    
    
}
/******************************************************************************
*
*   Name: DrvRomRegWR
*  Brief:

    1.idx: 0-->65535
    2.调用时先写再加

*  Input:
* Output:
* Return:
******************************************************************************/
UINT8 DrvRomRegWR(UINT16 idx,UINT8 ucData)
{
    UINT32 temp=0;
    volatile UINT8 ch = 0xef;
    
    switch (g_ucHostCmdNow)
    {


        // 90 xx xx xx
        case HCMD_ID:
        {
            if ((idx >= 3)&&(idx < 5))
            {
                
                ch = g_ucChipId[idx-3];
            }


            break;
        }

        //AE AH AM AL LH LL D0 D1 ... Dn
        case HCMD_DATA_W:
        {            
            switch (idx)
            {
                case 0:
                {
                    
                    g_ucHostCmdNow = HCMD_NOF;
                   
                    break;
                }
             
                case 1:
                case 2:
                case 3:
                {
                    pAppAddr[idx] = ucData;
                    break;
                }
              
                case 4:
                {
                    g_ucAppLen = ((UINT16)ucData<<8);
                    break;
                }
                case 5:
                {
                    g_ucAppLen += ucData;
                    break;
                }
                // 6-->255
                default:
                {
                 //   g_ucAppEcc ^= ucData;
                    PRAM_ADDR8(g_usAppStartAddr + idx - 6) = ucData;
                    break;
                }
            }

            break;
        }

        //85 AH AM AL  --> set read address
        case HCMD_DATA_R:
        {
            
            if((idx>0)&&(idx<4))
            {
                pAppAddr[idx] = ucData;                
            }
            if(idx >= 3)
            {
                ch = PRAM_ADDR8(g_usAppStartAddr + idx -3); 
                
            }
            break;    

        }
        //86 AH AM AL LH LL --> set read address read len
        case HCMD_DATA_RN:
        {
            
            if((idx>0)&&(idx<4))
            {
                pAppAddr[idx] = ucData; 
                g_ucAppLen =0;
            }
            if((idx>3)&&(idx<6))
            {
                g_ucAppLen<<=8;
                g_ucAppLen|=ucData;

            }
            if(idx >= 5)
            {
                ch = PRAM_ADDR8(g_usAppStartAddr + idx -5); 
                
            }
            break;    

        }
        //07
        //08
        case HCMD_APP_START:
        case HCMD_RST:
        {
            SFTRST  = 0xFF;
            break;
        }        


        case HCMD_BOOT_STATE:
        {
            ch = 0X66;
            break;
        }
        default:
        {
            break;
        }
    }
    
    return ch;
}

/******************************************************************************
*
*   Name: SPI0_IRQHandler
*  Brief:
*  Input:
* Output:
* Return:
******************************************************************************/
void SPI0_IRQHandler(void) interrupt 11
{
    UINT8 ch = 0;  

    
    if (SPI0RI)
    {
    //    SPI0RI_CLR = 1;
        ch = SPI0BUF;  
        
        if(SPI0_START)
        {        
            SPI0_START_CLR =1;
            g_ucHostIdx =0;
            /* 接收 Host 写的第一个数据，保存为cmd */
            g_ucHostCmdNow = ch;
            SPI0CON3 =1;
            SPI0_CLR_NUM =1;
            
        }
        //g_ucDataBuf[g_ucHostIdx&0x0f]=ch;
        // idx 先调用，后++
        SPI0BUF=DrvRomRegWR(g_ucHostIdx,ch);        

        
    }
#if 0
    //------------------------------------------------------------------------
    //每一帧读的第一个数据，总是提前准备，因此中断来后的index是从1开始的
    if (SPI0TI)
    {
        SPI0TI_CLR = 1;
        SPI0BUF = DrvRomRegWR(g_ucHostIdx,0); //place next byte
    }
#endif    
    g_ucHostIdx++;
    SPI0CON5 =0xff;
//    P0_7 =1;
}



/******************************************************************************
* Name:void Test_SPI0(void)
* Brief:
* Input:
* Output:
* Return:
******************************************************************************/
void Test_Byte(void)
{
    UINT8 BUF[8];
    SPI0_EI = 0;
    SPI0BUF=0X55;
    
    SPI0RI_EN = 1;
    SPI0TI_EN = 1;
    
    while(1)
    {
        while(!SPI0RI);

        BUF[0]=SPI0CON5;
        BUF[1]=SPI0BUF;
        BUF[2]=SPI0_START;
        SPI0CON5=0xff;
        SPI0BUF=0X55;
        SPI0_START_CLR=0;
        DBG_SPI("\n a SPI0CON5=%02x ",BUF[0]);
        DBG_SPI("\n SPI0BUF=%02x ",BUF[1]);
        DBG_SPI("\n SPI0_START=%02x ",BUF[2]);
        

        DBG_SPI("\n b SPI0CON5=%02x ",SPI0CON5);
        DBG_SPI("\n SPI0_START=%02x ",SPI0_START);
        
        
        
    }
}

/******************************************************************************
* Name:void Test_SPI0(void)
* Brief:
* Input:
* Output:
* Return:
******************************************************************************/
void Test_N_Byte(void)
{
    UINT8 buf[32],i=0,j=0;
    UINT8 txtemp=0x55;
    SPI0_EI = 0;
    SPI0BUF=txtemp++;
    
    while(1)
    {
        while(!SPI0RI);
        buf[i++]=SPI0BUF;        
        SPI0RI_CLR=1;        
        SPI0BUF=txtemp++;
        
        if(SPI0_STOP)
        {
            SPI0_STOP_CLR=1;
            for(j=0;j<i;j++)
            {
                DBG_SPI("\n SPI0BUF[%x]=%02x ",j,buf[j]);
            }
            i=0;
                         
        }
        
        
        
    }
}
/******************************************************************************
* Name:void Test_SPI0(void)
* Brief:
* Input:
* Output:
* Return:
******************************************************************************/
void Test_SPI0(void)
{
    UINT8 i=0,j=0;
    SPI0_Init();


    //Test_Byte();
    //Test_N_Byte();
    SPI0_EI = 1;
    
    while(1)
    {

        if(SPI0_STOP)
        {
            SPI0_STOP_CLR=1;
        //    for(i=0;i<16;i++)
       //     DBG_SPI("\n g_ucDataBuf[%02x]=%02x ",i,g_ucDataBuf[i]); 
            DBG_SPI("\n SPI0CON3=%02x ",SPI0CON3); 
            DBG_SPI("\n Byte_Num=%04x ",SPI0_Get_Byte_Num());
            SPI0CON3 =0;
        }
 
        

    }




}



#endif//_TEST_SPI0_EN


