/*
 *  rc_access.c
 *
 *  This is a part of the OpenControl SDK source code library.
 *
 *  Copyright (C) 1999 Rapid Logic, Inc.
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




#include "rc_options.h"
#include "rc_rlstddef.h"

#ifdef __RLI_DEBUG_ACCESS__
#include <stdio.h>
#endif

#ifdef __ENABLE_LAN_IP_FILTER__

/*-----------------------------------------------------------------------*/

extern Boolean RLI_CUSTOM_DYNAMIC_IP_FILTER_ENABLED();
extern Boolean RLI_CUSTOM_DYNAMIC_IP_CHECK(ubyte4 ClientIpAddr, ubyte4 FutureParam1, ubyte4 FutureParam2);

/*-----------------------------------------------------------------------*/

extern Boolean ACCESS_ValidIpAddress(ubyte4 ClientIpAddr)
{
#ifdef __RLI_DEBUG_ACCESS__
	Boolean retBool;

	printf("ACCESS_ValidIpAddress: Checking client request (address=%d.%d.%d.%d).\n", (int)((ClientIpAddr >> 24) & 0xff), (int)((ClientIpAddr >> 16) & 0xff), (int)((ClientIpAddr >> 8) & 0xff), (int)(ClientIpAddr & 0xff));
#endif

    /* is ip filtering enabled? */
	if (FALSE != RLI_CUSTOM_DYNAMIC_IP_FILTER_ENABLED())
	{
#ifndef __RLI_DEBUG_ACCESS__
	    /* check if the ip address is valid */
		return RLI_CUSTOM_DYNAMIC_IP_CHECK(ClientIpAddr, 0, 0);
#else
		retBool = RLI_CUSTOM_DYNAMIC_IP_CHECK(ClientIpAddr, 0, 0);

		if (TRUE == retBool)
			printf("ACCESS_ValidIpAddress: Valid IP address.\n");
		else
			printf("ACCESS_ValidIpAddress: Invalid IP address --- block!\n");

		return retBool;
#endif
	}
	
#ifdef __RLI_DEBUG_ACCESS__
	printf("ACCESS_ValidIpAddress: filtering not enabled.\n");
#endif

	return TRUE;
}

#endif /* __ENABLE_LAN_IP_FILTER__ */


/**************************************************************
 *	New Access control Procedure
 */

extern Boolean RC_ACCESS_Allowed(Access nResourceAccess, Access nUserAccess)
{

#ifdef __RLI_ACCESS_LEVEL_MASK__

	/* See if user's access level is high enough */
#if (__RLI_ACCESS_LEVEL_MASK__ != 0x00000000)
    if ((nResourceAccess & __RLI_ACCESS_LEVEL_MASK__) > 
		(nUserAccess     & __RLI_ACCESS_LEVEL_MASK__) ) 
		return(FALSE);
#endif

	/* See if user's options overlap with the resource's (if resource options is not empty) */
#if (__RLI_ACCESS_OPTION_MASK__ != 0x00000000)
    if (( nResourceAccess &               __RLI_ACCESS_OPTION_MASK__)       &&
		((nResourceAccess & nUserAccess & __RLI_ACCESS_OPTION_MASK__) == 0) )
	   return(FALSE);
#endif

	return(TRUE );

#else

    return (nResourceAccess <= nUserAccess); 

#endif /* __RLI_ACCESS_LEVEL_MASK__ */
}

