/*  
 *  vd_glue_struct.c
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
#include "vd_glue_struct.h"


/*-----------------------------------------------------------------------*/
/* Modular/Global Variables */

VD_GLUE_STRUCT_IpForwardEntry gForward;

static OS_SPECIFIC_MUTEX mForwardMutex;
static sbyte4            mForwardInfoCount = 0;



/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_STRUCT_ValidateIpAddress (environment *pEnv, void *pInputBuf)
{
    ubyte4      dummy;
    RLSTATUS    status;

    status = CONVERT_StrTo(pInputBuf, &dummy, kDTipaddress);
	return status;
}



/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_STRUCT_SetIpAddress(environment *pEnv, void *pDest, void *pInputBuf, ...)
{
	RLSTATUS    status;

    status = CONVERT_StrTo(pInputBuf, pDest, kDTipaddress);
    if (OK == status)
        status = kChange;

    return status;
}



/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_STRUCT_GetValidInteger (environment *pEnv, void *pOutputBuf, 
                                void *pObject, char *pArgs)
{
    CONVERT_ToStr ( ((sbyte4 *)pObject),
                  (sbyte *)pOutputBuf, kDTinteger );
    return;
}



/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_STRUCT_SetValidInteger (environment *pEnv, void *pDest, void *pInputBuf, ...)
{
    RLSTATUS    status;
    sbyte4      tempDest;
   
    status = CONVERT_StrTo ( ((sbyte*)pInputBuf), &tempDest, kDTinteger );
    switch (status)
    {
        case ERROR_CONVERSION_OVERFLOW:
        case ERROR_CONVERSION_UNDERFLOW:
        case ERROR_CONVERSION_INCORRECT_TYPE:
            status =  ERROR_GENERAL;
            break;
        default:
            *((sbyte4 *)pDest) = tempDest;
            status = kChange;
            break;
    }

    return status;
}



/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_STRUCT_GetIpForwardInfo(environment *pEnv, void *pOutputBuf, 
                              void *pObject, char *pArgs)
{
    CONVERT_ToStr ( (((sbyte4 *)pObject) + mForwardInfoCount),
                  (sbyte *)pOutputBuf, kDTinteger );
    
    if ( 0 == mForwardInfoCount)
        mForwardInfoCount++;
    else
        mForwardInfoCount = 0;    
    
    return;
}



/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_STRUCT_SetIpForwardInfo(environment *pEnv, void *pDest, void *pInputBuf, ...)
{
    RLSTATUS    status;
    
    status = VD_GLUE_STRUCT_SetValidInteger( pEnv, 
                                            (((sbyte4 *)pDest) + mForwardInfoCount),
                                            pInputBuf );
    if ( 0 == mForwardInfoCount )
        mForwardInfoCount++;
    else
        mForwardInfoCount = 0;

    return status;
}



/*-----------------------------------------------------------------------*/
extern void *
VD_GLUE_STRUCT_GetIpForwardEntryAddress(environment *pEnv, UniqId Id)
{
	return &gForward;
}



/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_STRUCT_LockIpForwardEntry(UniqId Id)
{
    OS_SPECIFIC_MUTEX_WAIT( mForwardMutex );
    return;
}



/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_STRUCT_UnlockIpForwardEntry(UniqId Id)
{
    OS_SPECIFIC_MUTEX_RELEASE( mForwardMutex );
    return;
}


/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_STRUCT_Init( void *pArg )
{
    sbyte4 tempForwardInfo[] = { 0, 0 };
    sbyte4 index;

    /* Initialize gForward using predefined values */
    CONVERT_StrTo("1.1.1.1",       &gForward.ipForwardDest,    kDTipaddress);
    CONVERT_StrTo("2.2.2.2",       &gForward.ipForwardNextHop, kDTipaddress);
    CONVERT_StrTo("255.255.255.0", &gForward.ipForwardMask,    kDTipaddress);

    gForward.ipForwardIfIndex   = 0;
    gForward.ipForwardMetric1   =  1;
    gForward.ipForwardMetric2   = -1;
    gForward.ipForwardMetric3   = -1;
    gForward.ipForwardMetric4   = -1;
    gForward.ipForwardMetric5   = -1;
    gForward.ipForwardAge       = 255;
    
    for (index = 0; index < 2; index++)
        gForward.ipForwardInfo[index] = tempForwardInfo[index];
    
    OS_SPECIFIC_MUTEX_CREATE( &mForwardMutex );

    return;
}


