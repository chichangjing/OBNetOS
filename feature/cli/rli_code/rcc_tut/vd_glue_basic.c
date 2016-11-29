/*  
 *  vd_glue_basic.c
 *
 *  This is a part of the RapidControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
 *  All rights reserved.
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

#include "vd_glue.h"
#include "vd_glue_basic.h"

#define  kHexPrefix  "0x"

/*-----------------------------------------------------------------------*/
/* Device level globals */
int         gGlobalInteger;
long        gGlobalHexInteger;
IpAddress   gIpAddress;
IpAddress   gNetMask;

sbyte       gName[kVD_GLUE_MaxStringSize];


/*-----------------------------------------------------------------------*/
/* Module level globals */
static int          mPrivateInteger;
static IpAddress    mNextHop;
static sbyte        mSecureID[kVD_GLUE_MaxStringSize];
static	char		PromptString[32];
static	char		mMessageOfTheDay[64];


/*-----------------------------------------------------------------------*/
/* #defines */

#define kVD_GLUE_BASIC_MinimumSecureIDSize 6

#define VD_GLUE_BASIC_ConvertDigit(arg) \
((10 > arg) ? (arg + '0') : ((arg - 10) + 'A'))

 
/*-----------------------------------------------------------------------*/

extern void 
VD_GLUE_BASIC_ReadPrivateVariable(environment *pEnv, void *pOutputBuf, void *pObject, char *pArgs)
{
    CONVERT_ToStr(&mPrivateInteger, pOutputBuf, kDTinteger);
    return;
}


/*-----------------------------------------------------------------------*/

static void 
VD_GLUE_BASIC_ReverseString(sbyte *pStartString)
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
VD_GLUE_BASIC_ConvertHexToStr ( unsigned long value, sbyte *pStr )
{
    sbyte *HeadString = pStr;

    if (NULL == pStr)
        return ERROR_GENERAL_NO_DATA;

    while (value >= 16)
    {
        *pStr++ = (sbyte) VD_GLUE_BASIC_ConvertDigit((value % 16));
        value /= 16;
    }

    *pStr++ = (sbyte) VD_GLUE_BASIC_ConvertDigit(value);
    *pStr = '\0';

    VD_GLUE_BASIC_ReverseString(HeadString);
    return OK;
}


/*-----------------------------------------------------------------------*/

extern void 
VD_GLUE_BASIC_ReadHexadecimalVariable(environment *pEnv, void *pOutputBuf, 
                                      void *pObject, char *pArgs)
{
    sbyte *pBuf;

    pBuf = (sbyte*)pOutputBuf;
    STRCPY(pOutputBuf, kHexPrefix);

    pBuf += STRLEN(kHexPrefix);
    VD_GLUE_BASIC_ConvertHexToStr( *((unsigned long*)pObject), (sbyte*)pBuf);
    return;
}


/*-----------------------------------------------------------------------*/

