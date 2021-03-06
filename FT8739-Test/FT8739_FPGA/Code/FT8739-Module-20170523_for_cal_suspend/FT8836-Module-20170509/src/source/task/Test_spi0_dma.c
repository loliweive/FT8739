/*******************************************************************************
* Copyright (C) 2016-2018, FocalTech Systems (R)，All Rights Reserved.
*
* File Name: Test_spi0_dma.c
*
*    Author: tangsujun
*
*   Created: 2017-03-03
*
*  Abstract: 
*
* Reference:
*
*******************************************************************************/
#include <intrins.h>
#include "Test_SPI0_dma.h"

#if _TEST_SPI0_DMA_EN


/*DMA_CTRL REG*/
#define DMA_TIMEOUT_EN           ((UINT16)1<<6)  
#define DMA_TIMEOUT_DIS          ((UINT16)0<<6)  

#define DMA_TRANS_ERR_EN         ((UINT16)1<<5)  /*timerout或dma结束后host继续发送sck是否要发送dma_err_dat.0:不发送;1:发送*/
#define DMA_TRANS_ERR_DIS        ((UINT16)0<<5)

#define DMA_CRC_EN               ((UINT16)1<<4)  /*dma接收/发送数据是否需要接收/发送crc. 0: no crc;1:crc*/
#define DMA_CRC_DIS              ((UINT16)0<<4)

#define DMA_WD                   ((UINT16)1<<3)  /*dma对ram进行读操作还是写操作.0:读;1:写*/
#define DMA_RD                   ((UINT16)0<<3) 

#define DMA_CLR_RI_EN            ((UINT16)1<<2)
#define DMA_CLR_RI_DIS           ((UINT16)0<<2)

#define DMA_CLR_TI_EN            ((UINT16)1<<1)
#define DMA_CLR_TI_DIS           ((UINT16)0<<1)

#define DMA_EN                   ((UINT16)1<<0)

/*DMA_ADDR_H REG*/
#define DMA_MEM_PRAM             ((UINT16)2<<14)/*dma访问的ram选择. 2'b00:aram;2'b01:dram;2'b1x:pram*/
#define DMA_MEM_DRAM             ((UINT16)1<<14)/*dma访问的ram选择. 2'b00:aram;2'b01:dram;2'b1x:pram*/
#define DMA_MEM_ARAM             ((UINT16)0<<14) /*dma访问的ram选择. 2'b00:aram;2'b01:dram;2'b1x:pram*/

#define ARAM_START_ADDRESS   0xA000
#define ARAM_END_ADDRESS     0xA500
#define ARAM_LENGTH          0x500
/*******************************************************************************
* 2.Private constant and macro definitions using #define
*******************************************************************************/
#define SPI1_DMA_EN         1

#define NUM_B   1280UL//256UL//128UL//256UL




/*******************************************************************************
* 3.Private enumerations, structures and unions using typedef
*******************************************************************************/

/*******************************************************************************
* 4.Static variables
*******************************************************************************/
volatile ST_Spi1DmaRegisters *XRAM pSpi1_DmaRegs = (ST_Spi1DmaRegisters *)SPI1_REG_BASE_ADDR;
volatile ST_Spi0DmaRegisters *XRAM pSpi0_DmaRegs = (ST_Spi0DmaRegisters *)SPI0_REG_BASE_ADDR;

volatile UINT8  XRAM DRAMBUF[2600] ;
/*******************************************************************************
* 5.Global variable or extern global variabls/functions
*******************************************************************************/
// ---------------- POPULAR POLYNOMIALS ----------------
//  CCITT:      x^16 + x^12 + x^5 + x^0                 (0x1021)
//  CRC-16:     x^16 + x^15 + x^2 + x^0                 (0x8005)
//  CCITT Kermit:     x^16 + x^12 + x^5 + x^0           (0x8408)
#define         CRC_16_POLYNOMIALS      0x8408
/*******************************************************************************
* 6.Static function prototypes
*******************************************************************************/

void CRCCCITT(const UINT8* pDataIn, UINT16 iLenIn, UINT16* pCRCOut)     
{     
    UINT16 wTemp = 0,i=0,j=0;      
    UINT16 wCRC = 0xffff;      
    
    for( i = 0; i < iLenIn; i++)      
    {  
        wCRC ^=pDataIn[i];
        
        for(j = 0; j < 8; j++)      
        {      
            if(wCRC&0x01) wCRC=(wCRC>>1)^CRC_16_POLYNOMIALS;
            else          wCRC=(wCRC>>1);
    
        }      
    }      
    
    *pCRCOut = wCRC;     
}

