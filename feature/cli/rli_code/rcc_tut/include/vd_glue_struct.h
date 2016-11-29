/*  
 *  vd_glue_struct.h
 *
 *  This is a part of the RapidControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */

#ifndef __VD_GLUE_STRUCT_HEADER__
#define __VD_GLUE_STRUCT_HEADER__

typedef struct VD_GLUE_STRUCT_IpForwardEntry
{
    IpAddress   ipForwardDest;
    IpAddress   ipForwardNextHop;
    IpAddress   ipForwardMask;
    sbyte4      ipForwardIfIndex;
    sbyte4      ipForwardMetric1;
    sbyte4      ipForwardMetric2;
    sbyte4      ipForwardMetric3;
    sbyte4      ipForwardMetric4;
    sbyte4      ipForwardMetric5;
    sbyte4      ipForwardAge;
    sbyte4      ipForwardInfo[2];

} VD_GLUE_STRUCT_IpForwardEntry;

extern VD_GLUE_STRUCT_IpForwardEntry gForward;

/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_STRUCT_ValidateIpAddress(environment *pEnv, void *pInputBuf);


/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_STRUCT_SetIpAddress(environment *pEnv, void *pDest, void *pInputBuf, ...);


/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_STRUCT_GetIpForwardInfo(environment *pEnv, void *pOutputBuf, 
                              void *pObject, char *pArgs);


/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_STRUCT_SetIpForwardInfo(environment *pEnv, void *pDest, void *pString, ...);


/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_STRUCT_GetValidInteger(environment *pEnv, void *pOutputBuf, 
                               void *pObject, char *pArgs);

/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_STRUCT_SetValidInteger (environment *pEnv, void *pDest, void *pInputBuf, ...);


/*-----------------------------------------------------------------------*/
extern void *
VD_GLUE_STRUCT_GetIpForwardEntryAddress(environment *pEnv, UniqId Id);


/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_STRUCT_LockIpForwardEntry(UniqId Id);


/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_STRUCT_UnlockIpForwardEntry(UniqId Id);


/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_STRUCT_Init( void *pArg );


#endif /* __VD_GLUE_STRUCT_HEADER__ */
