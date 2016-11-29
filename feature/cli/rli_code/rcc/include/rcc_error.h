/*  
 *  rcc_error.h
 *
 *  This is a part of the RapidControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
 *  All rights reserved.
 *
 *  RapidControl, RapidControl for Web, RapidControl Backplane,
 *  RapidControl for Applets, MIBway, RapidControl Protocol, and
 *  RapidMark are trademarks of Rapid Logic, Inc.  All rights reserved.
 *
 */

#ifndef __RCC_ERROR_H__
#define __RCC_ERROR_H__

#define RCC_ERROR    0xA000
#define RCC_STATUS   0xA100

/* errors pertinent to the RCC */
#define		ERROR_RCC					  (-10000)
#define		ERROR_RCC_FAILURE			  (ERROR_RCC - 1)
#define		ERROR_RCC_RETRY				  (ERROR_RCC - 2)
#define     ERROR_RCC_ABORT               (ERROR_RCC - 3)
#define		ERROR_RCC_TIMEOUT			  (ERROR_RCC - 4)
#define		ERROR_RCC_READ				  (ERROR_RCC - 5)
#define		ERROR_RCC_WRITE				  (ERROR_RCC - 6)
#define	    ERROR_RCC_INVALID_PARAM	      (ERROR_RCC - 7)
#define     ERROR_RCC_INVALID_VALUE       (ERROR_RCC - 8)
#define     ERROR_RCC_INVALID_NO          (ERROR_RCC - 9)
#define     ERROR_RCC_EXTRA_PARAMS        (ERROR_RCC - 10)
#define     ERROR_RCC_NULL_ENV            (ERROR_RCC - 11)
#define     ERROR_RCC_NULL_PARAM_LIST     (ERROR_RCC - 12)
#define     ERROR_RCC_LOGIN_FAIL          (ERROR_RCC - 13)
#define     ERROR_RCC_INVALID_HISTORY     (ERROR_RCC - 14)
#define     ERROR_RCC_WRONG_MODE          (ERROR_RCC - 15)
#define     ERROR_RCC_INVALID_SESSION     (ERROR_RCC - 16)
#define     ERROR_RCC_KEYWORD_AS_PARAM    (ERROR_RCC - 17)
#define     ERROR_RCC_AMBIGUOUS_PARAM     (ERROR_RCC - 18)
#define     ERROR_RCC_HANDLER_EXEC_FAILED (ERROR_RCC - 19)
#define	    ERROR_RCC_MISSING_PARAM	      (ERROR_RCC - 20)
#define	    ERROR_RCC_NO_PARAM_DATA	      (ERROR_RCC - 21)
#define	    ERROR_RCC_NO_INPUT_DATA       (ERROR_RCC - 22)
#define	    ERROR_RCC_INVALID_OPTION      (ERROR_RCC - 23)
#define     ERROR_RCC_NO_ERROR_MSG        (ERROR_RCC - 24)
#define     ERROR_RCC_BAD_COMMAND         (ERROR_RCC - 25)
#define     ERROR_RCC_NO_HANDLER          (ERROR_RCC - 26)
#define     ERROR_RCC_INVALID_USER        (ERROR_RCC - 27)
#define     ERROR_RCC_AMBIGUOUS_COMMAND   (ERROR_RCC - 28)
#define     ERROR_RCC_NO_HELP             (ERROR_RCC - 29)
#define     ERROR_RCC_CUSTOM_RETRY        (ERROR_RCC - 30)
#define     ERROR_RCC_CUSTOM_NO_RETRY     (ERROR_RCC - 31)
#define     ERROR_RCC_NO_CHAIN            (ERROR_RCC - 32)
#define     ERROR_RCC_MISSING_NO          (ERROR_RCC - 33)
#define     ERROR_RCC_MAX_ALIASES         (ERROR_RCC - 34)

/* not really errors, control commands */
#define	    STATUS_RCC_LOGOUT    	      (RCC_STATUS - 1)
#define     STATUS_RCC_EXIT               (RCC_STATUS - 2)
#define     STATUS_RCC_EXIT_ALL           (RCC_STATUS - 3)
#define	    STATUS_RCC_INTERNAL_COMMAND   (RCC_STATUS - 4)
#define	    STATUS_RCC_NOT_INTERNAL       (RCC_STATUS - 5)
#define     STATUS_RCC_HISTORY_EXEC       (RCC_STATUS - 6)
#define     STATUS_RCC_HISTORY_EDIT       (RCC_STATUS - 7)
#define     STATUS_RCC_NO_INTERMEDIATE    (RCC_STATUS - 8)
#define     STATUS_RCC_NO_ERROR           (RCC_STATUS - 9)
#define     STATUS_RCC_EXIT_TO_ROOT       (RCC_STATUS - 10)
#define     STATUS_RCC_KILL               (RCC_STATUS - 11)
#define     STATUS_RCC_PARAM_NODE         (RCC_STATUS - 12)
#define     STATUS_RCC_NO_PROMPT          (RCC_STATUS - 13)
#define     STATUS_RCC_NO_RAPIDMARKS      (RCC_STATUS - 14)
#define     STATUS_RCC_TOKEN_MATCH        (RCC_STATUS - 15)
#define     STATUS_RCC_CANCEL             (RCC_STATUS - 16)  

#endif /* __RCC_ERROR_H__ */
