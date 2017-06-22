/*******************************************************************************
* Copyright (C) 2016-2018, FocalTech Systems (R)��All Rights Reserved.
*
* File Name: Test_spi0_dma.h
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
#ifndef _TEST_SPI0_DMA_H__
#define _TEST_SPI0_DMA_H__

/*******************************************************************************
* Included files
*******************************************************************************/
#include "CfgGlobal.h"

/*******************************************************************************
* Global constant and macro definitions using #define
*******************************************************************************/
#if _TEST_SPI0_DMA_EN
/*******************************************************************************
* Global structures, unions and enumerations using typedef
*******************************************************************************/

/*******************************************************************************
* Global variable extern declarations
*******************************************************************************/
/*DMA_SRCR REG*/
#define BIT_DMA_NOT_FINISH_FLAG      BITn(11) /*cs����ʱ��dmaû���������DMA���ݰ����ݴ��䣬����1,��Ҫ������0*/
#define BIT_DMA_TIMEOUT_FLAG         BITn(10) /*dma���ݴ���ʱ����timeout,����1,��Ҫ������0.ֻ����ռ������ʱ,����������CRCʱ��*/
#define BIT_DMA_SCK_OVERTRANS_FLAG   BITn(9) /*DMA���ݰ����������host��������sck������1����Ҫ������0*/
#define BIT_DMA_CRC_ERROR_FLAG       BITn(8) /*������dmaдģʽʱ�������յ����ݼ������crc����յ���crc��һ��ʱ����1��������0*/

#define BIT_DMA_NOT_FINISH_CLR       BITn(3) /*д1��dma_not_finish_flag*/
#define BIT_DMA_TIMEOUT_CLR          BITn(2) /*д1��dma_timeout_flag*/
#define BIT_DMA_SCK_OVERTRANS_CLR    BITn(1)  /*д1��dma_sck_overtrans_flag*/
#define BIT_DMA_CRC_ERROR_CLR        BITn(0)  /*д1��dma_crc_error_flag*/
/*DMA_CTRL REG*/

#define BIT_DMA_TIMEOUT_EN           BITn(6)  /*timeout����ʹ��:0:disable;1:enable*/
#define BIT_DMA_TRANS_ERR_EN         BITn(5)  /*timerout��dma������host��������sck�Ƿ�Ҫ����dma_err_dat.0:������;1:����*/
#define BIT_DMA_CRC_EN               BITn(4)  /*dma����/���������Ƿ���Ҫ����/����crc. 0: no crc;1:crc*/
#define BIT_DMA_RW                   BITn(3)  /*dma��ram���ж���������д����.0:��;1:д*/
//#define BIT_DMA_RAM_SEL              (BITn(2)|BITn(1))  /*dma���ʵ�ramѡ��. 2'b00:aram;2'b01:dram;2'b1x:pram*/
#define BIT_DMA_EN                   BITn(0)  /*dma����ʹ��hw auto clr after exit dma. 0:disbale;1:enable*/


/*DMA_ERR_TIMEOUT_CTRL REG*/
#define BIT_DMA_TIMEOUT_NUM          BITn(8) /*dma timeoutʱ������,��32kʱ��Ϊ��λ*/
#define BIT_DMA_ERR_DAT              BITn(0) /*err byte:��timeout����dma���������host��������sck��spi0��host����error byte*/




/*DMA_CTRL REG*/
#define DMA_TIMEOUT_EN           ((UINT16)1<<6)  
#define DMA_TIMEOUT_DIS          ((UINT16)0<<6)  

#define DMA_TRANS_ERR_EN         ((UINT16)1<<5)  /*timerout��dma������host��������sck�Ƿ�Ҫ����dma_err_dat.0:������;1:����*/
#define DMA_TRANS_ERR_DIS        ((UINT16)0<<5)

#define DMA_CRC_EN               ((UINT16)1<<4)  /*dma����/���������Ƿ���Ҫ����/����crc. 0: no crc;1:crc*/
#define DMA_CRC_DIS              ((UINT16)0<<4)

#define DMA_WD                   ((UINT16)1<<3)  /*dma��ram���ж���������д����.0:��;1:д*/
#define DMA_RD                   ((UINT16)0<<3) 

#define DMA_CLR_RI_EN            ((UINT16)1<<2)
#define DMA_CLR_RI_DIS           ((UINT16)0<<2)

#define DMA_CLR_TI_EN            ((UINT16)1<<1)
#define DMA_CLR_TI_DIS           ((UINT16)0<<1)

#define DMA_EN                   ((UINT16)1<<0)

/*DMA_ADDR_H REG*/
#define DMA_MEM_PRAM             ((UINT16)2<<14)/*dma���ʵ�ramѡ��. 2'b00:aram;2'b01:dram;2'b1x:pram*/
#define DMA_MEM_DRAM             ((UINT16)1<<14)/*dma���ʵ�ramѡ��. 2'b00:aram;2'b01:dram;2'b1x:pram*/
#define DMA_MEM_ARAM             ((UINT16)0<<14) /*dma���ʵ�ramѡ��. 2'b00:aram;2'b01:dram;2'b1x:pram*/

/*******************************************************************************
* Global function prototypes
*******************************************************************************/
void SPI0_DMA_Init(void);
void SPI0_DMA_ClrStatus(UINT8 *val);
void SPI0_DMA_Wait_Done(void);
void SPI0_DMA_SetAddr(UINT32 start_addr,UINT32 len,UINT16 mem_sel);// reentrant;
void SPI0_DMA_Enable(UINT8 time_out,UINT8 reg);
//void Test_DMA(void);
#else
#define SPI0_DMA_Init       /##/
#define SPI0_DMA_ClrStatus  /##/
#define SPI0_DMA_Wait_Done  /##/
#define SPI0_DMA_SetAddr    /##/
#define SPI0_DMA_Enable     /##/
#define Test_DMA            /##/

#endif

#endif