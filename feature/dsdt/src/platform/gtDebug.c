
#include "mconfig.h"
#if MARVELL_SWITCH

#include <Copyright.h>
/********************************************************************************
* debug.c
*
* DESCRIPTION:
*       Debug message display routine
*
* DEPENDENCIES:
*       OS Dependent
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#ifdef DEBUG_QD
#ifdef _VXWORKS
#include "vxWorks.h"
#include "logLib.h"
#include "stdarg.h"
#elif defined(WIN32)
#include "windows.h"
/* #include "wdm.h" */
#elif defined(LINUX)
#include "stdarg.h"
#elif defined(OS_FREERTOS)
#include "stdarg.h"
#include "stdio.h"
#endif

/*******************************************************************************
* gtDbgPrint
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*       None
*
*******************************************************************************/
#if defined(_VXWORKS) || defined(WIN32) || defined(LINUX) || defined(OS_FREERTOS)
void gtDbgPrint(char* format, ...)
{
    va_list argP;
    char dbgStr[100] = "";

    va_start(argP, format);

    vsprintf(dbgStr, format, argP);

#ifdef _VXWORKS
	printf("%s",dbgStr);
/*	logMsg(dbgStr,0,1,2,3,4,5); */
#elif defined(WIN32)
	printf("%s",dbgStr);
/*	DbgPrint(dbgStr);*/
#elif defined(LINUX)
	printk("%s",dbgStr);
#elif defined(OS_FREERTOS)
	printf("%s\r\n",dbgStr);
#endif
	return;
}
#else
void gtDbgPrint(char* format, ...)
{
}
#endif
#else /* DEBUG_QD not defined */
void gtDbgPrint(char* format, ...)
{
}
#endif /* DEBUG_QD */

#endif