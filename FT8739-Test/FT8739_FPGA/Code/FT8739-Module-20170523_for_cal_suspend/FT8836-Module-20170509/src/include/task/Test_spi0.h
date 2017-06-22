/*******************************************************************************
* Copyright (C) 2016-2018, FocalTech Systems (R)��All Rights Reserved.
*
* File Name: Test_spi0.h
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
#ifndef _TEST_SPI0_H__
#define _TEST_SPI0_H__

/*******************************************************************************
* Included files
*******************************************************************************/
#include "CfgGlobal.h"

/*******************************************************************************
* Global constant and macro definitions using #define
*******************************************************************************/
#if _TEST_SPI0_EN
/*******************************************************************************
* Global structures, unions and enumerations using typedef
*******************************************************************************/

/*******************************************************************************
* Global variable extern declarations
*******************************************************************************/

/*******************************************************************************
* Global function prototypes
*******************************************************************************/
void Test_SPI0(void);
void SPI0_Init(void);

#else
#define Test_SPI0(void) /##/
#define SPI0_Init(void)   /##/
#endif

#endif