/*******************************************************************************
*   Name: SPI1_Init
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI1_Init(void)
{

    SPI1CKEN = 1;
    SPI1CON2 =0x46;
    SPI1CON3 =0;
    SPI1CON4 =0X10;
    /* IO配置: P10-->P13 */
    P1MOD = 0xFF;//9f;
    
    
    SPI1_EI = 0;

    SPI1MAS = 1;                       // 1：maste mode;
    SPI1_SLVSLELEN  = 1;  //                      // 1：master 模式下slvselo受SPI0CON2[2]控制

    SPI1PH = 1;
    SPI1PO = 0;

    //SPI分频配置
    SPI1CKH  = 0;
    SPI1CKM  = 0;
    SPI1CKL  = 0;                // 00：4分频

    //中断标志使能
    SPI1RI_EN = 1;
    SPI1TI_EN = 1;


     SPI1_SW_IO_ENH=0;
     SPI1_SW_IO_ENL=0;
     

    SPI1RX_AUTO_CLR =1;
    SPI1TX_AUTO_CLR =1;



    DBG_SPI("\n SPI1CON1=%02x ",SPI1CON);
    DBG_SPI("\n SPI1CON2=%02x ",SPI1CON2);
    DBG_SPI("\n SPI1CON3=%02x ",SPI1CON3);
    DBG_SPI("\n SPI1CON4=%02x ",SPI1CON4);
    DBG_SPI("\n P1MOD=%02x ",P1MOD);
    DBG_SPI("\n P1SEL=%02x ",P1SEL);
    DBG_SPI("\n P1DIR=%02x ",P1DIR);
    DBG_SPI("\n P1BUF=%02x ",P1BUF);
    
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
/*******************************************************************************
*   Name: SPI_ReadData
*  Brief: 读1个数据
*  Input:
* Output:
* Return:
*******************************************************************************/
UINT8 SPI1_RW_Byte(UINT8 val)
{
    SPI1BUF = val;                        // Dummy write to output serial clock
    while (!SPI1RI);                    // Wait for the value to be read
    //SPI1RI  = 0;    
    return SPI1BUF;
}

