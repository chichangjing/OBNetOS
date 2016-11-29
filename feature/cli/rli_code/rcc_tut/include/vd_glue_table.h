/*  
 *  vd_glue_table.h
 *
 *  This is a part of the RapidControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */


#ifndef __VD_GLUE_TABLE_HEADER__
#define __VD_GLUE_TABLE_HEADER__

#include "rcc_custom.h"
#include "rcc_structs.h"
#include "rcc_rcb.h"

#define kMaxForwardNameSize   32

typedef struct VD_GLUE_TABLE_IpForwardName 
{
    sbyte   name[kMaxForwardNameSize];

} VD_GLUE_TABLE_IpForwardName;

/*-----------------------------------------------------------------------------------*/

extern void *
VD_GLUE_TABLE_AddrOfIpForwardEntry(environment *pEnv, ubyte2 id);

/*-----------------------------------------------------------------------------------*/

extern void *
VD_GLUE_TABLE_AddrOfNextIpForwardEntry(environment *pEnv, void *pPrevList, int index);

/*-----------------------------------------------------------------------------------*/

extern void 
VD_GLUE_TABLE_LockIpForwardTable(UniqId dummy);

/*-----------------------------------------------------------------------------------*/

extern void 
VD_GLUE_TABLE_UnlockIpForwardTable(UniqId dummy);

/*-----------------------------------------------------------------------------------*/

extern void *
VD_GLUE_TABLE_AddrOfIpForwardName(environment *pEnv, ubyte2 id);

/*-----------------------------------------------------------------------------------*/

extern void *
VD_GLUE_TABLE_AddrOfNextIpForwardName(environment *pEnv, void *pPrevList, int index);

/*-----------------------------------------------------------------------------------*/

extern void
VD_GLUE_TABLE_RetrieveTableLen(environment *pEnv, void *pOutputBuf, 
                               void *pObject, char *pArgs);

/*-----------------------------------------------------------------------------------*/

extern void VD_GLUE_TABLE_Init( void *pArg );

/*-----------------------------------------------------------------------------------*/

extern void
VD_InitIpForwardTableDescriptor(TableDescr *pTableDescr);

/*-----------------------------------------------------------------------------------*/

extern void
VD_InitTcpConnTableDescriptor(TableDescr *pTableDescr);

#endif /* __VD_GLUE_TABLE_HEADER__ */
