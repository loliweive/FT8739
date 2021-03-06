/*******************************************************************************
* Copyright (C) 2013-2014, FocalTech Systems (R)��All Rights Reserved.
*
* File Name: Test_RomEcc.h
*
*    Author: xinkunpeng
*
*   Created: 2014-04-18
*
*  Abstract:
*
* Reference:
*
* Version:
* 0.1:
*
*******************************************************************************/
#ifndef __TEST_ROMECC__
#define __TEST_ROMECC__

/*******************************************************************************
* 1.Included files
*******************************************************************************/
#include "CfgGlobal.h"

#if _TEST_ROMECC_EN

/*******************************************************************************
* 2.Global constant and macro definitions using #define
*******************************************************************************/

/*******************************************************************************
* 3.Global structures, unions and enumerations using typedef
*******************************************************************************/

/*******************************************************************************
* 4.Global variable extern declarations
*******************************************************************************/

/*******************************************************************************
* 5.Global function prototypes
*******************************************************************************/
void Test_RomEcc(void);
#else
#define Test_RomEcc  /##/
#endif

#endif