/*******************************************************************************
*   Name: SPI_Read
*  Brief: 读n个数据
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI1_Write(UINT8 far *pBuf, UINT32 len)
{
    UINT32 i;
  
    SPI1SLVELO=0;
#if 0
    for (i = 0; i < len; i++)
    {
        SPI1BUF  = *pBuf++;
        while (!SPI1RI);
        SPI1RI=0;
    }

#else
    SPI1BUF  = *pBuf++;
    for (i = 0; i < (len-1); i++)
    {
        
        while (!SPI1RI);
        SPI1BUF  = *pBuf++;
        SPI1RI=0;
    }
    while (!SPI1RI);
    SPI1RI=0;
#endif
    
    SPI1SLVELO=1;
}
/*******************************************************************************
*   Name: SPI_Read
*  Brief: 读n个数据
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI1_Read(UINT8 far *pBuf, UINT32 len)
{
    UINT32 i;


    SPI1SLVELO=0;
#if 0
    for (i = 0; i < len; i++)
    {
        SPI1BUF  = 0;
        while (!SPI1RI);
       // SPI1RI = 0;
        *pBuf++ = SPI1BUF;
    }

#else
    SPI1BUF  = 0;
    for (i = 0; i < (len-1); i++)
    {
        
        while (!SPI1RI);
        SPI1BUF  = 0;
   //    SPI1RI = 0;
        *pBuf++ = SPI1BUF;
    }
    while (!SPI1RI);
    *pBuf++ = SPI1BUF;
#endif
    
    SPI1SLVELO=1;

}
#if SPI1_DMA_EN
/*******************************************************************************
*   Name: SPI0_DMA_Init
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI1_DMA_Init(void)
{
    SPI0_32K_EN =1;

    pSpi1_DmaRegs->dma_ctrl=0;
    pSpi1_DmaRegs->timeout_ctrl=0;
    pSpi1_DmaRegs->addr_h=0;
    pSpi1_DmaRegs->start_addr_l=0X7788;
    pSpi1_DmaRegs->end_addr_l=0X6677;
    pSpi1_DmaRegs->dma_srcr=0x000f;
    
    
    DBG_SPI("\n dma_srcr=%04x ",pSpi1_DmaRegs->dma_srcr);
    DBG_SPI("\n dma_ctrl=%04x ",pSpi1_DmaRegs->dma_ctrl);
    DBG_SPI("\n dma_crc =%04x ",pSpi1_DmaRegs->dma_crc);
    DBG_SPI("\n dma_timeout=%04x ",pSpi1_DmaRegs->timeout_ctrl);
    DBG_SPI("\n start_addr_h=%04x ",pSpi1_DmaRegs->addr_h);
    DBG_SPI("\n start_addr_l=%04x ",pSpi1_DmaRegs->start_addr_l);
    DBG_SPI("\n end_addr_l=%04x ",pSpi1_DmaRegs->end_addr_l);
    
}
/*******************************************************************************
*   Name: SPI0_DMA_ClrStatus
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI1_DMA_ClrStatus(void)
{    
    pSpi1_DmaRegs->dma_srcr=0x000f;    
}
/*******************************************************************************
*   Name: SPI0_DMA_Wait_Done
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI1_DMA_Wait_Done(void)
{    
    while(pSpi1_DmaRegs->dma_ctrl & BIT_DMA_EN);    
}
/*******************************************************************************
*   Name: SPI0_DMA_SetAddr
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI1_DMA_SetAddr(UINT32 start_addr,UINT32 len,UINT16 mem_sel)
{    
    UINT16 start_addr_h,end_addr_h;
    UINT32 end_addr;
    
    end_addr        =start_addr+len-1;

    start_addr_h    =(start_addr>>16)&0x03;
    end_addr_h      =(end_addr  >>16)&0x03;

    mem_sel &= 0xC000;    
    
    pSpi1_DmaRegs->addr_h=mem_sel|(start_addr_h<<8)|end_addr_h;  
    
    pSpi1_DmaRegs->start_addr_l=start_addr&0xffff;
    pSpi1_DmaRegs->end_addr_l=end_addr&0xffff;
    
}
#endif
/*******************************************************************************
*   Name: SPI0_DMA_config
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI1_DMA_Enable(UINT8 time_out,UINT8 reg)
{
    
    UINT8  dma_errdr=0xec;

    pSpi1_DmaRegs->dma_srcr=0x000f;
        
    pSpi1_DmaRegs->timeout_ctrl=((UINT16)time_out<<8)|dma_errdr;

    pSpi1_DmaRegs->dma_ctrl=reg;
    
}
/*******************************************************************************
*   Name: SPI0_Init
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI0_REG_Init(void)
{

    SPI0CKEN = 1;
    SPI0CON5 =0xff;
    SPI0CON2 =0x00;
    SPI0CON3 =0;
    SPI0CON4 =0X00;  

    SPI0_EDGE_SEL =1;
    /* IO配置: P05-->P02  MOSI MISO CLK CS */
    P0MOD |= BITn(5) + BITn(4)+BITn(3) + BITn(2);
    //P0SEL |= BITn(5) + BITn(4)+BITn(3) + BITn(2);
    
    //P0DIR |= BITn(4)+BITn(3) + BITn(2);
    //P0DIR &= ~ BITn(5);

    //中断标志使能
    SPI0RI_EN = 0;
    SPI0TI_EN = 0;
    SPI0_STOP_EI=1;

    SPI0RX_AUTO_CLR =0;
    SPI0TX_AUTO_CLR =0;

    DBG_SPI("\n SPI0CON1=%02x ",SPI0CON);
    DBG_SPI("\n SPI0CON2=%02x ",SPI0CON2);
    DBG_SPI("\n SPI0CON3=%02x ",SPI0CON3);
    DBG_SPI("\n SPI0CON4=%02x ",SPI0CON4);

}
/*******************************************************************************
*   Name: SPI0_DMA_Init
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI0_DMA_Init(void)
{
    SPI0_32K_EN =1;

    pSpi0_DmaRegs->dma_ctrl=0;
    pSpi0_DmaRegs->timeout_ctrl=0;
    pSpi0_DmaRegs->addr_h=0X3344;
    pSpi0_DmaRegs->start_addr_l=0X1122;
    pSpi0_DmaRegs->end_addr_l=0X2345;
    pSpi0_DmaRegs->dma_srcr=0x000f;
    
    
    DBG_SPI("\n 0dma_srcr=%04x ",pSpi0_DmaRegs->dma_srcr);
    DBG_SPI("\n 0dma_ctrl=%04x ",pSpi0_DmaRegs->dma_ctrl);
    DBG_SPI("\n 0dma_crc =%04x ",pSpi0_DmaRegs->dma_crc);
    DBG_SPI("\n 0dma_timeout=%04x ",pSpi0_DmaRegs->timeout_ctrl);
    DBG_SPI("\n 0start_addr_h=%04x ",pSpi0_DmaRegs->addr_h);
    DBG_SPI("\n 0start_addr_l=%04x ",pSpi0_DmaRegs->start_addr_l);
    DBG_SPI("\n 0end_addr_l=%04x ",pSpi0_DmaRegs->end_addr_l);
    
}
/*******************************************************************************
*   Name: SPI0_DMA_ClrStatus
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI0_DMA_ClrStatus(void)
{    
    pSpi0_DmaRegs->dma_srcr=0x000f;    
}
/*******************************************************************************
*   Name: SPI0_DMA_Wait_Done
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI0_DMA_Wait_Done(void)
{    
    while(pSpi0_DmaRegs->dma_ctrl & BIT_DMA_EN);    
}
/*******************************************************************************
*   Name: SPI0_DMA_SetAddr
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI0_DMA_SetAddr(UINT32 start_addr,UINT32 len,UINT16 mem_sel)
{    
    UINT16 start_addr_h,end_addr_h;
    UINT32 end_addr;
    
    end_addr        =start_addr+len-1;

    start_addr_h    =(start_addr>>16)&0x03;
    end_addr_h      =(end_addr  >>16)&0x03;

    mem_sel &= 0xC000;    
    
    pSpi0_DmaRegs->addr_h=mem_sel|(start_addr_h<<8)|end_addr_h;  
    
    pSpi0_DmaRegs->start_addr_l=start_addr&0xffff;
    pSpi0_DmaRegs->end_addr_l=end_addr&0xffff;
    
}

/*******************************************************************************
*   Name: SPI0_DMA_config
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void SPI0_DMA_Enable(UINT8 time_out,UINT8 reg)
{
    
    UINT8  dma_errdr=0xec;

    pSpi0_DmaRegs->dma_srcr=0x000f;
        
    pSpi0_DmaRegs->timeout_ctrl=((UINT16)time_out<<8)|dma_errdr;

    pSpi0_DmaRegs->dma_ctrl=reg;
    
}
/*******************************************************************************
*   Name: Test_DMA_MEM
*  Brief: MAIN CLK: 48M
*  Input:
* Output:
* Return:
*******************************************************************************/
void Test_DMA_MEM(UINT16 mem_sel)
{
    volatile UINT8  far *ucpAsmram;
    UINT16 i=0,j=0,wCRC=0;
    UINT8  reg=0,temp=10;
    UINT32 byte_num=0;    

    
    SPI0_REG_Init(); 
  /******************************************/
    I2C_STOP_EI = 1;
    DMA_END_EI  =0;

    if(mem_sel==DMA_MEM_PRAM)
    {
        ucpAsmram=PRAM2_PTR8(0);
    }
    else if(mem_sel==DMA_MEM_DRAM)
    {
        ucpAsmram =&DRAMBUF[0];
    }
    else 
    {
        mem_sel=DMA_MEM_ARAM;
        ucpAsmram =ARAM_START_ADDRESS;
    }
    
    AFEACKEN = 1;
    ARAM_SEL = 1;   /* ARAM_SEL: 1: Mcu write and read; 0: others */


    for (i = 0; i < 16; i++)
    {     
        *(ucpAsmram+i) = 0x10+i;            
    }
    /*
    for (i = 0; i < 0X500; i++)
    {
        if (i%16 == 0)
        {
            DBG_SPI("\n");
        }
        DBG_SPI("%02x ",*(ucpAsmram+i));

    }*/

    
    SPI0_DMA_Init();
    SPI0_DMA_SetAddr((UINT32)ucpAsmram,(UINT32)5,mem_sel);
    
    reg=SYSCON;
    SYSCON =reg|BITn(6)|BITn(5);  
    SYSCON =reg|BITn(6)|BITn(5);
    SYSCON =reg|BITn(6)|BITn(5);
    do
    {
        while(!BUS_SEL);
        DelayMs(2000);
    }while(!BUS_SEL);
    
    while(1)
    {

        CRCCCITT(ucpAsmram,6,&wCRC);

        DBG_SPI("\n1  wCRC=%04x ",wCRC);
        DBG_SPI("\n test dma write Mam");
        while(s_ucUartTxLen);


 #if 1       
        SPI0_DMA_Enable(0xff,(DMA_TIMEOUT_DIS|DMA_TRANS_ERR_EN|DMA_CRC_DIS|DMA_WD|
        DMA_CLR_RI_EN|DMA_CLR_TI_EN|DMA_EN));
        
        
        SPI0_DMA_Wait_Done();
  #if 1      
        byte_num=SPI0_Get_Byte_Num();
        DBG_SPI("\n byte_num=%lx ",byte_num);
        

        DelayUs(100);
        DBG_SPI("\n dma_srcr=%04x ",pSpi0_DmaRegs->dma_srcr);
        DBG_SPI("\n dma_ctrl=%04x ",pSpi0_DmaRegs->dma_ctrl);
        DBG_SPI("\n dma_crc =%04x ",pSpi0_DmaRegs->dma_crc);
        DBG_SPI("\n dma_timeout=%04x ",pSpi0_DmaRegs->timeout_ctrl);
        DBG_SPI("\n start_addr_h=%04x ",pSpi0_DmaRegs->addr_h);
        DBG_SPI("\n start_addr_l=%04x ",pSpi0_DmaRegs->start_addr_l);
        DBG_SPI("\n end_addr_l=%04x ",pSpi0_DmaRegs->end_addr_l);

        
        #if 1
        for (i = 0; i < 16; i++)
        {
            if (i%16 == 0)
            {
                DBG_SPI("\n");
            }
            DBG_SPI("%02x ",*(ucpAsmram+i));
            
            *(ucpAsmram+i)=0xA7;
        }
        #endif
        while(s_ucUartTxLen);

#endif
//#else        
        SPI0_DMA_Enable(0xff,(DMA_TIMEOUT_DIS|DMA_TRANS_ERR_DIS|DMA_CRC_DIS|DMA_RD|
        DMA_CLR_RI_EN|DMA_CLR_TI_EN|DMA_EN));
        DBG_SPI("\n test dma read Mam");
        
        SPI0_DMA_Wait_Done();

        while(s_ucUartTxLen);
#endif        
        DelayMs(1000);
    }
    
}


