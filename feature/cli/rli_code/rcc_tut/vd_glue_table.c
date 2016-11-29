/*  
 *  vd_glue_table.c
 *
 *  This is a part of the RapidControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */

#include "rc.h"
#include "rcc.h"

#include "vd_glue.h"
#include "vd_glue_struct.h"
#include "vd_glue_table.h"

#include <stdio.h>
#include <stdlib.h>

/* Definitions and default values */
#define kMaxTableEntries                    14
#define kInitialEntries                     10

#define kIpAddrStrLen                       20
#define kIpAddrBaseUnitMaxValue             250
#define kNumMaskBits                        32
#define kStartMask                          0x80000000

#define kMaxIpForwardIfIndex                10
#define kMaxIpForwardMetric                 25
#define kMaxIpForwardAge                    255
#define kMaxIpForwardProto                  6

/* Module variables */
static VD_GLUE_STRUCT_IpForwardEntry	     mIpForwardEntryTable[kMaxTableEntries];
static VD_GLUE_TABLE_IpForwardName           mIpForwardNameTable[kMaxTableEntries];
static ubyte4                                mNumEntries;
static OS_SPECIFIC_MUTEX		             mIpForwardTableMutex;

/*-----------------------------------------------------------------------*/

extern void *
VD_GLUE_TABLE_AddrOfIpForwardEntry(environment *pEnv, ubyte2 id)
{
	return ( (void * )&( mIpForwardEntryTable[0] ) );
}

/*-----------------------------------------------------------------------*/

extern void *
VD_GLUE_TABLE_AddrOfNextIpForwardEntry(environment *pEnv, void *pPrevList, int index)
{
	return ( (void * )&( mIpForwardEntryTable[index] ) );
}

/*-----------------------------------------------------------------------*/

extern void *
VD_GLUE_TABLE_AddrOfIpForwardName(environment *pEnv, ubyte2 id)
{
	return ( (void * )&( mIpForwardNameTable[0] ) );
}

/*-----------------------------------------------------------------------*/

extern void *
VD_GLUE_TABLE_AddrOfNextIpForwardName(environment *pEnv, void *pPrevList, int index)
{
	return ( (void * )&( mIpForwardNameTable[index] ) );
}

/*-----------------------------------------------------------------------*/

extern void 
VD_GLUE_TABLE_LockIpForwardTable(UniqId dummy)
{
	OS_SPECIFIC_MUTEX_WAIT(mIpForwardTableMutex);
}

/*-----------------------------------------------------------------------*/

extern void 
VD_GLUE_TABLE_UnlockIpForwardTable(UniqId dummy)
{
	OS_SPECIFIC_MUTEX_RELEASE(mIpForwardTableMutex);
}

/*-----------------------------------------------------------------------------------*/

