/*******************************************************************************
* Copyright (C) 2012-2014, FocalTech Systems (R)��All Rights Reserved.
*
* File Name: Drv_Cal.c
*
*    Author: xinkunpeng
*
*   Created: 2014-05-07
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
#include "Drv_Cal.h"
#include <string.h>


#if _TEST_CAL_EN
/*******************************************************************************
* 2.Private constant and macro definitions using #define
*******************************************************************************/
/* DMA unit default value */
#define DMA_LEN        1            //����Amove֮�������㷨��DmaLength 1~31
#define DMA_LENA       1            //Amove��DmaLength                 1~31
#define DMALENGTH     ((DMA_LENA << 5) + DMA_LEN)


/*******************************************************************************
* 3.Private enumerations, structures and unions using typedef
*******************************************************************************/

/*******************************************************************************
* 4.Static variables
*******************************************************************************/
//ST_CalRegisters * XRAM pCalRegs = (volatile ST_CalRegisters *)CAL_REG_BASE_ADDR;
ST_CalRegisters *pCalRegs = (volatile ST_CalRegisters *)CAL_REG_BASE_ADDR;

/*******************************************************************************
* 5.Global variable or extern global variabls/functions
*******************************************************************************/
BOOLEAN g_bCalResumeFlag;
BOOLEAN g_bAmoveResumeFlag;
/*******************************************************************************
* 6.Static function prototypes
*******************************************************************************/

/*******************************************************************************
*   Name: DrvCal_Init
*  Brief: Config DMALENGTH,TxNum/RxNum and CalMask/AmoveMask for Cal-modules
*  Input:TxNum,RxNum,CalMask,AmoveMask
* Output:
* Return: None
*******************************************************************************/
void DrvCal_Init(UINT8 ucTxNum, UINT8 ucRxNum, UINT16 usDmask, UINT16 usDmaskA)

{
    CALCKEN = 1;

    pCalRegs->rDmaLen = DMALENGTH;
    
    pCalRegs->rTxRxNum = (ucTxNum << 8) + ucRxNum;       //CAL����
    pCalRegs->rTxRxNumAm = (ucTxNum << 8)+ ucRxNum;      //Amove����

    /* ����ʱ�жϿɴ��Դ���� */
    pCalRegs->rDmask = usDmask;//CAL��Mask
    pCalRegs->rDmaskA = usDmaskA;//Amove��Mask

    CALCKEN = 0;
}

/*******************************************************************************
*   Name: DrvCal_Cmd
*  Brief: Set cmd to cal_module and wait for finishing.
*  Input: cmd
* Output:
* Return: None
*******************************************************************************/
void DrvCal_Cmd(UINT16 cmd)
{
    pCalRegs->rCmd &= 0xFF0F;                                  
    pCalRegs->rCmd |= (cmd|CAL_CMD_START);                       

    /* mcu hold ... cal run */
    while((pCalRegs->rInt&CAL_CMD_INT) != CAL_CMD_INT)           
    {
        //DBG_CAL("\nCALINT:%d ",pCalRegs->rInt); 
        
        /* if cal madule is resumed,start again */
        if((pCalRegs->rCmd&CAL_CMD_SUSPEND) == CAL_CMD_SUSPEND)  
        {    
            DBG_CAL("\n\rSUSPEND=%d\n",CAL_CMD_INT);
            pCalRegs->rCmd |= CAL_CMD_RESUME;                    
            g_bCalResumeFlag = TRUE;
        }
    }
    pCalRegs->rCmd |= CAL_CMD_INT_CLR;
    pCalRegs->rCmd  = 0;                                        
}