extern void
VD_GLUE_BASIC_ReadIpAddress(environment *pEnv, void *pOutputBuf, 
                            void *pObject, char *pArgs)
{
    CONVERT_ToStr(pObject, pOutputBuf, kDTipaddress);
    return;
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS 
VD_GLUE_BASIC_WriteIpAddress(environment *pEnv, void *pDest, void *pInputBuf, ...)
{
	return CONVERT_StrTo(pInputBuf, pDest, kDTipaddress);
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS 
VD_GLUE_BASIC_ValidateIpAddress(environment *pEnv, void *pInputBuf)
{
    ubyte4      dummy;
    RLSTATUS    status;

    /* We'll simply take advantage of the error checking inherent in 
     * the CONVERT_StrTo() function to verify that the IP Address
     * is of the form "aaa.bbb.ccc.ddd".
     */
    status = CONVERT_StrTo(pInputBuf, &dummy, kDTipaddress);
	return status;
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS 
VD_GLUE_BASIC_ValidateNetMask(environment *pEnv, void *pInputBuf)
{
    ubyte4      dummy;
    RLSTATUS    status;

    /* We'll simply take advantage of the error checking inherent in 
     * the CONVERT_StrTo() function to verify that the IP Address
     * is of the form "aaa.bbb.ccc.ddd".
     */
    status = CONVERT_StrTo(pInputBuf, &dummy, kDTipaddress);
	return status;
}


/*-----------------------------------------------------------------------*/

extern void 
VD_GLUE_BASIC_ReadNextHop(environment *pEnv, void *pOutputBuf, void *pObject, char *pArgs)
{
    VD_GLUE_BASIC_ReadIpAddress(pEnv, pOutputBuf, &mNextHop, pArgs);
    return;
}


/*-----------------------------------------------------------------------*/

extern void 
VD_GLUE_BASIC_ReadSecureID(environment *pEnv, void *pOutputBuf, void *pObject, 
                     char *pArgs)
{
    STRCPY(pOutputBuf, mSecureID);
    return;
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS 
VD_GLUE_BASIC_WriteSecureID(environment *pEnv, void *pDest, void *pInputBuf, ...)
{
    STRCPY(mSecureID, pInputBuf);
	return OK;
}


/*-----------------------------------------------------------------------*/

static RLSTATUS 
VD_GLUE_BASIC_CheckMaxStrLen(sbyte *pBuf)
{
    if ( kVD_GLUE_MaxStringSize <= STRLEN(pBuf))
        return ERROR_GENERAL;

	return OK;
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS 
VD_GLUE_BASIC_ValidateName(environment *pEnv, void *pInputBuf)
{
    RLSTATUS    status;

    status = VD_GLUE_BASIC_CheckMaxStrLen((sbyte*)pInputBuf);
	return status;
}


/*-----------------------------------------------------------------------*/

static RLSTATUS 
VD_GLUE_BASIC_CheckSecureIDLen(sbyte *pInputBuf)
{
    RLSTATUS    status;

    status = VD_GLUE_BASIC_CheckMaxStrLen((sbyte*)pInputBuf);
    if ( OK > status )
        return status;

    if ( kVD_GLUE_BASIC_MinimumSecureIDSize > STRLEN(pInputBuf))
        return ERROR_GENERAL;
    
    return OK;
}


/*-----------------------------------------------------------------------*/

static RLSTATUS 
VD_GLUE_BASIC_CheckSecureIDContent(sbyte *pInputBuf)
{ 
    /* make sure it has at least one digit in it */
    while ( '\0' != *pInputBuf )
    {
        if ( ISDIGIT( *pInputBuf ))
            return OK;
        
        pInputBuf++;
    }
    
    return ERROR_GENERAL_NOT_FOUND;
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS 
VD_GLUE_BASIC_ValidateSecureID(environment *pEnv, void *pInputBuf)
{
    RLSTATUS    status;

    status = VD_GLUE_BASIC_CheckSecureIDLen((sbyte*)pInputBuf);
    if ( OK > status )
        return status;

    status = VD_GLUE_BASIC_CheckSecureIDContent((sbyte*)pInputBuf);
    return status;
}



/*-----------------------------------------------------------------------*/

void VD_MOTDRead(environment *pEnv, void *pOutputBuf, void *pObject, sbyte *pArgs)
{
	STRCPY(pOutputBuf, mMessageOfTheDay);
	return;
}

/*-----------------------------------------------------------------------*/

RLSTATUS VD_MOTDWrite(environment *pEnv, void *pDest, void *pInputBuf, sbyte *pArgs, ...)
{

	sbyte*		pTempBuf= (sbyte*) pInputBuf;
	
	if(STRLEN(pTempBuf)>31)
		return -1;
	else
		STRCPY(mMessageOfTheDay, pTempBuf);
	return OK;
}

/*-----------------------------------------------------------------------*/

void VD_CLIPromptExampleRead(environment *pEnv, void *pOutputBuf, void *pObject, sbyte *pArgs)
{
	STRCPY(pOutputBuf, PromptString);
	return;
}

/*-----------------------------------------------------------------------*/

RLSTATUS VD_CLIPromptExampleWrite(environment *pEnv, void *pDest, void *pInputBuf, sbyte *pArgs, ...)
{
	sbyte*		pTempBuf= (sbyte*) pInputBuf;
	
	if(STRLEN(pTempBuf)>31)
		return -1;
	else
		STRCPY(PromptString, pTempBuf);
	return OK;
}

/*-----------------------------------------------------------------------*/

extern void
VD_GLUE_BASIC_Init( void *pArg )
{
    gGlobalInteger      = 888;
    mPrivateInteger     = 333;

    gGlobalHexInteger   = 0xD0B62772;  /* Lee's Birthday */

    CONVERT_StrTo("205.178.113.98", &gIpAddress, kDTipaddress);
    CONVERT_StrTo("255.255.255.0",  &gNetMask,   kDTipaddress);
    CONVERT_StrTo("205.178.113.1",  &mNextHop,   kDTipaddress);

    MEMSET(gName, 0, kVD_GLUE_MaxStringSize);
    MEMSET(mSecureID, 0, kVD_GLUE_MaxStringSize);

    STRCPY(gName, "SuperDuper 8000");
    STRCPY(mSecureID, "Y2K");
	STRCPY(PromptString, "Sample Prompt");
	STRCPY(mMessageOfTheDay, "Welcome to the RCC tutorial");
}

#if 1

ubyte4   gPortNo[kRCC_MAX_CLI_TASK+1] = {0};
ubyte4   gDomainID[kRCC_MAX_CLI_TASK+1] = {0};
extern environment  **gppCliSessions;

extern void VD_ReadPortNo(environment *pEnv, void *pOutputBuf, void *pObject, sbyte *pArgs)
{
	int numConn = 0;

	for(numConn=0; numConn<kRCC_MAX_CLI_TASK+1; numConn++) {
		if(pEnv == gppCliSessions[numConn])
			break;
	}
	if(numConn == kRCC_MAX_CLI_TASK+1)
		return;

	CONVERT_ToStr(&gPortNo[numConn], pOutputBuf, kDTuinteger);
	
	return;
}  

extern RLSTATUS VD_WritePortNo(environment *pEnv, void *pDest, void *pInputBuf, ...)
{
	int numConn = 0;

	for(numConn=0; numConn<kRCC_MAX_CLI_TASK+1; numConn++) {
		if(pEnv == gppCliSessions[numConn])
			break;
	}
	if(numConn == kRCC_MAX_CLI_TASK+1)
		return ERROR_GENERAL;

    CONVERT_StrTo(pInputBuf, &gPortNo[numConn], kDTuinteger);

    return OK;
} 

extern void VD_ReadDomainID(environment *pEnv, void *pOutputBuf, void *pObject, sbyte *pArgs)
{
	int numConn = 0;

	for(numConn=0; numConn<kRCC_MAX_CLI_TASK+1; numConn++) {
		if(pEnv == gppCliSessions[numConn])
			break;
	}
	if(numConn == kRCC_MAX_CLI_TASK+1)
		return;

	CONVERT_ToStr(&gDomainID[numConn], pOutputBuf, kDTuinteger);
	
	return;
}  

extern RLSTATUS VD_WriteDomainID(environment *pEnv, void *pDest, void *pInputBuf, ...)
{
	int numConn = 0;

	for(numConn=0; numConn<kRCC_MAX_CLI_TASK+1; numConn++) {
		if(pEnv == gppCliSessions[numConn])
			break;
	}
	if(numConn == kRCC_MAX_CLI_TASK+1)
		return ERROR_GENERAL;

    CONVERT_StrTo(pInputBuf, &gDomainID[numConn], kDTuinteger);

    return OK;
} 
#endif

