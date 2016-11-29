

/*************************************************************
 * Filename     : ob_debug.c
 * Description  : Debug for OBNetOS
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#include "mconfig.h"

/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* Kernel includes. */
#include "os_mutex.h"

/* BSP includes */
#include "stm32f2xx.h"

/*
#if MODULE_CLI
#include "rc.h"
#include "rcc.h"
#endif
*/

typedef struct {
	u32	modules;
	u16	module_opmode;
	u32	debug_enable:1;
	u32 print_enable:1;
	char buffer[256];
	OS_MUTEX_T	mutex;
} ob_debug_t;

static ob_debug_t DebugDB;

void ob_debug_init(void)
{
	ob_debug_t *pdbg = &DebugDB;

	memset(&DebugDB, 0, sizeof(ob_debug_t));

	pdbg->debug_enable  = 1;
	pdbg->print_enable  = 1;
	pdbg->modules 		= 0;

	os_mutex_init(&pdbg->mutex);
	
	return;
}

void ob_debug_print(int module, char* fmt, ...)
{
	ob_debug_t *pdbg = &DebugDB;
    int count;
    char *buf = pdbg->buffer;
    va_list args;

	if(!pdbg->debug_enable)
		return;

	if(fmt == NULL)
		return;

	if((pdbg->modules & module) == 0)
		return;

	os_mutex_lock(&pdbg->mutex, OS_MUTEX_WAIT_FOREVER);
	
    va_start(args, fmt);
    count = vsprintf(buf, fmt, args); 
    va_end(args);

	printf("%s", buf);	/* console print */
	
	os_mutex_unlock(&pdbg->mutex);	

	return;
}

u32 ob_debug_module_get(void)
{
	return DebugDB.modules;
}

void ob_debug_module_set(u32 module)
{
	DebugDB.modules = module;
}

void ob_debug_module_add(u32 module)
{
	DebugDB.modules |= module;
}

void ob_debug_module_del(u32 module)
{
	DebugDB.modules &= ~module;
}

u32 ob_debug_module_check(u32 module)
{
	return (DebugDB.modules & module);
}