/*******************************************************************************
*   Name: DrvCal_MatrixProc
*  Brief: Let hardware do ADD/DEC/MOV matrix calculation.
*         ADD/DEC: pCalRegs->rOutAddr = (pCalRegs->rS1Addr +/- pCalRegs->rS2Addr)>>ucShift
*         MOV:     pCalRegs->rOutAddr = pCalRegs->rS1Addr
*  Input: ST_CalMcMatrixParam *pMcMP =>
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MatrixProc(ST_CalMcMatrixParam *pMcMP)
{
    CALCKEN = TRUE;
   
    g_bCalResumeFlag = FALSE;

    pCalRegs->rS1Addr  = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr  = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr = pMcMP->usDstAddr;
    pCalRegs->rSize    = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;
    pCalRegs->rPThd = pMcMP->ucShift;
    // run
    pCalRegs->rCmd  = ((pMcMP->ucCfg & 0x01)<<8);
    DrvCal_Cmd(pMcMP->usCalCmd);
       
    CALCKEN = FALSE;
}

/*******************************************************************************
*   Name: DrvCal_MatrixAmp
*  Brief: Let hardware do AMP matrix calculation.
*         AMP: pCalRegs->rOutAddr = (pCalRegs->rS1Addr * mul)>>ucShift
*  Input: ST_CalMcMatrixParam *pMcMP =>
*         UINT8 mul
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MatrixAmp(ST_CalMcMatrixParam *pMcMP, SINT16 mul)
{
    CALCKEN = TRUE;
    
    pCalRegs->rS1Addr  = pMcMP->usSrc1Addr;
    pCalRegs->rOutAddr = pMcMP->usDstAddr;    
    pCalRegs->rSize    = (pMcMP->ucTxLen << 8) + pMcMP->ucRxLen;
    pCalRegs->rPThd = pMcMP->ucShift;
    pCalRegs->rPar  = mul;

    // run
    pCalRegs->rCmd = 0;
    DrvCal_Cmd(pMcMP->usCalCmd);
   
    CALCKEN = FALSE;
}

/*******************************************************************************
*   Name: DrvCal_MatrixDiv
*  Brief: Let hardware do AMP matrix calculation.
*         AMP: pCalRegs->rOutAddr = pCalRegs->rS1Addr/div
*  Input: ST_CalMcMatrixParam *pMcMP =>
*         UINT8 div
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MatrixDiv(ST_CalMcMatrixParam *pMcMP, UINT8 div)
{
    CALCKEN = TRUE;
    
    pCalRegs->rS1Addr  = pMcMP->usSrc1Addr;
    pCalRegs->rOutAddr = pMcMP->usDstAddr;
	pCalRegs->rSize    = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;
    pCalRegs->rPar  = div;
    // run
    pCalRegs->rCmd = 0;
    DrvCal_Cmd(pMcMP->usCalCmd);
   
    CALCKEN = FALSE;
}

/*******************************************************************************
*   Name: DrvCal_MatrixMul
*  Brief: M1 : M*K
*         M2 : K*N
*         OUT: M*N
*         Out[i][j] = sum(S1[i][0...(K-1)]*S2[0...(k-1)[j]])
*  Input: ST_CalMcMatrixParam *pMcMP =>
*
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MatrixMul(ST_CalMcMatrixParam *pMcMP, UINT8 K_Num)
{
    //clk
    CALCKEN = TRUE;
    
    // Address
    pCalRegs->rS1Addr  = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr  = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr = pMcMP->usDstAddr;
	pCalRegs->rSize    = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;
    pCalRegs->rPThd   = pMcMP->ucShift;
    pCalRegs->rNThd   = K_Num;

    pCalRegs->rCmd  = 0;
    DrvCal_Cmd(pMcMP->usCalCmd);
   
    CALCKEN = FALSE;
}


/*******************************************************************************
*   Name: DrvCal_McPeak
*  Brief: Let hardware Search positive/negative peaks
*  Input: ST_CalMcPeakDetectParam *pMcPDP => MC Peak Detect Parameters Struct
*         ST_CalMcPeakInfo *pMcPI => MC Peak Information Struct
* Output: g_stFrmData.CalOut => Calculate Output of Global Frame Data
* Return: None
*******************************************************************************/
#define TX_START      0  //�����Ͻǿ�ʼ
void DrvCal_McPeak(ST_CalMcPeakDetectParam *pMcPDP, ST_CalMcPeakInfo *pMcPI)
{
    CALCKEN = 1;
    
    pCalRegs->rS1Addr = pMcPDP->usDiffAddr;
    pCalRegs->rS2Addr = pMcPDP->usNegOutAddr;//;
    pCalRegs->rOutAddr  = pMcPDP->usCalOutAddr;
    pCalRegs->rSize   = (pMcPDP->ucTxLen<<8) + pMcPDP->ucRxLen;
    pCalRegs->rPar    = (TX_START<<8) + pMcPDP->ucMaxPeakNum;

    pCalRegs->rPThd   = pMcPDP->usPosThd;
    pCalRegs->rNThd   = pMcPDP->usNegThd;
    pCalRegs->rAThd   = pMcPDP->usAreaThd;
    pCalRegs->rNaThd  = pMcPDP->usNegAreaThd;

    // cmd
    pCalRegs->rCmd = pMcPDP->ucSearchMod<<8;
    DrvCal_Cmd(OP_PKS);

    // Get results
    pMcPI->ucMcPosPeakNum = pCalRegs->rPsCnt;
    pMcPI->ucMcNegPeakNum = pCalRegs->rNsCnt;
    pMcPI->ucMcPosAreaChNum = pCalRegs->rPaCnt;
    pMcPI->ucMcNegAreaChNum = pCalRegs->rNaCnt;

    pMcPI->usMCminValue = pCalRegs->rMinV;
    pMcPI->usMCminPos   = pCalRegs->rMinPos;
    pMcPI->usMCmaxValue = pCalRegs->rMaxV;
    pMcPI->usMCmaxPos   = pCalRegs->rMaxPos;
    
    CALCKEN = 0;
}