/*******************************************************************************
*   Name: Test_DMA_HS
*  Brief: SPI1-->SPO0
*  Input:
* Output:
* Return:
*******************************************************************************/
void Test_DMA_HS(void)
{
    volatile UINT8  far *ucpAsmram;
    UINT16 i=0,j=0,wCRC=0;
    UINT8  reg=0,temp=10;
    UINT32 byte_num=0;
    
    SPI1_Init();
    SPI0_REG_Init();

  
  /******************************************/
    DMA_END_EI  =0;
    I2C_STOP_EI = 1;  

    ucpAsmram =ARAM_START_ADDRESS;
    
    AFEACKEN = 1;
    ARAM_SEL = 1;   /* ARAM_SEL: 1: Mcu write and read; 0: others */
    for (i = 0; i < NUM_B; i++)
    {
     
        DRAMBUF[i] = (32+i)&0XFF;
        *(ucpAsmram+i) = (UINT8)((0xC0+i)&0XFF);
    }

    for (i = 0; i < NUM_B; i++)
    {     
        //*(ucpAsmram+i) = (~i )&0xff;
        PRAM2_ADDR8(i)=i&0xff+0x20;     
    }
    
    for (i = 0; i < NUM_B; i++)
    {
        if (i%16 == 0)
        {
            DBG_SPI("\n");
        }
        DBG_SPI("DRAM=%02x ",DRAMBUF[i]);//PRAM2_ADDR8(i));

    }

    
    SPI0_DMA_Init();
    
    SPI0_DMA_SetAddr((UINT32)ucpAsmram,(UINT32)(NUM_B),DMA_MEM_ARAM);
    
    reg=SYSCON;
    SYSCON =reg|BITn(6)|BITn(5);  
    SYSCON =reg|BITn(6)|BITn(5);
    SYSCON =reg|BITn(6)|BITn(5);
    do
    {
        while(!BUS_SEL);
        DelayMs(2000);
    }while(!BUS_SEL);

    
    while(1)
    {

     //   CRCCCITT(ucpAsmram,6,&wCRC);

     //   DBG_SPI("\n1  wCRC=%04x ",wCRC);
        DBG_SPI("\n test dma write aram");
        while(s_ucUartTxLen);


 #if 1       
        SPI0_DMA_Enable(0xff,(DMA_TIMEOUT_DIS|DMA_TRANS_ERR_EN|DMA_CRC_DIS|DMA_WD|
        DMA_CLR_RI_EN|DMA_CLR_TI_EN|DMA_EN));

        
        SPI1_Write(DRAMBUF,NUM_B);
        //SPI1_Write(PRAM2_ADDR8(0),NUM_B);
        
        SPI0_DMA_Wait_Done();
  #if 1      
        byte_num=SPI0_Get_Byte_Num();
        DBG_SPI("\n1 byte_num=%lx ",byte_num);
        SPI0_CLR_NUM =1;
        byte_num=SPI0_Get_Byte_Num();
        DBG_SPI("\n2 byte_num=%lx,%02x ",byte_num,SPI0_CLR_NUM);    

        DelayUs(100);
        DBG_SPI("\n dma_srcr=%04x ",pSpi0_DmaRegs->dma_srcr);
        DBG_SPI("\n dma_ctrl=%04x ",pSpi0_DmaRegs->dma_ctrl);
        DBG_SPI("\n dma_crc =%04x ",pSpi0_DmaRegs->dma_crc);
        DBG_SPI("\n dma_timeout=%04x ",pSpi0_DmaRegs->timeout_ctrl);
        DBG_SPI("\n start_addr_h=%04x ",pSpi0_DmaRegs->addr_h);
        DBG_SPI("\n start_addr_l=%04x ",pSpi0_DmaRegs->start_addr_l);
        DBG_SPI("\n end_addr_l=%04x ",pSpi0_DmaRegs->end_addr_l);

        
        #if 1
        for (i = 0; i < NUM_B; i++)
        {
            if (DRAMBUF[i] != *(ucpAsmram+i))
            //if (PRAM2_ADDR8(i) != *(ucpAsmram+i))    
            {
                DBG_SPI("\ntest write err\n");
                HOLD;
            }
            if (i%16 == 0)
            {
           //     DBG_SPI("\n");
            }
           // DBG_SPI("%02x ",*(ucpAsmram+i));
            
        //    *(ucpAsmram+i)=0xA7;
        }
        #endif
        while(s_ucUartTxLen);

#endif
//#else        
        SPI0_DMA_Enable(0xff,(DMA_TIMEOUT_DIS|DMA_TRANS_ERR_DIS|DMA_CRC_DIS|DMA_RD|
        DMA_CLR_RI_EN|DMA_CLR_TI_EN|DMA_EN));
        DBG_SPI("\n test dma read aram");
        SPI1_Read(&DRAMBUF[NUM_B],NUM_B);
        //SPI1_Read(PRAM2_ADDR8(NUM_B),NUM_B);
        SPI0_DMA_Wait_Done();

#if 1
        for (i = 0; i < NUM_B; i++)
        {
            //if (DRAMBUF[i] != DRAMBUF[NUM_B+i])
            if (*(ucpAsmram+i) != DRAMBUF[NUM_B+i]) 
            //if (*(ucpAsmram+i) != PRAM2_ADDR8(NUM_B+i))
            {
                DBG_SPI("\ntest read err=%lx\n",i);
                DBG_SPI("DRAM=%02x,ARAM=%02x ",DRAMBUF[128+i],*(ucpAsmram+i));
                HOLD;
            }
            if (i%16 == 0)
            {
            //    DBG_SPI("\n");
            }
          //  DBG_SPI("%02x ",DRAMBUF[128+i]);            
      
        }
#endif        
        while(s_ucUartTxLen);
        
#endif        
     //   DelayMs(1000);
    }
    
}
/*******************************************************************************
*   Name: Test_DMA_PRAM
*  Brief: SPI1-->SPO0
*  Input:
* Output:
* Return:
*******************************************************************************/
void Test_DMA_PRAM(void)
{
    volatile UINT8  far *ucpAsmram;
    UINT16 i=0,j=0,wCRC=0;
    UINT8  reg=0,temp=10;
    UINT32 byte_num=0;
    
    SPI1_Init();
    SPI0_REG_Init();

  
  /******************************************/
    DMA_END_EI  =0;
    I2C_STOP_EI = 1;  

    ucpAsmram =ARAM_START_ADDRESS;
    
    AFEACKEN = 1;
    ARAM_SEL = 1;   /* ARAM_SEL: 1: Mcu write and read; 0: others */
    for (i = 0; i < NUM_B; i++)
    {
     
        DRAMBUF[i] = (32+i)&0XFF;
        *(ucpAsmram+i) = (UINT8)((0xC0+i)&0XFF);
    }

    for (i = 0; i < NUM_B; i++)
    {     
        //*(ucpAsmram+i) = (~i )&0xff;
        PRAM2_ADDR8(i)=(i+0X60)&0xff;     
    }
    
    for (i = 0; i < NUM_B; i++)
    {
        if (i%16 == 0)
        {
            DBG_SPI("\n");
        }
        DBG_SPI("PRAM=%02x ",PRAM2_ADDR8(i));

    }

    
    SPI0_DMA_Init();
    
    SPI0_DMA_SetAddr((UINT32)ucpAsmram,(UINT32)(NUM_B),DMA_MEM_ARAM);


    reg=SYSCON;
    SYSCON =reg|BITn(6)|BITn(5);  
    SYSCON =reg|BITn(6)|BITn(5);
    SYSCON =reg|BITn(6)|BITn(5);
    do
    {
        while(!BUS_SEL);
        DelayMs(2000);
    }while(!BUS_SEL);

    
    while(1)
    {

     //   CRCCCITT(ucpAsmram,6,&wCRC);

     //   DBG_SPI("\n1  wCRC=%04x ",wCRC);
        DBG_SPI("\n test dma write aram");
        while(s_ucUartTxLen);


 #if 1       
        SPI0_DMA_Enable(0xff,(DMA_TIMEOUT_DIS|DMA_TRANS_ERR_EN|DMA_CRC_DIS|DMA_WD|
        DMA_CLR_RI_EN|DMA_CLR_TI_EN|DMA_EN));

        
        //SPI1_Write(DRAMBUF,NUM_B);
        SPI1_Write(PRAM2_PTR8(0),NUM_B);
        
        SPI0_DMA_Wait_Done();
  #if 1      
        byte_num=SPI0_Get_Byte_Num();
        DBG_SPI("\n1 byte_num=%lx ",byte_num);
        SPI0_CLR_NUM =1;
        byte_num=SPI0_Get_Byte_Num();
        DBG_SPI("\n2 byte_num=%lx,%02x ",byte_num,SPI0_CLR_NUM);    

        DelayUs(100);
        DBG_SPI("\n dma_srcr=%04x ",pSpi0_DmaRegs->dma_srcr);
        DBG_SPI("\n dma_ctrl=%04x ",pSpi0_DmaRegs->dma_ctrl);
        DBG_SPI("\n dma_crc =%04x ",pSpi0_DmaRegs->dma_crc);
        DBG_SPI("\n dma_timeout=%04x ",pSpi0_DmaRegs->timeout_ctrl);
        DBG_SPI("\n start_addr_h=%04x ",pSpi0_DmaRegs->addr_h);
        DBG_SPI("\n start_addr_l=%04x ",pSpi0_DmaRegs->start_addr_l);
        DBG_SPI("\n end_addr_l=%04x ",pSpi0_DmaRegs->end_addr_l);

        
        #if 1
        for (i = 0; i < NUM_B; i++)
        {
            //if (DRAMBUF[i] != *(ucpAsmram+i))
            if (PRAM2_ADDR8(i) != *(ucpAsmram+i))    
            {
                DBG_SPI("\ntest write err\n");
                HOLD;
            }
            if (i%16 == 0)
            {
           //     DBG_SPI("\n");
            }
           // DBG_SPI("%02x ",*(ucpAsmram+i));
            
        //    *(ucpAsmram+i)=0xA7;
        }
        #endif
        while(s_ucUartTxLen);

#endif
//#else        
        SPI0_DMA_Enable(0xff,(DMA_TIMEOUT_DIS|DMA_TRANS_ERR_DIS|DMA_CRC_DIS|DMA_RD|
        DMA_CLR_RI_EN|DMA_CLR_TI_EN|DMA_EN));
        DBG_SPI("\n test dma read aram");
        //SPI1_Read(&DRAMBUF[NUM_B],NUM_B);
        SPI1_Read(PRAM2_PTR8(NUM_B),NUM_B);
        SPI0_DMA_Wait_Done();

#if 1
        for (i = 0; i < NUM_B; i++)
        {
            //if (DRAMBUF[i] != DRAMBUF[NUM_B+i])
            //if (*(ucpAsmram+i) != DRAMBUF[NUM_B+i]) 
            if (*(ucpAsmram+i) != PRAM2_ADDR8(NUM_B+i))
            {
                DBG_SPI("\ntest read err=%lx\n",i);
                DBG_SPI("DRAM=%02x,ARAM=%02x ",DRAMBUF[NUM_B+i],*(ucpAsmram+i));
                HOLD;
            }
            if (i%16 == 0)
            {
            //    DBG_SPI("\n");
            }
          //  DBG_SPI("%02x ",DRAMBUF[128+i]);            
      
        }
#endif        
        while(s_ucUartTxLen);
        
#endif        
     //   DelayMs(1000);
    }
    
}
#if SPI1_DMA_EN
/*******************************************************************************
*   Name: Test_DMA_DMA
*  Brief: SPI1 DMA(只能访问DRAM)-->SPI0 DMA(ARAM)
*  Input:
* Output:
* Return:
*******************************************************************************/
void Test_DMA_DMA(void)
{
    volatile UINT8  far *ucpAsmram;
    UINT16 i=0,j=0,wCRC=0;
    UINT8  reg=0,temp=10;
    UINT32 byte_num=0;
    
    SPI1_Init();
    SPI0_REG_Init();

  
  /******************************************/
    DMA_END_EI  =0;
    I2C_STOP_EI = 1;  

    ucpAsmram =ARAM_START_ADDRESS;
    
    AFEACKEN = 1;
    ARAM_SEL = 1;   /* ARAM_SEL: 1: Mcu write and read; 0: others */
    for (i = 0; i < NUM_B; i++)
    {
     
        DRAMBUF[i] = (32+i)&0XFF;
        *(ucpAsmram+i) = (UINT8)((0x77+i)&0XFF);
    }

    
    for (i = 0; i < NUM_B; i++)
    {
        if (i%16 == 0)
        {
            DBG_SPI("\n");
        }
        //DBG_SPI("DRAM=%02x ",DRAMBUF[i]);//PRAM2_ADDR8(i));
        DBG_SPI("ARAM=%02x ",*(ucpAsmram+i) );

    }

    SPI1_DMA_Init();
    SPI0_DMA_Init();
    
    SPI0_DMA_SetAddr((UINT32)ucpAsmram,(UINT32)(NUM_B),DMA_MEM_ARAM);
    


    reg=SYSCON;
    SYSCON =reg|BITn(6)|BITn(5);  
    SYSCON =reg|BITn(6)|BITn(5);
    SYSCON =reg|BITn(6)|BITn(5);
    SYSCON =reg|BITn(6)|BITn(5);
    
    do
    {
        while(!BUS_SEL);
        DelayMs(2000);
    }while(!BUS_SEL);

    
    while(1)
    {

     //   CRCCCITT(ucpAsmram,6,&wCRC);

     //   DBG_SPI("\n1  wCRC=%04x ",wCRC);
        DBG_SPI("\n test dma write aram");
        while(s_ucUartTxLen);


 #if 1 

        SPI0_DMA_Enable(0xff,(DMA_TIMEOUT_DIS|DMA_TRANS_ERR_EN|DMA_CRC_DIS|DMA_WD|
        DMA_CLR_RI_EN|DMA_CLR_TI_EN|DMA_EN));


        
        SPI1_DMA_SetAddr((UINT32)DRAMBUF,(UINT32)(NUM_B),DMA_MEM_DRAM);
        
        DBG_SPI("\n1 start_addr_h=%04x ",pSpi1_DmaRegs->addr_h);
        DBG_SPI("\n1 start_addr_l=%04x ",pSpi1_DmaRegs->start_addr_l);
        DBG_SPI("\n1 end_addr_l=%04x ",pSpi1_DmaRegs->end_addr_l);
        SPI1SLVELO=0;
        SPI1_DMA_Enable(0xff,(DMA_TIMEOUT_DIS|DMA_TRANS_ERR_DIS|DMA_CRC_DIS|DMA_RD|
        DMA_CLR_RI_EN|DMA_CLR_TI_EN|DMA_EN));
        SPI1SLVELO=1;
        DBG_SPI("\n dma_srcr=%04x ",pSpi1_DmaRegs->dma_srcr);
        DBG_SPI("\n dma_ctrl=%04x ",pSpi1_DmaRegs->dma_ctrl);
        DBG_SPI("\n DMA_TIMEOUT_EN");
        while(s_ucUartTxLen);
        
        SPI0_DMA_Wait_Done();
  #if 1      
        byte_num=SPI0_Get_Byte_Num();
        DBG_SPI("\n1 byte_num=%lx ",byte_num);
        SPI0_CLR_NUM =1;
        byte_num=SPI0_Get_Byte_Num();
        DBG_SPI("\n2 byte_num=%lx,%02x ",byte_num,SPI0_CLR_NUM);    

        DelayUs(100);
        DBG_SPI("\n dma_srcr=%04x ",pSpi0_DmaRegs->dma_srcr);
        DBG_SPI("\n dma_ctrl=%04x ",pSpi0_DmaRegs->dma_ctrl);
        DBG_SPI("\n dma_crc =%04x ",pSpi0_DmaRegs->dma_crc);
        DBG_SPI("\n dma_timeout=%04x ",pSpi0_DmaRegs->timeout_ctrl);
        DBG_SPI("\n start_addr_h=%04x ",pSpi0_DmaRegs->addr_h);
        DBG_SPI("\n start_addr_l=%04x ",pSpi0_DmaRegs->start_addr_l);
        DBG_SPI("\n end_addr_l=%04x ",pSpi0_DmaRegs->end_addr_l);

        
        #if 1
        for (i = 0; i < NUM_B; i++)
        {
            if (DRAMBUF[i] != *(ucpAsmram+i)) 
            {
                DBG_SPI("\ntest write err=%lx\n",i);
                HOLD;
            }
            if (i%16 == 0)
            {
            //    DBG_SPI("\n");
            }
            //DBG_SPI("%02x ",*(ucpAsmram+i));
            
        
        }
        #endif
        while(s_ucUartTxLen);

#endif
//#else 
        //    SPI1RI = 0;
        //    SPI1TI = 0;

        SPI0_DMA_Enable(0xff,(DMA_TIMEOUT_DIS|DMA_TRANS_ERR_DIS|DMA_CRC_DIS|DMA_RD|
        DMA_CLR_RI_EN|DMA_CLR_TI_EN|DMA_EN));
        DBG_SPI("\n test dma read aram");


        SPI1_DMA_SetAddr((UINT32)&DRAMBUF[NUM_B],(UINT32)(NUM_B),DMA_MEM_DRAM);
        
        DBG_SPI("\n1 start_addr_h=%04x ",pSpi1_DmaRegs->addr_h);
        DBG_SPI("\n1 start_addr_l=%04x ",pSpi1_DmaRegs->start_addr_l);
        DBG_SPI("\n1 end_addr_l=%04x ",pSpi1_DmaRegs->end_addr_l);
         while(s_ucUartTxLen);
        SPI1SLVELO=0;
        SPI1_DMA_Enable(0xff,(DMA_TIMEOUT_DIS|DMA_TRANS_ERR_DIS|DMA_CRC_DIS|DMA_WD|
        DMA_CLR_RI_EN|DMA_CLR_TI_EN|DMA_EN));
        SPI1SLVELO=1;
        SPI0_DMA_Wait_Done();

#if 1
        for (i = 0; i < NUM_B; i++)
        {
            //if (DRAMBUF[i] != DRAMBUF[NUM_B+i])
            if (*(ucpAsmram+i) != DRAMBUF[NUM_B+i])            
            {
                DBG_SPI("\ntest read err=%lx\n",i);
                DBG_SPI("DRAM=%02x,ARAM=%02x ",DRAMBUF[NUM_B+i],*(ucpAsmram+i));
                HOLD;
            }
            if (i%16 == 0)
            {
            //    DBG_SPI("\n");
            }
            //DBG_SPI("%02x ",DRAMBUF[NUM_B+i]); 
      
        }
#endif        
        while(s_ucUartTxLen);
        
        
#endif        
    
    for (i = 0; i < NUM_B; i++)
    {
     
        DRAMBUF[NUM_B+i] = 0X55;
        *(ucpAsmram+i) = 0XAA;
    }
    }
    
}
#endif
/*******************************************************************************
*   Name: Test_DMA
*  Brief: 
*  Input:
* Output:
* Return:
*******************************************************************************/
void Test_DMA(void)
{

    Test_DMA_MEM(DMA_MEM_ARAM);
    //Test_DMA_PRAM();
    //Test_DMA_HS();
    Test_DMA_DMA();
}







void SPI0_DMA_STOP_ISR(void) interrupt 8
{
    if(SPI0_STOP)
    {
        SPI0_STOP_CLR=1;
        SPI0CON5    =0XFF;
        DBG_SPI("\n CS INT_srcr=%04x ",pSpi0_DmaRegs->dma_srcr);
        
     //   SPI0_DMA_ClrStatus();
        
    }
    if(DMA_END_FLAG)
    {
        DMA_END_CLR =1;
        DBG_SPI("\n DMA INT_srcr=%04x ",pSpi0_DmaRegs->dma_srcr);
       
    //    SPI0_DMA_ClrStatus();
        
    }
    
}




#endif//if _SPI0_DMA_EN