static void 
VD_GLUE_TABLE_AssignEntryValues(VD_GLUE_STRUCT_IpForwardEntry *pIpForwardEntry)
{
	sbyte   ipAddrStr[ kIpAddrStrLen ];
    ubyte4  tempIpAddress;
    sbyte4  tempMask;
	ubyte	baseIpAddrUnit;

    /* Create a ipForwardDest address */
	while (10 > (baseIpAddrUnit = rand() % kIpAddrBaseUnitMaxValue));
    MEMSET(&ipAddrStr[0], 0, kIpAddrStrLen);
    sprintf(ipAddrStr, "%d.%d.%d.%d", baseIpAddrUnit, baseIpAddrUnit + 1,
                                      baseIpAddrUnit + 2, baseIpAddrUnit + 3);
    CONVERT_StrTo( ipAddrStr, &tempIpAddress, kDTipaddress );

    pIpForwardEntry->ipForwardDest = tempIpAddress;

    /* Create a ipForwardNextHop address */
	while (10 > (baseIpAddrUnit = rand() % kIpAddrBaseUnitMaxValue));
    MEMSET(&ipAddrStr[0], 0, kIpAddrStrLen);
    sprintf(ipAddrStr, "%d.%d.%d.%d", baseIpAddrUnit, baseIpAddrUnit + 10,
                                      baseIpAddrUnit + 20, baseIpAddrUnit + 30);
    CONVERT_StrTo( ipAddrStr, &tempIpAddress, kDTipaddress );

    pIpForwardEntry->ipForwardNextHop = tempIpAddress;

    /* Create a netmask */
    tempMask = kStartMask;
    tempMask >>= rand() % kNumMaskBits;

    pIpForwardEntry->ipForwardMask = HTON4(tempMask);

    /* fill in the rest */
    pIpForwardEntry->ipForwardIfIndex = rand() % kMaxIpForwardIfIndex;
    pIpForwardEntry->ipForwardMetric1 = rand() % kMaxIpForwardMetric;
    pIpForwardEntry->ipForwardMetric2 = rand() % kMaxIpForwardMetric;
    pIpForwardEntry->ipForwardMetric3 = rand() % kMaxIpForwardMetric;
    pIpForwardEntry->ipForwardMetric4 = rand() % kMaxIpForwardMetric;
    pIpForwardEntry->ipForwardMetric5 = rand() % kMaxIpForwardMetric;
    pIpForwardEntry->ipForwardAge     = rand() % kMaxIpForwardAge;
    pIpForwardEntry->ipForwardInfo[0] = rand() % kMaxIpForwardProto;
    pIpForwardEntry->ipForwardInfo[1] = rand() % kMaxIpForwardProto;

    return;
}

/*-----------------------------------------------------------------------*/

extern void
VD_GLUE_TABLE_RetrieveTableLen(environment *pEnv, void *pOutputBuf, 
                               void *pObject, char *pArgs)
{
    ubyte4  numEntryCount;

    /* This subtraction is necessary because our table iteration code
     * does an inclusive loop.  That is, the interation code checks to
     * see if the loop index is "<=" to the final loop value, rather 
     * than simply checking to see if it is "<" the final value */
    numEntryCount = mNumEntries - 1;

    CONVERT_ToStr(&numEntryCount, pOutputBuf, kDTulong);
    return;
}

/*-----------------------------------------------------------------------*/

extern void 
VD_GLUE_TABLE_Init( void *pArg )
{
    sbyte *pNames[] = {"Paul     ", "David C. ", "David R. ", "Sheila   ", "Sergey   ",
                       "Shawn-Lin", "Lee      ", "Kedron   ", "Soo-Fei  " ,"Eric     " };

    VD_GLUE_STRUCT_IpForwardEntry *pIpForwardEntry;
    VD_GLUE_TABLE_IpForwardName   *pIpForwardName;

    sbyte4 i;

    srand(OS_SPECIFIC_GET_SECS());
    mNumEntries = 0;
    for ( i = 0; i < kInitialEntries; i++ )
    {
        pIpForwardName = &mIpForwardNameTable[i];
        pIpForwardEntry = &mIpForwardEntryTable[i];

        MEMSET(pIpForwardName , 0, sizeof(VD_GLUE_TABLE_IpForwardName));
        MEMSET(pIpForwardEntry, 0, sizeof(VD_GLUE_STRUCT_IpForwardEntry));

        STRCPY(pIpForwardName->name, pNames[i]);
        VD_GLUE_TABLE_AssignEntryValues(pIpForwardEntry);
        mNumEntries++;
    }
    
    OS_SPECIFIC_MUTEX_CREATE( &mIpForwardTableMutex );
    return;
}

/*-----------------------------------------------------------------------*/