/*******************************************************************************
*   Name: DrvCal_Mac
*  Brief: Let hardware do center of gravity calculation
*         pCalRegs->rS1Addr : Data to be computed
*         pCalRegs->rOutAddr  : Result output, sum(c_tx), sum(tx), sum(c_rx), sum(rx)
*  Input: ST_CalMcMacParam *pMcMParm => MC Mac Parameters Struct
* Output: g_stFrmData.CalOut => Calculate Output of Global Frame Data
* Return: None
*******************************************************************************/
void DrvCal_Mac(ST_CalMcMacParam *pMacParm)
{
    CALCKEN = 1;
    
    pCalRegs->rS1Addr = pMacParm->usDiffAddr;
    pCalRegs->rOutAddr  = pMacParm->usCalOutAddr;

    pCalRegs->rSize = (pMacParm->ucTxLen<<8) + pMacParm->ucRxLen;
    pCalRegs->rPar  = (pMacParm->ucTxStart<<8) + pMacParm->ucRxStart;
    pCalRegs->rPThd = pMacParm->ucNoise;

    pCalRegs->rCmd  = (pMacParm->ucMode & 0x03) << 8;

    DrvCal_Cmd(OP_MAC);
    
    CALCKEN = 0;
}

/*******************************************************************************
*   Name: DrvCalcID
*  Brief: Match IDs
*         pCalRegs->rS1Addr: Last ID Position
*         pCalRegs->rS2Addr: Current ID Position
*  Input: UINT16 LastIdPosAddr => Last ID Position Information
*         UINT16 CurrIdPosAddr => Current ID Position Information
*         UINT8 MaxPoint => Maximum Point Number supported
* Output: None
* Return: UINT16 IDs
*******************************************************************************/
void DrvCal_KmID(ST_CalKmIdParam *pKmIDParm)
{
    UINT8 i,j;
    UINT8 MaxPoint;

    MaxPoint = pKmIDParm->ucLastPoint;
    if (MaxPoint < pKmIDParm->ucCurrPoint)
    {
        MaxPoint = pKmIDParm->ucCurrPoint;
    }

    memset((UINT16*)pKmIDParm->usCalOut, 0xFF, 121<<1);

    //clk
    CALCKEN = 1;
   
    // 1.���� MDS ���������໥����
    pCalRegs->rS1Addr = pKmIDParm->usLastIdPosAddr;
    pCalRegs->rS2Addr = pKmIDParm->usCurrIdPosAddr;
    pCalRegs->rOutAddr = pKmIDParm->usCalOut;
    pCalRegs->rSize    = (pKmIDParm->ucLastPoint << 8) + pKmIDParm->ucCurrPoint;

    //cmd
    pCalRegs->rCmd = 0;
    DrvCal_Cmd(OP_MDS);

    for (i = 0; i < pKmIDParm->ucLastPoint; i++)
    {
        for (j = 0; j < pKmIDParm->ucCurrPoint; j++)
        {
            *(UINT16*)(pKmIDParm->usCalOut+(i*11 + j)*2) = ~(*(UINT16*)(pKmIDParm->usCalOut+(i*11 + j)*2));
        }
    }

    // 2.��cal ID��
    pCalRegs->rS1Addr = pKmIDParm->usCalOut;
    pCalRegs->rSize   = (MaxPoint<<8) + MaxPoint;

    //cmd
    pCalRegs->rCmd  = 0;
    DrvCal_Cmd(OP_KM);

    /* ����Ӳ��CalID ����Ľ�� */
    for (i = 0; i < 11; i++)
    {
        // Get ID
        if (i < 4)
        {
            *(UINT16*)(pKmIDParm->usCalOut + i*2) = ( pCalRegs->rID0 >> (4*i) ) % 0x10;
        }
        else if (i < 8)
        {
            *(UINT16*)(pKmIDParm->usCalOut + i*2) = ( pCalRegs->rID1 >> (4*(i-4)) ) % 0x10;
        }
        else
        {
            *(UINT16*)(pKmIDParm->usCalOut + i*2) = ( pCalRegs->rID2 >> (4*(i-8)) ) % 0x10;
        }
    }
          
    CALCKEN = 0;
  
}

