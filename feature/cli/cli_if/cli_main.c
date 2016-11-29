
/*************************************************************
 * Filename     : cli_main.c
 * Description  : Entry for CLI
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#include "mconfig.h"

#include <stdio.h>

#include "rc.h"
#ifdef __RCC_ENABLED__
#include "rcc.h"
#endif /* __RCC_ENABLED__ */

#ifdef __RCC_TUTORIAL__
#include "vd_glue.h"
#include "vd_glue_basic.h"
#include "vd_glue_struct.h"
#include "vd_glue_table.h"
#endif /* __RCC_TUTORIAL__ */

#include "svn_revision.h"
#include "cli_util.h"

void cli_telnet_welcome(cli_env *pCliEnv)
{
	extern char FirmareVersion[];

	if(NULL == pCliEnv)
		return;
	#if 0
	cli_printf(pCliEnv, "\r\n\r\n");
	cli_printf(pCliEnv, "  ***************************************\r\n");	
	cli_printf(pCliEnv, "  **           Welcome to OBNetOS             **\r\n");
	cli_printf(pCliEnv, "  **  -- -- -- -- -- -- -- -- -- -- -- -- --  **\r\n");
	cli_printf(pCliEnv, "  **   Firmware version release: v%s     **\r\n", &FirmareVersion[0]);
	cli_printf(pCliEnv, "  **   Firmware version release: v%s     **\r\n", &FirmareVersion[0]);	
	cli_printf(pCliEnv, "  **   Copyright (c) OB Telecom Electronics   **\r\n");
	cli_printf(pCliEnv, "  **********************************************\r\n");	
	cli_printf(pCliEnv, "\r\n");
	#endif

	cli_printf(pCliEnv, "\r\n\r\n");
	cli_printf(pCliEnv, "  ******************************************\r\n");
	cli_printf(pCliEnv, "  **          Welcome to OBNetOS          **\r\n");
	cli_printf(pCliEnv, "  **   -- -- -- -- -- -- -- -- -- -- --   **\r\n");
#if RELEASE_TRUNK_VERSION
	cli_printf(pCliEnv, "  **   Version release : v%s r%-5d  **\r\n", &FirmareVersion[0], SVN_REVISION);
#else
	cli_printf(pCliEnv, "  **   Revision release: r%-5d           **\r\n", SVN_REVISION);
#endif
	cli_printf(pCliEnv, "  **   Build time: %s    **\r\n", BUILD_TIME);
	cli_printf(pCliEnv, "  ******************************************\r\n");
	cli_printf(pCliEnv, "\r\n");
	
};

void cli_console_welcom(cli_env *pCliEnv)
{
	extern char FirmareVersion[];

	if(NULL == pCliEnv)
		return;
	
	cli_printf(pCliEnv, "\r\n");
	cli_printf(pCliEnv, "Welcome to Console CLI, please login.\r\n");
	cli_printf(pCliEnv, "\r\n");	
}


void cli_main(void)
{
    Startup start;

	cli_debug_init();
	
    MEMSET(&start, 0, sizeof(Startup));
	
	if(OK > OCB_Init(&start))
		printf("Error -- unable to initialize the OCB\n");
#if 0
#ifdef __RCC_TUTORIAL__
    VD_GLUE_BASIC_Init( NULL );
    VD_GLUE_STRUCT_Init( NULL );
    VD_GLUE_TABLE_Init( NULL );
#endif /* __RCC_TUTORIAL__ */

	start.pArg = NULL;
#endif

#ifdef __RCC_ENABLED__
#ifdef __MULTI_THREADED_SERVER_ENABLED__
	OS_SPECIFIC_CREATE_THREAD((void *)RCC_TELNETD, "tCLI", NULL, 0, 0, NULL);
#else
	RCC_TELNETD();
#endif
#endif

}