extern void
VD_InitIpForwardTableDescriptor(TableDescr *pTableDescr)
{
     pTableDescr->bIsSNMP           = FALSE;
#ifdef __SNMP_API_ENABLED__
     pTableDescr->pTableName        = "";
     pTableDescr->pLengthInstance   = "";
     pTableDescr->pFirstInstance    = "";
     pTableDescr->pSkipAhead        = "";
     pTableDescr->pFilterRule       = "";
     pTableDescr->pFilterType       = "";
     pTableDescr->pCreateTableArgList = NULL;
#endif /* __SNMP_API_ENABLED__ */

     pTableDescr->pDynamicStart = NULL;
     pTableDescr->pDynamicEnd   = "tableEntries";

     pTableDescr->rowStart = 0;
     pTableDescr->rowEnd   = 0;
     pTableDescr->numColEntries = 5;

     pTableDescr->colEntries[0].pRapidMark = "rowName";
     pTableDescr->colEntries[0].pArgs      = NULL;
     pTableDescr->colEntries[0].pTitle     = "Name";
     pTableDescr->colEntries[0].width      = 12;

     pTableDescr->colEntries[1].pRapidMark = "rowIpForwardDest";
     pTableDescr->colEntries[1].pArgs      = NULL;
     pTableDescr->colEntries[1].pTitle     = "Dest";
     pTableDescr->colEntries[1].width      = 15;

     pTableDescr->colEntries[2].pRapidMark = "rowIpForwardMask";
     pTableDescr->colEntries[2].pArgs      = NULL;
     pTableDescr->colEntries[2].pTitle     = "Mask";
     pTableDescr->colEntries[2].width      = 15;

     pTableDescr->colEntries[3].pRapidMark = "rowIpForwardNextHop";
     pTableDescr->colEntries[3].pArgs      = NULL;
     pTableDescr->colEntries[3].pTitle     = "Next";
     pTableDescr->colEntries[3].width      = 15;

     pTableDescr->colEntries[4].pRapidMark = "rowIpForwardMetric1";
     pTableDescr->colEntries[4].pArgs      = NULL;
     pTableDescr->colEntries[4].pTitle     = "Metric";
     pTableDescr->colEntries[4].width      = 5;
}

/*-----------------------------------------------------------------------------------*/

extern void
VD_InitTcpConnTableDescriptor(TableDescr *pTableDescr)
{
    pTableDescr->bIsSNMP          = TRUE;

#ifdef __SNMP_API_ENABLED__
    pTableDescr->pTableName       = "tcpConnState";
    pTableDescr->pLengthInstance  = "*";
    pTableDescr->pFirstInstance   = "";
    pTableDescr->pSkipAhead       = "";
    pTableDescr->pFilterRule      = "";
    pTableDescr->pFilterType      = "";
    pTableDescr->pCreateTableArgList = 
       "tcpConnLocalAddress,tcpConnLocalPort,tcpConnRemAddress,tcpConnRemPort,tcpConnState";

#endif /* __SNMP_API_ENABLED__ */

    pTableDescr->pDynamicStart    = NULL;
    pTableDescr->pDynamicEnd      = "tcpConnTableLen";

    pTableDescr->rowStart         = 0;
    pTableDescr->rowEnd           = 50;
    pTableDescr->numColEntries    = 5;

    pTableDescr->colEntries[0].pRapidMark = "tcpConnLocalAddress";
    pTableDescr->colEntries[0].pArgs      = NULL;
    pTableDescr->colEntries[0].pTitle     = "Local Address";
    pTableDescr->colEntries[0].width      = 15;

    pTableDescr->colEntries[1].pRapidMark = "tcpConnLocalPort";
    pTableDescr->colEntries[1].pArgs      = NULL;
    pTableDescr->colEntries[1].pTitle     = "Local Port";
    pTableDescr->colEntries[1].width      = 0;

    pTableDescr->colEntries[2].pRapidMark = "tcpConnRemAddress";
    pTableDescr->colEntries[2].pArgs      = NULL;
    pTableDescr->colEntries[2].pTitle     = "Rem Address";
    pTableDescr->colEntries[2].width      = 15;

    pTableDescr->colEntries[3].pRapidMark = "tcpConnRemPort";
    pTableDescr->colEntries[3].pArgs      = NULL;
    pTableDescr->colEntries[3].pTitle     = "Rem Port";
    pTableDescr->colEntries[3].width      = 0;

    pTableDescr->colEntries[4].pRapidMark = "tcpConnState";
    pTableDescr->colEntries[4].pArgs      = NULL;
    pTableDescr->colEntries[4].pTitle     = "Conn State";
    pTableDescr->colEntries[4].width      = 0;
}