/*******************************************************************************
*  Name: DrvCal_CRC16
*  Brief: 
*  1. if Area == 0 Calculat CRC for Dram
*  2. if Area == 1 CRC start Address is 0xFC:xxxx
*  3. if Area == 2 CRC start Address is 0xFD:xxxx
*  4. if Area == 3 CRC Start Address is 0xFE:xxxx
*  5. if Area == 4 CRC start Address is 0xFF:xxxx
*  6. usLen:0 ~ 70k
*  
*  for example:
*  1.if want to calculat CRC Result for the zone (0xFE:E800 ~ 0xFF:FFFF),
*  set Area = 3,usSrc1Addr = 0xE800, uslen = 35*1024(70k)
*  2.if want to calculat CRC Result for the zone (0xFF:0000 ~ 0xFF:FFFF),
*  set Area = 4,usSrc1Addr = 0x0000, uslen = 32*1024(64k)
*  3.if want to calculat CRC Result for the zone (0xFC:E800 ~ 0xFC:FFFF),
*  set Area = 1,usSrc1Addr = 0xE800, uslen = 3*1024(6k)
*
* Input:  usSrc1Addr,usLen,Area,type,AddrH
* Output: None
* Return: CRC16 result
*******************************************************************************/
UINT16 DrvCal_CRC16(UINT16 usSrc1Addr, UINT16 usLen, UINT8 Area, BOOLEAN type)
{
    UINT16 value;
    //clk
    CALCKEN = 1;
          
    //cfg
    pCalRegs->rS1Addr = usSrc1Addr;
    pCalRegs->rSize   = usLen;

    //����
    pCalRegs->rCmd = (UINT16)type << 10;     //set xorsum function

    switch(Area)
    {
        case CRC_SRC_DRAM:
            pCalRegs->rCmd |= (0x00<<8);
            pCalRegs->rInt &= ~(1<<8);
            break;

        case CRC_SRC_FC_ZONE:
            pCalRegs->rCmd |= (0x01<<8);
            pCalRegs->rInt &= ~(1<<8);
            break;
            
        case CRC_SRC_FD_ZONE:
            pCalRegs->rCmd |= (0x01<<8);
            pCalRegs->rInt |= (1<<8); 
            break;

        case CRC_SRC_FE_ZONE:
            pCalRegs->rCmd |= (0x03<<8);
            pCalRegs->rInt &= ~(1<<8); 
            break;
            
        case CRC_SRC_FF_ZONE:
            pCalRegs->rCmd |= (0x03<<8);
            pCalRegs->rInt |= (1<<8); 
            break;
        default :
            break;            
    }
    
    DrvCal_Cmd(OP_CRC);

    value = pCalRegs->rCrc;
      
    CALCKEN = 0;
    return value;
}


#if 0
/*******************************************************************************
*   Name: Test_CAL_MaxtixMul_PtoP1
*  Brief: M1 : M*N
*         M2 : 1*N
*         OUT: M*N
*         Out[i][j] = M2[1][j]*M1[i][j];
*  Input: ST_CalMcMatrixParam *pMcMP =>
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MatrixMul_PtoP1(ST_CalMcMatrixParam *pMcMP)
{
    //clk
    CALCKEN = TRUE;
          
    // Address
    pCalRegs->rS1Addr  = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr  = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr = pMcMP->usDstAddr;
    pCalRegs->rSize    = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;
    pCalRegs->rPThd   = pMcMP->ucShift;
    
    pCalRegs->rInt &=(~(1<<13));
    pCalRegs->rInt |= (pMcMP->ucCfg<<13);
    
    pCalRegs->rCmd &= (~CAL_CMD_MUL2_EN);    // 1*2����

    DrvCal_Cmd(pMcMP->usCalCmd);
          
    CALCKEN = FALSE;
}

/*******************************************************************************
*   Name: DrvCal_MatrixMul_PtoP2
*  Brief: M1 : M*N
*         M2 : M*N
*         OUT: M*N
*         Out[i][j] = M1[i][j]*M2[i][j];
*  Input: ST_CalMcMatrixParam *pMcMP =>
*
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MatrixMul_PtoP2(ST_CalMcMatrixParam *pMcMP)
{
    //clk
    CALCKEN = TRUE;
      
    // Address
    pCalRegs->rS1Addr  = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr  = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr = pMcMP->usDstAddr;
    pCalRegs->rSize    = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;
    pCalRegs->rPThd   = pMcMP->ucShift;

    pCalRegs->rCmd |= CAL_CMD_MUL2_EN;       // 2*2����

    DrvCal_Cmd(pMcMP->usCalCmd);
          
    CALCKEN = FALSE;
}
#endif
/*******************************************************************************
*  Name: DrvCal_MatrixMul2
*  Brief:
*         if CalReg->cmd.bit13 == 1:          // 2*2��������
*         {
*            S1 : M*N
*            S2 : M*N
*            OUT: M*N
*            Out[i][j] = S1[i][j]*S2[i][j];
*         }
*
*         if  CalReg->Cmd.bit13 == 0:         // 1*2��������
*         {
*           if CalReg->Int.bit13 == 1:        //���г˹̶���
*           {
*              S1 : M*N
*              S2 : 1*N
*              OUT: M*N
*              Out[i][j] = S2[1][j]*S1[i][j]
*           }
*           if CalReg->Int.bit13 == 0:        //���г˹̶���
*           {
*              S1 : M*N
*              S2 : M*1
*              OUT: M*N
*              Out[i][j] = S2[i][1]*S1[i][j]
*           }                        
*         }
*
*  Input: ST_CalMcMatrixParam *pMcMP =>
*  Output: None
*  Return: None
*******************************************************************************/
void DrvCal_MatrixMul2(ST_CalMcMatrixParam *pMcMP,BOOLEAN MulMode)
{
    //clk
    CALCKEN = TRUE;
      
    // Address
    pCalRegs->rS1Addr  = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr  = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr = pMcMP->usDstAddr;
    pCalRegs->rSize    = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;
    pCalRegs->rPThd   = pMcMP->ucShift;

    if(MulMode == MUL2_MODE_1)                   // 2*2����  
    {
        pCalRegs->rCmd |= CAL_CMD_MUL2_EN;       // 2*2��ʽ   
    }
    else if(MulMode == MUL2_MODE_0)              // 1*2���� 
    {
        pCalRegs->rInt &=(~(1<<13));
        pCalRegs->rInt |= (pMcMP->ucCfg<<13);    // ���г˻��߰��г�    
        pCalRegs->rCmd &= (~CAL_CMD_MUL2_EN);    // 1*2��ʽ            
    }

    DrvCal_Cmd(pMcMP->usCalCmd);
          
    CALCKEN = FALSE;
}

