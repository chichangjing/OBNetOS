/*
 *  rc_environ.c
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
$History: rc_environ.c $
 * 
 * *****************  Version 29  *****************
 * User: Pstuart      Date: 7/30/01    Time: 4:01p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * add explicit braces
 * 
 * *****************  Version 28  *****************
 * User: Pstuart      Date: 2/26/01    Time: 10:46a
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * use new envoy macro test
 * 
 * *****************  Version 27  *****************
 * User: James        Date: 10/09/00   Time: 4:25p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Removed compiler warnings for non-Envoy customers.
 * 
 * *****************  Version 26  *****************
 * User: James        Date: 6/22/00    Time: 2:22p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Moved un-necessary header file.
 * 
 * *****************  Version 25  *****************
 * User: Dreyna       Date: 6/20/00    Time: 3:39p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Updated History
 * 
 * *****************  Version 23  *****************
 * User: Leech        Date: 6/16/00    Time: 1:58p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Added flags to switch invocation of OCSNMP_DestructSnmpEnviron
 * depending on available SNMP agent (Epilogue uses new version, SRI uses
 * old)
 * 
 * *****************  Version 22  *****************
 * User: Schew        Date: 5/26/00    Time: 2:18p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Adding support for WindNET SNMP.
 * 
 * *****************  Version 21  *****************
 * User: Leech        Date: 5/24/00    Time: 5:59p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * changed API of OCSNMP_DestructSnmpEnviron().  Added MIBway headers that
 * contains correct prototype
 * 
 * *****************  Version 20  *****************
 * User: Epeterson    Date: 4/26/00    Time: 2:06p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Enabled VSS auto-archive feature using keyword expansion
 * $History: rc_environ.c $
 * 
 * *****************  Version 29  *****************
 * User: Pstuart      Date: 7/30/01    Time: 4:01p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * add explicit braces
 * 
 * *****************  Version 28  *****************
 * User: Pstuart      Date: 2/26/01    Time: 10:46a
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * use new envoy macro test
 * 
 * *****************  Version 27  *****************
 * User: James        Date: 10/09/00   Time: 4:25p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Removed compiler warnings for non-Envoy customers.
 * 
 * *****************  Version 26  *****************
 * User: James        Date: 6/22/00    Time: 2:22p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Moved un-necessary header file.
 * 
 * *****************  Version 25  *****************
 * User: Dreyna       Date: 6/20/00    Time: 3:39p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Updated History
 * 



*/
#include "rc_options.h"
#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_environ.h"
#include "rc_database.h"
#include "rc_cache.h"


/*-----------------------------------------------------------------------*/

#ifdef __ENABLE_CUSTOM_STRUCT__

/* vector function for initializing a custom struct */
int  (*gp_funcConstructCustom)(environment *p_envVar, void **ppCustomData) = NULL;

/* vector function for destroying a custom struct */
void (*gp_funcDestructCustom) (void *pCustomObject) = NULL;

#endif

#ifdef __SNMP_API_ENABLED__
#ifdef ENVOY_STACK_K
#include "rcm_envoy.h"
#include "rcm_ev_cnv.h"
#include "rcm_snmp.h"

#else

extern void OCSNMP_DestructSnmpEnviron(environment *p_envVar);
extern RLSTATUS	OCSNMP_ConstructSnmpEnviron(environment *p_envVar);

#endif
#endif



/*-----------------------------------------------------------------------*/

extern RLSTATUS
ENVIRONMENT_PutEnvironment(environment *p_envVar, Counter index, sbyte *pValue)
{
    if ((NULL == p_envVar) || (kMaxEnvSize <= index) ||
        (NULL != p_envVar->variables[index]))
    {
        return ERROR_GENERAL_NOT_FOUND;
    }

    p_envVar->variables[index] = pValue;

    return OK;

}



/*-----------------------------------------------------------------------*/

extern sbyte *ENVIRONMENT_GetEnvironment(environment *p_envVar, Counter index)
{
    if ((NULL == p_envVar) || (kMaxEnvSize <= index))
        return NULL;

    return p_envVar->variables[index];

}


