/*******************************************************************************
* Copyright (C) 2012-2013, FocalTech Systems (R)��All Rights Reserved.
*
* File Name: Drv_mcu.h
*
*    Author: linjianjun
*
*   Created: 2013-09-01
*
*  Abstract:
*
* Reference:
*
* Version:
*
*******************************************************************************/

#ifndef __DRV_MCU_H__
#define __DRV_MCU_H__


/*******************************************************************************
* 1.Included files
*******************************************************************************/
#include "CfgGlobal.h"

/*******************************************************************************
* 2.Global constant and macro definitions using #define
*******************************************************************************/
/* main clk */
#define DIV_CLK_48M           0
#define DIV_CLK_24M           1
#define MAIN_CLK              DIV_CLK_24M

#define I2C_SLAVE_ADDR        0x70  // 

#define DATABUFF  I2CBUF
#define RIFLAG    I2CRI
#define TIFLAG    I2CTI

/*******************************************************************************
* 3.Global structures, unions and enumerations using typedef
*******************************************************************************/

/*******************************************************************************
* 4.Global variable extern declarations
*******************************************************************************/
/*******************************************************************************
* 5.Global function prototypes
*******************************************************************************/
void DrvIOInit(void);
#endif //_DRV_HOST_COMM_H_

