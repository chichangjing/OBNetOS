/*  
 *  vd_basic.c
 *
 *  This is a part of the OpenControl SDK source code library. 
 *
 *  Copyright (C) 1998 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */

/*----------------------------------------------------------------------
 *
 * NAME CHANGE NOTICE:
 *
 * On May 11th, 1999, Rapid Logic changed its corporate naming scheme.
 * The changes are as follows:
 *
 *      OLD NAME                        NEW NAME
 *
 *      OpenControl                     RapidControl
 *      WebControl                      RapidControl for Web
 *      JavaControl                     RapidControl for Applets
 *      MIBway                          MIBway for RapidControl
 *
 *      OpenControl Backplane (OCB)     RapidControl Backplane (RCB)
 *      OpenControl Protocol (OCP)      RapidControl Protocol (RCP)
 *      MagicMarkup                     RapidMark
 *
 * The source code portion of our product family -- of which this file 
 * is a member -- will fully reflect this new naming scheme in an upcoming
 * release.
 *
 *
 * RapidControl, RapidControl for Web, RapidControl Backplane,
 * RapidControl for Applets, MIBway, RapidControl Protocol, and
 * RapidMark are trademarks of Rapid Logic, Inc.  All rights reserved.
 *
 */

#include "rc_options.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_errors.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_convert.h"
#include "rc_database.h"
 
#include "vd_defs.h"
#include "vd_basic.h"

#define  kHexPrefix  "0x"

/* This file gives some examples of the APIs used by Rapid Logic. */
int         gGlobalDecimalInteger;
static int  mPrivateDecimalInteger;
long        gGlobalHexadecimalInteger;
static		char PromptString[32];

IpAddress   gDeviceIpAddress;
IpAddress   gDeviceNetMask;
static  IpAddress   mDeviceDefaultGateway;



#define VD_BASIC_ConvertDigit(arg) ((10 > arg) ? (arg + '0') : ((arg - 10) + 'A'))

 
/*-----------------------------------------------------------------------*/

static void 
VD_BASIC_ReverseString(sbyte *pStartString)
{
    sbyte   Char;
    sbyte   *pEndString;

    if (NULL == pStartString)
        return;

    pEndString = pStartString + STRLEN(pStartString) - 1;

    while (pStartString < pEndString)
    {
        Char = *pStartString;
        *pStartString = *pEndString;
        *pEndString = Char;

        pStartString++;
        pEndString--;
    }
}



/*-----------------------------------------------------------------------*/

static RLSTATUS 
VD_BASIC_ConvertHexToStr ( unsigned long value, sbyte *pStr )
{
    sbyte *HeadString = pStr;

    if (NULL == pStr)
        return ERROR_GENERAL_NO_DATA;

    while (value >= 16)
    {
        *pStr++ = (sbyte) VD_BASIC_ConvertDigit((value % 16));
        value /= 16;
    }

    *pStr++ = (sbyte) VD_BASIC_ConvertDigit(value);
    *pStr = '\0';

    VD_BASIC_ReverseString(HeadString);
    return OK;
}



/*-----------------------------------------------------------------------*/

extern void 
VD_BASIC_ReadHexadecimalVariable(environment *pEnv, void *pHtmlOutputBuf, void *pObject, char *pArgs)
{
    sbyte *pOutputBuf;

    pOutputBuf = (sbyte*)pHtmlOutputBuf;
    STRCPY(pOutputBuf, kHexPrefix);

    pOutputBuf += STRLEN(kHexPrefix);
    VD_BASIC_ConvertHexToStr( *((unsigned long*)pObject), (sbyte*)pOutputBuf);
    return;
}



/*-----------------------------------------------------------------------*/

extern void
VD_BASIC_ReadIpAddress(environment *pEnv, void *pHtmlOutputBuf, void *pObject, char *pArgs)
{
    CONVERT_ToStr(pObject, pHtmlOutputBuf, kDTipaddress);
    return;
}



/*-----------------------------------------------------------------------*/

extern void 
VD_BASIC_ReadDefaultGateway(environment *pEnv, void *pHtmlOutputBuf, void *pObject, char *pArgs)
{
    VD_BASIC_ReadIpAddress(pEnv, pHtmlOutputBuf, &mDeviceDefaultGateway, pArgs);
    return;
}



/*-----------------------------------------------------------------------*/

extern void 
VD_BASIC_ReadPrivateVariable(environment *pEnv, void *pHtmlOutputBuf, void *pObject, char *pArgs)
{
    CONVERT_ToStr(&mPrivateDecimalInteger, pHtmlOutputBuf, kDTinteger);
    return;
}

/*-----------------------------------------------------------------------*/

extern void
VD_BASIC_Init( void *pArg )
{
    mPrivateDecimalInteger = 100;
    gGlobalDecimalInteger  = 51;

    gGlobalHexadecimalInteger = 0xD0B62772;

    CONVERT_StrTo("205.178.113.98", &gDeviceIpAddress, kDTipaddress);
    CONVERT_StrTo("255.255.255.0",  &gDeviceNetMask, kDTipaddress);
    CONVERT_StrTo("205.178.113.1",  &mDeviceDefaultGateway, kDTipaddress);
	STRCPY(PromptString, "Sample Prompt");
}