/*-----------------------------------------------------------------------*/

extern RLSTATUS
ENVIRONMENT_ReplaceEnvironment(environment *p_envVar, Counter index, sbyte *pValue)
{
    if ((NULL == p_envVar) || (kMaxEnvSize <= index) ||
        (NULL == p_envVar->variables[index]))
    {
        return ERROR_GENERAL_NOT_FOUND;
    }

    STRCPY(p_envVar->variables[index], pValue);

    return OK;

}



/*-----------------------------------------------------------------------*/

extern Access ENVIRONMENT_GetAccessLevel(environment *p_envVar)
{
    /* return the lowest access level if we get a bad p_envVar */
    if (NULL == p_envVar)
        return 0;

    return p_envVar->UserLevel;
}


/*-----------------------------------------------------------------------*/

extern Boolean ENVIRONMENT_IsPostValid(environment *p_envVar)
{
    if (NULL == p_envVar)
        return FALSE;

    return p_envVar->PostValid;
}



/*-----------------------------------------------------------------------*/

extern void ENVIRONMENT_SetPostInvalid(environment *p_envVar)
{
    if (NULL == p_envVar)
        return;

    p_envVar->PostValid = FALSE;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS
ENVIRONMENT_Construct(OS_SPECIFIC_SOCKET_HANDLE sock, environment **pp_envInit)
{
    int         index;

    if (NULL == (*pp_envInit = RC_MALLOC(sizeof(environment))))
        return ERROR_MEMMGR_NO_MEMORY;

    MEMSET(*pp_envInit, 0x00, sizeof(environment));

    (*pp_envInit)->phCacheHandle        = NULL;

#ifdef __MACRO_REPEAT_NEST__
	for (index=0;index<kMacroRepeatNestMax;index++)
	{
		(*pp_envInit)->Macro_Index[index]			  = 0;
		(*pp_envInit)->p_MacroLstIndexedValues[index] = List_Construct();
#ifdef __MACRO_REPEAT_NEST_FULL__
		(*pp_envInit)->Macro_TableName[index][0]	  = '\0';
		(*pp_envInit)->p_MacroSnmpData[     index]    = NULL;
#endif
	}
	for (index=0;index<kMacroTableMax;index++)
	{
		(*pp_envInit)->Macro_Type[  index]       = 0;
		(*pp_envInit)->Macro_pStart[index]       = 0;
		(*pp_envInit)->Macro_pExit[ index]       = 0;
		(*pp_envInit)->Macro_Length[index]       = 0;
	}
    (*pp_envInit)->Macro_NestDepth		= 0;
    (*pp_envInit)->Macro_Counter		= -1;
#endif
    (*pp_envInit)->MacroIndex           = 0;
    (*pp_envInit)->p_lstIndexedValues   = List_Construct();
    (*pp_envInit)->pData                = NULL;
    (*pp_envInit)->cBytesAnalyzed       = 0;
    (*pp_envInit)->ScopeDepth           = 0;

    (*pp_envInit)->sock                 = sock;
    (*pp_envInit)->PostValid            = TRUE;
    (*pp_envInit)->PostBuffer           = NULL;
    (*pp_envInit)->PostBufferLen        = 0;
#ifdef LOGIN_ACCESS_ENABLE
    (*pp_envInit)->UserLevel            = (1 << __RLI_ACCESS_LEVEL_SHIFT__);
#else
    (*pp_envInit)->UserLevel            = 0;
#endif    
    (*pp_envInit)->TxFlowStatus         = kFlowOn;
    (*pp_envInit)->clientIndex          = -1;
    (*pp_envInit)->isPersistent         = FALSE;
    (*pp_envInit)->sendChunkedData      = FALSE;
/*     (*pp_envInit)->blade   = kBTunknown; */
    (*pp_envInit)->blade   = kBTwc;

    for (index = 0; index < kMaxEnvSize; index++)
        (*pp_envInit)->variables[index] = NULL;

#ifdef __OCP_ENABLED__
    (*pp_envInit)->plst_MmOps = NULL;
    (*pp_envInit)->pContext   = NULL;
    (*pp_envInit)->bIsSetOp = FALSE;
#endif  /* __OCP_ENABLED__ */

    return OK;

}   /* ENVIRONMENT_Construct */



/*-----------------------------------------------------------------------*/

static void DestructListObject(void *p_lstObject)
{
    if (NULL != p_lstObject)
        RC_FREE(p_lstObject);
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS
ENVIRONMENT_Destruct(environment **pp_envTemp)
{
    environment *p_envTemp;
    Counter index;

    if (NULL == pp_envTemp)
        return ERROR_GENERAL_NO_DATA;

    p_envTemp = *pp_envTemp;

    if (NULL == p_envTemp)
        return ERROR_GENERAL_NO_DATA;

#ifdef __MACRO_REPEAT_NEST__
	for (index=0;index<kMacroRepeatNestMax;index++)
	{
		if (NULL != p_envTemp->p_MacroLstIndexedValues[index])
			List_Destruct(&(p_envTemp->p_MacroLstIndexedValues[index]), (void(*)(void *))DestructListObject);
	}
#endif
    if (NULL != p_envTemp->p_lstIndexedValues)
        List_Destruct(&(p_envTemp->p_lstIndexedValues), (void(*)(void *))DestructListObject);

    if (NULL != p_envTemp->phCacheHandle)
    {
#ifndef __DISABLE_STRUCTURES__
        Cache_Destruct(p_envTemp, &(p_envTemp->phCacheHandle));
#endif /* __DISABLE_STRUCTURES__ */
        p_envTemp->phCacheHandle = NULL;
    }

    if (NULL != p_envTemp->PostBuffer)
    {
        RC_FREE(p_envTemp->PostBuffer);
        p_envTemp->PostBuffer = NULL;
    }

    for (index = 0; index < kMaxEnvSize; index++)
    {
        if (NULL != p_envTemp->variables[index])
        {
            RC_FREE(p_envTemp->variables[index]);
            p_envTemp->variables[index] = NULL;
        }
    }

#ifdef __SNMP_API_ENABLED__
#ifdef ENVOY_STACK_K
	OCSNMP_DestructSnmpEnviron(p_envTemp, NULL);
#else
	OCSNMP_DestructSnmpEnviron(p_envTemp);
#endif
#endif

#ifdef __ENABLE_CUSTOM_STRUCT__
    if (NULL != gp_funcDestructCustom)
        gp_funcDestructCustom(p_envTemp->pCustomData);
#endif

    RC_FREE(p_envTemp);
    *pp_envTemp = NULL;

    return OK;
}



/*-----------------------------------------------------------------------*/

#ifdef __ENABLE_CUSTOM_STRUCT__

extern void ENVIRONMENT_SetUpCustomVectors(
        int  (*p_funcConstructCustom)(environment *p_envVar, void **ppCustomData),
        void (*p_funcDestructCustom) (void *pCustomData) )
{
    gp_funcConstructCustom = p_funcConstructCustom;
    gp_funcDestructCustom  = p_funcDestructCustom;
}

extern void *ENVIRONMENT_GetPointerToCustomData(environment *p_envVar)
{
    return (p_envVar->pCustomData);
}

#endif

extern enum BladeType ENVIRONMENT_GetBladeType(environment *p_envVar)
{
	return p_envVar->blade;
}	

#ifdef __OCP_ENABLED__
extern  sbyte*      ENVIRONMENT_GetContext(environment *p_envVar)
{
    return (p_envVar->pContext);
}

extern RLSTATUS ENVIRONMENT_PutContext(environment *p_envVar, sbyte *pValue)
{
    p_envVar->pContext = pValue;

    return OK;
}

extern Boolean ENVIRONMENT_isSetOp(environment *p_envVar)
{
	return p_envVar->bIsSetOp;
}	

#endif  /* __OCP_ENABLED__ */

