/*
 *  rc_ocb_init.c
 *
 *  This is a part of the OpenControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
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
/*
$History: rc_ocb_init.c $
 * 
 * *****************  Version 19  *****************
 * User: Dreyna       Date: 7/19/00    Time: 7:25p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Utilized new rc.h for rc_options.h compatibility.
 * 
 * *****************  Version 18  *****************
 * User: Dreyna       Date: 6/20/00    Time: 3:39p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Updated History
 * 
 * *****************  Version 17  *****************
 * User: Pstuart      Date: 5/08/00    Time: 3:20p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Made MsgHdlr_SystemInit 2nd in OCB_Init
 * 
 * *****************  Version 16  *****************
 * User: Dreyna       Date: 5/08/00    Time: 1:47p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * In OCB_Init(), initialized the memory manager before running startup
 * functions (to allow mallocs).
 * 
 * *****************  Version 15  *****************
 * User: Schew        Date: 5/02/00    Time: 3:08p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Change rc_oc_init.c to rc_ocb_init.c in title
 * 
 * *****************  Version 14  *****************
 * User: Schew        Date: 4/28/00    Time: 10:03a
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Change #include rc_ocpgroup.h to #include rca_ocpgroup.h
 * 
 * *****************  Version 13  *****************
 * User: Pstuart      Date: 4/27/00    Time: 4:43p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * updated #includes for rc_ name change
 * 


*/

#include "rc.h"

#ifdef __FILE_MANAGER_ENABLED__
#include "rc_filemgr.h"
#endif

#include "rc_database.h"
#include "rc_startup.h"

#ifdef __DECOMPRESSION_ENABLED__
#include "rc_dc_defs.h"
#include "rc_inflate.h"
#include "rc_decomp.h"
#endif

#ifdef __OCP_ENABLED__
#include "rca_ocpgroup.h"
#endif /* __OCP_ENABLED__ */    


#include "rc_ocb_init.h"

#ifdef __EPILOGUE_ENVOY_MASTER_SUBAGENT_SNMP_STACK__
extern RLSTATUS MIBWAY_CreateMutexPool(void);
#endif /* __EPILOGUE_ENVOY_MASTER_SUBAGENT_SNMP_STACK__ */

/*-----------------------------------------------------------------------*/

extern RLSTATUS
OCB_Init(Startup *pStartup)
{
    RLSTATUS status;

    if (OK != (status = MemMgr_Init()))
        return status;

    if (OK != (status = MsgHdlr_SystemInit()))
        return status;

    if (NULL != pStartup)
        if (NULL != pStartup->pfuncPreInit)
            pStartup->pfuncPreInit(pStartup->pPreArg);

#ifdef __FILE_MANAGER_ENABLED__
    FILEMGR_Construct();
#endif

#ifdef __DECOMPRESSION_ENABLED__
    status = DECOMP_Init();
    if ( 0 > status )
    {
        OS_SPECIFIC_LOG_ERROR(kUnrecoverableError, "Unable to configure decompression");
        return status;
    }
#endif

    if (OK > (status = DB_Construct()) )
    {
        OS_SPECIFIC_LOG_ERROR(kUnrecoverableError, "Unable to build the OCB");
        return status;
    }

#ifdef __OCP_ENABLED__
    if (OK != OCPGRP_InitGroups())
    {
        OS_SPECIFIC_LOG_ERROR(kWarningError, "Unable to create groups");
    }
#endif /* __OCP_ENABLED__ */    

    Ignite_Database();
    if (OK > (status = DB_LockDown()) )
    {
        OS_SPECIFIC_LOG_ERROR(kUnrecoverableError, "Unable to lock down the OCB");
        return status;
    }                                               
    
#ifdef __EPILOGUE_ENVOY_MASTER_SUBAGENT_SNMP_STACK__
    if (OK != (status = MIBWAY_CreateMutexPool()) )
    {
        OS_SPECIFIC_LOG_ERROR(kUnrecoverableError, "Unable to create MIBway mutex pool");
        return status;
    }
#endif /* __EPILOGUE_ENVOY_MASTER_SUBAGENT_SNMP_STACK__ */
   
#ifdef __OCP_ENABLED__
	Ignite_Groups();
#endif /* __OCP_ENABLED__ */    
    if (NULL != pStartup)
        if (NULL != pStartup->pfuncMsgHandlerStartup)
            pStartup->pfuncMsgHandlerStartup();

    return OK;
}

