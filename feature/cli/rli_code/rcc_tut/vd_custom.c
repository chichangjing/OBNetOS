/*
 *  vd_custom.c
 *
 *  This is a part of the RapidControl SDK source code library.
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */

#include "rc.h"
#include "rcc.h"
#include "vd_custom.h"
#include <stdio.h>

/*-----------------------------------------------------------------------*/

extern RLSTATUS VD_CUSTOM_ValidateTime(environment *pEnv, sbyte *pString, DTTypeInfo *pTypeInfo, Boolean errorMsg)
{
	RLSTATUS status  = OK;
    int      hour    = -1;
    int      minute  = -1;
    int      second  = -1;
    int      timeLen = -1;


	sscanf(pString,"%d:%d:%d", &hour, &minute, &second);

    timeLen = STRLEN(pString);

	if ((0 > hour)   || (hour   > 23) ||
	    (0 > minute) || (minute > 59) ||
	    (0 > second) || (second > 59) ||
	    (timeLen > 8) )
    {
        status = ERROR_GENERAL_OUT_OF_RANGE;

        if (errorMsg)
            RCC_DB_AppendErrorText(pEnv, "00:00:00 to 23:59:59", 0);
    }

	return status;

}