#if 0
/*******************************************************************************
*   Name: DrvCal_MatrixCompensation_1
*  Brief:
*  Input: ST_CalMcMatrixParam *pMcMP =>
*         UINT8 Thr: Threshold
*         UINT8 step: Step
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MatrixCompensation_1(ST_CalMcMatrixParam *pMcMP, UINT8 Thr, UINT8 step)
{
    // CLK
    CALCKEN = TRUE;

    // Address
    pCalRegs->rS1Addr = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr  = pMcMP->usDstAddr;

    // ���󳤶�����
    pCalRegs->rSize    = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;

    // com�¾�ģʽѡ��
    if(pMcMP->ucCfg == COM_CFG_0)
    {
        pCalRegs->rInt&=(~(1<<14));
    }
    else if(pMcMP->ucCfg == COM_CFG_1) 
    {
        pCalRegs->rInt|=(1<<14);
    }

    // Mode
    pCalRegs->rPThd = (COMPENSATION_MODE<<8);

    // Step and threshold
    pCalRegs->rNThd = ((Thr<<8)|(step));

    // ��������������
    pCalRegs->rPThd |= ((NO_COMPENSATION_TX_LENGTH<<4) | (NO_COMPENSATION_RX_LENGTH));
    pCalRegs->rPar  = ((NO_COMPENSATION_TX_START <<8) | (NO_COMPENSATION_RX_START));


    //����
    pCalRegs->rCmd  = 0;
    DrvCal_Cmd(pMcMP->usCalCmd);
      
    CALCKEN = FALSE;
}
/*******************************************************************************
*   Name: DrvCal_MatrixCompensation_2
*  Brief:
*  Input: ST_CalMcMatrixParam *pMcMP =>
*         UINT8 Thr: Threshold
*         UINT8 step: Step
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MatrixCompensation_2(ST_CalMcMatrixParam *pMcMP, UINT8 Thr, UINT8 mode)
{
    // CLK
    CALCKEN = TRUE;
      
    // Address
    pCalRegs->rS1Addr = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr  = pMcMP->usDstAddr;
    
    // ���󳤶�����
    pCalRegs->rSize    = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;

    // Mode
    pCalRegs->rPThd = (COMPENSATION_MODE<<8);

    // com�¾�ģʽѡ��
    if(pMcMP->ucCfg == COM_CFG_0)
    {
        pCalRegs->rInt&=(~(1<<14));
    }
    else if(pMcMP->ucCfg == COM_CFG_1) 
    {
        pCalRegs->rInt|=(1<<14);      
    }

    // Step and threshold
    pCalRegs->rNThd = Thr;

    // ��������������
    pCalRegs->rPThd |= ((NO_COMPENSATION_TX_LENGTH<<4) | (NO_COMPENSATION_RX_LENGTH));
    pCalRegs->rPar  = ((NO_COMPENSATION_TX_START <<8) | (NO_COMPENSATION_RX_START));

    pCalRegs->rCmd  = 0;
    // com��ģʽ�����м���ѡ��: 0: ���м���, 1: ���м���
    pCalRegs->rCmd |= (mode<<10);  
    //����
    DrvCal_Cmd(pMcMP->usCalCmd);
      
    CALCKEN = FALSE;
}
#endif
/*******************************************************************************
*  Name: DrvCal_MatrixCompensation
*  Brief: 1.�ɲ����㷨:������1������(diff)������ֵ��ֵ,������2(base)��Ӧλ�õ���
*           �ݼ��ϲ���ֵ�Ժ�ŵ���������Ӧλ��.��С����ֵ��ֵ��������2(base)��
*           Ӧλ�õ����ݼ�ȥ����ֵ�Ժ�ŵ���������Ӧλ��.����ֵ����
*
*         2.�²����㷨:������1������(rawdata)������ֵ��ֵ���򽫸����ݼ�ȥ����ֵ
*           �Ժ�ŵ����������С����ֵ��ֵ���򽫸����ݼ��ϲ���ֵ�Ժ�ŵ����
*           ��������ֵ����(���л���еĲ���ֵ��һ��������2Ϊ����ֵ)
*  Input: ST_CalMcMatrixParam *pMcMP =>
*         UINT8 Thr: Threshold
*         UINT8 step: Step
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MatrixCompensation(ST_CalMcMatrixParam *pMcMP, UINT8 Thr, UINT8 Cfg)
{
    // CLK
    CALCKEN = TRUE;
      
    // Address
    pCalRegs->rS1Addr = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr  = pMcMP->usDstAddr;
    
    // ���󳤶�����
    pCalRegs->rSize = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;

    pCalRegs->rCmd  = 0;
    // com�¾�ģʽѡ��
    if(pMcMP->ucCfg == COM_CFG_0)
    {
        pCalRegs->rInt&=(~CAL_FUNC_SEL);            //���㷨(���� diff ֵ���� base)

        // Step and threshold
        pCalRegs->rNThd = ((Thr<<8)|(Cfg));         //thr:������threshold;Cfg:����ֵ        
    }
    else if(pMcMP->ucCfg == COM_CFG_1) 
    {
        pCalRegs->rInt|= CAL_FUNC_SEL;              //���㷨(linefilter)

        // Step and threshold
        pCalRegs->rNThd = Thr;                      //��ֵ�ľ���ֵ
        
        // com��ģʽ�����м���ѡ��: 0: ���м���, 1: ���м���
        pCalRegs->rCmd |= (Cfg<<10);        
    }

    // Mode
    pCalRegs->rPThd = (COMPENSATION_MODE << 8); //0:ȫ������;1:�в���������    
    // ��������������
    pCalRegs->rPThd |= ((NO_COMPENSATION_TX_LENGTH<<4) | (NO_COMPENSATION_RX_LENGTH));
    pCalRegs->rPar  = ((NO_COMPENSATION_TX_START <<8) | (NO_COMPENSATION_RX_START));

    //����
    DrvCal_Cmd(pMcMP->usCalCmd);
      
    CALCKEN = FALSE;
}

/*******************************************************************************
*  Name: DrvCal_LDS
*  Brief: 
*         S1[M][N]  (S1��һ��Ԫ��Ϊ16bit(x,y)��x��y��Ϊ8bits)
*         S2[M][N]
*         OUT[i][j] = sqrt((S1[i][j].x-S2[i][j].x)^2+(S1[i][j].y-S2[i][j].y)^2)
*  Input: ST_CalMcMatrixParam *pMcMP
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_LDS(ST_CalMcMatrixParam *pMcMP)
{
    // Clk
    CALCKEN = TRUE;
      
    // Address
    pCalRegs->rS1Addr = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr  = pMcMP->usDstAddr;
    pCalRegs->rSize    = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;

    //����
    pCalRegs->rCmd  = 0;
    DrvCal_Cmd(pMcMP->usCalCmd);
      
    CALCKEN = FALSE;
}

/*******************************************************************************
*   Name: DrvCal_MDS
*  Brief: ���ڲ������������ľ���point to point
*         S1[M]  (S1��һ��Ԫ��Ϊ16bit(x,y)��x��y��Ϊ8bits)
*         S2[N]
*         OUT[i][j] = sqrt((S1[i].x-S2[j].x)^2+(S1[i].y-S2[j].y)^2)
*  Input: ST_CalMcMatrixParam *pMcMP =>
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MDS(ST_CalMcMatrixParam *pMcMP)
{
    // Clk
    CALCKEN = TRUE;
      
    // Address
    pCalRegs->rS1Addr = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr  = pMcMP->usDstAddr;
	pCalRegs->rSize    = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;

    //����
    pCalRegs->rCmd  = 0;
    DrvCal_Cmd(pMcMP->usCalCmd);
      
    CALCKEN = FALSE;
}

/*******************************************************************************
*   Name: DrvCal_Noise
*  Brief:
*         1.һ������ ��ֵ = ���ֵ - ��Сֵ
*         2.�����еĲ�ֵ�������ֵ max = max(��ֵ)
*  Input: UINT16 usSrc1Addr
*         UINT8  Tx
*         UINT8  Rx
* Output: None
* Return: Get Max of value,which is sub value of max-min in the same column
*******************************************************************************/
UINT16 DrvCal_Noise(UINT16 usSrc1Addr,UINT8 Tx_Len, UINT8 Rx_Len)
{
    UINT16 Noise;
    //clk
    CALCKEN = TRUE;

    // Cfg
    pCalRegs->rS1Addr = usSrc1Addr;
    pCalRegs->rSize    = (Tx_Len<<8) + Rx_Len;

    pCalRegs->rCmd  = 0;
    DrvCal_Cmd(OP_CHK);

    Noise = pCalRegs->rChk;
      
    CALCKEN = FALSE;
    return Noise;
}

