/*  
 *  rcc_system.c
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

#include "rc.h"

#ifdef __RCC_ENABLED__

#include "rcc.h"

const EditKeys gEditKeys[] = 
{
    {kKEY_DELETE_CHAR,       "Delete current character"},
    {kKEY_DELETE_FROM_START, "Delete text up to cursor"},
    {kKEY_DELETE_TO_END,     "Delete from cursor to end of line"},
    {kKEY_LINE_START,        "Move to beginning of line"},
    {kKEY_LINE_END,          "Move to end of line"},
    {kKEY_MOVE_UP,           "Get prior command from history"},
    {kKEY_MOVE_DOWN,         "Get next command from history"},
    {kKEY_MOVE_LEFT,         "Move cursor left"},
    {kKEY_MOVE_RIGHT,        "Move cursor right"},
    {kKEY_WORD_PREV,         "Move back one word"},
    {kKEY_WORD_NEXT,         "Move forward one word"},
    {kKEY_UPPERCASE,         "Convert rest of word to uppercase"},
    {kKEY_LOWERCASE,         "Convert rest of word to lowercase"},
    {kKEY_DELETE_WORD_END,   "Delete remainder of word"},
    {kKEY_DELETE_WORD_START, "Delete word up to cursor"},
    {kKEY_TRANSPOSE,         "Transpose current and previous character"},
    {kKEY_END_OF_ENTRY,      "Enter command and return to root prompt"},
    {kKEY_REFRESH_DISPLAY,   "Refresh input line"}
};

#define TASK_BOTH_K      TASK_RETRY_K | TASK_NOMSG_K

ErrorTable gErrorTable[] =
{
    {TASK_NONE_K,  ERROR_GENERAL_INVALID_PATH,     kMSG_ERROR_GENERAL_INVALID_PATH},
    {TASK_NONE_K,  ERROR_RCC_INVALID_HISTORY,       kMSG_ERROR_RCC_INVALID_HISTORY},
    {TASK_NONE_K,  ERROR_GENERAL_NOT_FOUND,         kMSG_ERROR_GENERAL_NOT_FOUND},
    {TASK_NONE_K,  ERROR_RCC_HANDLER_EXEC_FAILED,   NULL},
    {TASK_NONE_K,  ERROR_RCC_NO_ERROR_MSG,          NULL},     
    {TASK_NONE_K,  ERROR_GENERAL_ACCESS_DENIED,     kMSG_ERROR_GENERAL_ACCESS_DENIED},
    {TASK_NONE_K,  ERROR_RCC_BAD_COMMAND,           kMSG_ERROR_RCC_BAD_COMMAND},
    {TASK_BOTH_K,  ERROR_RCC_INVALID_PARAM,         kMSG_ERROR_RCC_INVALID_PARAM},
    {TASK_BOTH_K,  ERROR_GENERAL_ILLEGAL_VALUE,     kMSG_ERROR_RCC_INVALID_PARAM},
    {TASK_BOTH_K,  ERROR_CONVERSION_INCORRECT_TYPE, kMSG_ERROR_RCC_INVALID_PARAM},
    {TASK_BOTH_K,  ERROR_CONVERSION_OVERFLOW,       kMSG_ERROR_RCC_INVALID_PARAM},
    {TASK_BOTH_K,  ERROR_RCC_INVALID_VALUE,         kMSG_ERROR_RCC_INVALID_PARAM},
    {TASK_RETRY_K, ERROR_GENERAL_DATA_AMBIG,        kMSG_ERROR_GENERAL_DATA_AMBIG},
    {TASK_RETRY_K, ERROR_RCC_AMBIGUOUS_PARAM,       kMSG_ERROR_RCC_AMBIGUOUS_PARAM},
    {TASK_RETRY_K, ERROR_RCC_NO_PARAM_DATA,         kMSG_ERROR_RCC_NO_PARAM_DATA},
    {TASK_NONE_K,  ERROR_GENERAL_INVALID_RAPIDMARK, kMSG_ERROR_INVALID_RAPIDMARK},
    {TASK_RETRY_K, ERROR_RCC_MISSING_PARAM,         kMSG_ERROR_RCC_MISSING_PARAM},
    {TASK_NONE_K,  ERROR_RCC_NO_HANDLER,            kMSG_ERROR_RCC_NO_HANDLER},
    {TASK_NONE_K,  ERROR_RCC_INVALID_USER,          kMSG_ERROR_RCC_INVALID_USER},
    {TASK_NONE_K,  ERROR_CONVERSION_TOO_LONG,       kMSG_ERROR_CONVERSION_TOO_LONG},
    {TASK_NONE_K,  ERROR_RCC_NO_HELP,               kMSG_ERROR_RCC_NO_HELP},
    {TASK_RETRY_K, ERROR_GENERAL_OUT_OF_RANGE,      kMSG_ERROR_GENERAL_OUT_OF_RANGE},
    {TASK_NONE_K,  ERROR_GENERAL_FILE_NOT_FOUND,    kMSG_ERROR_GENERAL_FILE_NOT_FOUND},
    {TASK_RETRY_K, ERROR_RCC_AMBIGUOUS_COMMAND,     kMSG_ERROR_RCC_AMBIGUOUS_COMMAND},
    {TASK_RETRY_K, ERROR_RCC_INVALID_NO,            kMSG_ERROR_RCC_INVALID_NO},
    {TASK_BOTH_K,  ERROR_RCC_EXTRA_PARAMS,          kMSG_ERROR_RCC_EXTRA_PARAMS},
    {TASK_RETRY_K, ERROR_RCC_KEYWORD_AS_PARAM,      kMSG_ERROR_RCC_KEYWORD_AS_PARAM},
    {TASK_RETRY_K, ERROR_RCC_CUSTOM_RETRY,          kMSG_ERROR_RCC_CUSTOM_RETRY},
    {TASK_NONE_K,  ERROR_RCC_CUSTOM_NO_RETRY,       kMSG_ERROR_RCC_CUSTOM_NO_RETRY},
    {TASK_RETRY_K, ERROR_RCC_NO_CHAIN,              kMSG_ERROR_RCC_NO_CHAIN},
    {TASK_BOTH_K,  ERROR_RCC_MISSING_NO,            kMSG_ERROR_RCC_MISSING_NO},
    {TASK_NONE_K,  ERROR_RCC_MAX_ALIASES,           kMSG_ERROR_RCC_MAX_ALIASES}
};

const sbyte4 gEditKeysCount   = ARRAY_SIZE(gEditKeys);
const sbyte4 gErrorTableCount = ARRAY_SIZE(gErrorTable);

#endif /* __RCC_ENABLED__ */