/*******************************************************************************
*   Name: DrvCal_MatrixAMov
*  Brief: 
*    1.Amov��Ƕ��CAL����,������Amovʱ�ɴ�ϵ�ǰCAL�������AMov����
*    2.�������������ж��а���Aram��rawdata��dram
* input : 
* Return: None
*******************************************************************************/
void DrvCal_MatrixAMov(ST_CalMatrixAMov *pMcMP)
{
    UINT8 CALCKEN_BAK = CALCKEN;
	//UINT16 ucCmd = pCalRegs->rCmd;  //ĿǰCAL�Ѹ������������ñ���
    CALCKEN = 1;//clk
    
	//pCalRegs->rCmd &= 0x7F0F;      
    pCalRegs->rSizeAm = (pMcMP->ucS1TxLen << 8) + pMcMP->ucS1RxLen;  //  21 TxRxLen
    pCalRegs->rS1AddrA = pMcMP->usSrc1Addr; // SrcAddr
    pCalRegs->rOutAddrA = pMcMP->usDstAddr;   // DstAddr
    pCalRegs->rInt &= 0xFF00;
    pCalRegs->rInt |= (0<<3);//��֧�ֺϲ�aMove Mode Select 03
    pCalRegs->rInt |= CAL_CMD_AMOV_EN;//����aMove  04
    pCalRegs->rInt |= CAL_CMD_AMOV_START;//aMoveStart 0
     /* mcu hold ... cal run */
    while ((pCalRegs->rInt&CAL_CMD_AMOV_INT) != CAL_CMD_AMOV_INT)
    {
        DBG_CAL("\nwaiting...int");
        /* if cal madule is resumed,start again */
        if ((pCalRegs->rInt&CAL_CMD_AMOV_SUSPEND) == CAL_CMD_AMOV_SUSPEND)
        {
            pCalRegs->rInt |= CAL_CMD_AMOV_RESUME;
            g_bAmoveResumeFlag = TRUE;
        }
    }    
    pCalRegs->rInt |= CAL_CMD_AMOV_INT_CLR;
    pCalRegs->rInt &= 0xFF00;
    
    //�п�����ASM�ж��е�����AMOVE,Ȼ��ASM�жϴ��������CAL�㷨���˳�Amove�㷨ʱ,����ص���
    //CALCKEN���򱻴�ϵ�CAL�㷨��û��ִ����ȥ������ϵͳ����.��ˣ���Ҫ�ڵ���AMOVE��ʱ�򣬰�
    //����֮ǰ��CALCKEN�����������˳���ʱ��ָ���ԭ��.
	//pCalRegs->rCmd = ucCmd;
    CALCKEN = CALCKEN_BAK;	   
}

/*******************************************************************************
*  Name: DrvCal_MatrixAvg
*  Brief: 
*  1.���а�����ȡԴ�����ƽ��ֵ��Src1Addr��ŵ���Դ���ݾ���Src2Addr��ŵ��Ǹ���
*    ����е���ֵ(���л���ж����Լ���������ֵ)���������OutAddr�С�
*  2.usAvgMode = 0����Դ���ݳ�����ֵ,���ø��л���е���ֵ����Դ���ݲ������
*  3.usAvgMode = 1����Դ���ݳ�����ֵ,�򽫸����ݶ�����������л����ʣ���ƽ��ֵ
*  input:   
*    ucCfg: 0: ���������з�����;  1:���������޷�����
*    ucLineCfg: 0: ���м���; 1: ���м���
*    ucDataType: 0: ԭʼ����; 1: �����������ֵ 
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MatrixAvg(ST_CalMcMatrixParam *pMcMP,UINT8 usAvgMode)
{
    //clk
    CALCKEN = TRUE;
             
    // Address
    pCalRegs->rS1Addr   = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr   = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr  = pMcMP->usDstAddr;
    pCalRegs->rSize     = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen; // 6

    pCalRegs->rInt &=(~(1<<14));
    pCalRegs->rInt |=(usAvgMode<<14);    
    pCalRegs->rCmd  = (pMcMP->ucLineCfg<<10)|(pMcMP->ucDataType<<9)|(pMcMP->ucCfg<<8);
    pCalRegs->rCmd |= CAL_CMD_MOD_EXTEND;
    DrvCal_Cmd(pMcMP->usCalCmd);
      
    CALCKEN = FALSE;
}

/*******************************************************************************
*  Name: DrvCal_MatrixMaxMin
*  Brief: 
*  input:   
*        ucCfg: 0: ���������з�����;  1:���������޷�����
*    ucLineCfg: 0: ���м���; 1: ���м���
*   ucDataType: 0: ԭʼ����; 1: �����������ֵ 
* Output: None
* Return: None
*******************************************************************************/
void DrvCal_MatrixMaxMin(ST_CalMcMatrixParam *pMcMP)
{
    //clk
    CALCKEN = TRUE;
            
    // Address
    pCalRegs->rS1Addr   = pMcMP->usSrc1Addr;
    pCalRegs->rOutAddr  = pMcMP->usDstAddr;  
    pCalRegs->rSize   = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;
    pCalRegs->rCmd= (pMcMP->ucLineCfg<<10)|(pMcMP->ucDataType<<9)|(pMcMP->ucCfg<<8); //00   
    pCalRegs->rCmd |= CAL_CMD_MOD_EXTEND;//
    DrvCal_Cmd(pMcMP->usCalCmd);
      
    CALCKEN = FALSE;
}

/*******************************************************************************
*   Name: DrvCal_MatrixCount
*  Brief:
*     IN: M*N����  
*    OUT: ����[min max]֮��ľ���Ԫ�ظ���
* Return:
*******************************************************************************/
UINT16 DrvCal_MatrixCount(ST_CalMcMatrixParam *pMcMP, SINT16 ssMin,SINT16 ssMax)
{
    UINT16 usNum = 0;
    //clk
	CALCKEN = TRUE;
      
    // Address
    pCalRegs->rS1Addr = pMcMP->usSrc1Addr;   
    pCalRegs->rSize   = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;
    pCalRegs->rPThd     = ssMax;
    pCalRegs->rNThd     = ssMin;

    pCalRegs->rCmd = 0; 
    pCalRegs->rCmd |= CAL_CMD_MOD_EXTEND;
    DrvCal_Cmd(pMcMP->usCalCmd);

    usNum = pCalRegs->rCntNum;
          
	CALCKEN = FALSE;
    return usNum;
}

/*******************************************************************************
*   Name: DrvCal_SubConst
*  Brief:
*     IN: M*N  1*N
*    OUT: M*N
* input : usCfgLineCol: 0: ���м���;  1: ���м���
* Return: None
*******************************************************************************/
void DrvCal_MatrixSubConst(ST_CalMcMatrixParam *pMcMP)
{
    //clk
    CALCKEN = TRUE;
      
    // Address
    pCalRegs->rS1Addr = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr = pMcMP->usDstAddr;   
    pCalRegs->rSize   = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;

    pCalRegs->rCmd = (pMcMP->ucLineCfg<<10)+(pMcMP->ucCfg<<8);
    pCalRegs->rCmd |= CAL_CMD_MOD_EXTEND;
    DrvCal_Cmd(pMcMP->usCalCmd);
      
    CALCKEN = FALSE;
}

/*******************************************************************************
*  Name: DrvCal_MatrixCMP
*  Brief:
*   IN: S1[M][N],S2[M][N]  
*   OUT:1. if pCalRegs->rInt  bit14 = 1, OUT[i][j] = MAX(S1[i][j],S2[i][j])
*       2. if pCalRegs->rInt  bit14 = 0, OUT[i][j] = MIN(S1[i][j],S2[i][j])
* input : pMcMP
* Return: None
*******************************************************************************/
void DrvCal_MatrixCMP(ST_CalMcMatrixParam *pMcMP)
{
    //clk
    CALCKEN = TRUE;
      
    // Address
    pCalRegs->rS1Addr = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr = pMcMP->usDstAddr;   
    pCalRegs->rSize   = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;
    pCalRegs->rInt &= (~(1<<14));
    pCalRegs->rInt |= (pMcMP->ucDataType<<14);                                //ucDataType������ȡ���ֵ������Сֵ

    pCalRegs->rCmd &= (~(1<<8));    
    pCalRegs->rCmd |= CAL_CMD_MOD_EXTEND + ((pMcMP->ucCfg & 0x01)<<8);        //ucCfg�������޷���

    DrvCal_Cmd(pMcMP->usCalCmd);
      
    CALCKEN = FALSE;
}

/*******************************************************************************
*  Name: DrvCal_MatrixMul3
*  Brief:
*  Brief: S1 : M*1
*         S2 : N*1
*         OUT: M*N
*         Out[i][j] = S1[i][1]*S2[j][1];
* input : pMcMP
* Return: None
*******************************************************************************/
void DrvCal_MatrixMul3(ST_CalMcMatrixParam *pMcMP)
{
    //clk
    CALCKEN = TRUE;
      
    // Address
    pCalRegs->rS1Addr = pMcMP->usSrc1Addr;
    pCalRegs->rS2Addr = pMcMP->usSrc2Addr;
    pCalRegs->rOutAddr = pMcMP->usDstAddr;     
    pCalRegs->rSize   = (pMcMP->ucTxLen<<8) + pMcMP->ucRxLen;
    pCalRegs->rPThd = pMcMP->ucShift;
    pCalRegs->rCmd |= CAL_CMD_MOD_EXTEND;
    DrvCal_Cmd(pMcMP->usCalCmd);
      
    CALCKEN = FALSE;
}
#endif

