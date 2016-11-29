/*  
 *  rcc_help.c
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

extern const EditKeys gEditKeys[];
extern const sbyte4   gEditKeysCount;

#ifndef HELP_NODE_HANDLER_FN
#define HELP_NODE_HANDLER_FN(c, n)      RCC_HELP_NodeHandler(c, n)
#endif

#ifndef HELP_PARAM_HANDLER_FN
#define HELP_PARAM_HANDLER_FN(c, n, p)  RCC_HELP_ParamHandler(c, n, p)
#endif

void    HELP_NODE_HANDLER_FN(cli_env *pCliEnv, cmdNode *pNode);
Boolean HELP_PARAM_HANDLER_FN(cli_env *pCliEnv, cmdNode *pNode, paramDefn *pParam);

#ifdef __RCC_HIDE_UNAMED_KEYWORD__
#define RCC_HELP_KEYWORD(pParam)    MPARM_HasKeyword(pParam)
#else
#define RCC_HELP_KEYWORD(pParam)    HAS_STRING(pParam->pKeyword)
#endif


#define  SHOW_NEXT_M(pCliEnv, pNode)  \
  (HELP_SET(pCliEnv, HELP_FLAG_NEXT) || FLAG_SET(pNode, kRCC_COMMAND_SHOW_NEXT))


RLSTATUS RCC_HELP_SetFeature(cli_env * pCliEnv, ubyte4 feature, sbyte * pText)
{
    helpInfo * pHelpInfo = MMISC_GetHelpPtr(pCliEnv);
    Boolean    setText   = TRUE;
    sbyte **   ppNewBuf  = NULL;
    sbyte *    pOldBuf   = NULL;

    if (NULL == pText)
        return OK;
               
    switch (feature)
    {
        case HELP_FLAG_LEADER:
            pHelpInfo->leader = *pText;
            setText  = FALSE;
            break;
        case HELP_FLAG_WIDTH:
            CONVERT_StrTo(pText, &pHelpInfo->width, kDTinteger);
            setText  = FALSE;
            break;
        case HELP_FLAG_ALLOC_NODE:
            pOldBuf  = pHelpInfo->pNode;
            ppNewBuf = &pHelpInfo->pNode;
            pOldBuf  = *ppNewBuf;
            break;
        case HELP_FLAG_ALLOC_DELIM:
            pOldBuf  = pHelpInfo->pDelimiter;
            ppNewBuf = &pHelpInfo->pDelimiter;
            pOldBuf  = *ppNewBuf;
            break;
        case HELP_FLAG_ALLOC_TITLE:
            pOldBuf  = pHelpInfo->pTitle;
            ppNewBuf = &pHelpInfo->pTitle;
            break;
        case HELP_FLAG_ALLOC_PREFIX:
            pOldBuf  = pHelpInfo->pPrefix;
            ppNewBuf = &pHelpInfo->pPrefix;
            break;
        case HELP_FLAG_ALLOC_QUOTE:
            pOldBuf  = pHelpInfo->pQuote;
            ppNewBuf = &pHelpInfo->pQuote;
            break;
        case HELP_FLAG_ALLOC_UNQUOTE:
            pOldBuf  = pHelpInfo->pUnquote;
            ppNewBuf = &pHelpInfo->pUnquote;
            break;
        default:
            return RCC_ERROR_THROW(RCC_ERROR);
    }

    if (setText)
    {
        if ((NULL != pOldBuf) && (FLAG_SET(pHelpInfo, feature)))
            RC_FREE(pOldBuf);

        if (0 == *pText)
        {
            CLEAR_FLAG(pHelpInfo, feature);
            *ppNewBuf = NULL;
            return OK;
        }

        if (NULL == (*ppNewBuf = RC_MALLOC(STRLEN(pText) + 1)))
            return RCC_ERROR_THROW(ERROR_MEMMGR_NO_MEMORY);

        STRCPY(*ppNewBuf, pText);
        SET_FLAG(pHelpInfo, feature);
    }

    return OK;
}


RL_STATIC Boolean
HELP_Title(cli_env * pCliEnv, sbyte * pTitle, Boolean printed)
{
    sbyte      titleChar;
    helpInfo * pHelpInfo = MMISC_GetHelpPtr(pCliEnv);

    if (NULL == pTitle)
        return printed;

    if (NULL == pHelpInfo->pTitle)
        return printed;

    titleChar = TOUPPER(*pTitle);

    RCC_EXT_WriteStrLine(pCliEnv, "");
    RCC_EXT_Write(pCliEnv, &titleChar, 1);
    RCC_EXT_WriteStr(pCliEnv, ++pTitle);
    RCC_EXT_WriteStrLine(pCliEnv, pHelpInfo->pTitle);

    return TRUE;
}

/*-----------------------------------------------------------------------*/

extern void RCC_HELP_PrintLabel(cli_env *pCliEnv, cmdNode *pNode, 
                                sbyte *pText, sbyte *pWrapper)
{
    helpInfo *  pHelpInfo = MMISC_GetHelpPtr(pCliEnv);
    sbyte4      width     = pHelpInfo->width;
    sbyte       leader    = pHelpInfo->leader;
    sbyte4      label     = 0;
    sbyte4      length    = 0; /* total length */

    if (NULL != pHelpInfo->pPrefix)
    {
       RCC_EXT_WriteStr(pCliEnv, pHelpInfo->pPrefix);
       length += STRLEN(pHelpInfo->pPrefix);
    }

    if (NULL != pWrapper)
    {
        RCC_EXT_Write(pCliEnv, pWrapper++, 1);
        length++;
    }

    if ( (NULL != pNode)                        && 
          FLAG_SET(pNode, kRCC_COMMAND_NO)      &&
          HELP_SET(pCliEnv, HELP_FLAG_SHOW_NO))
    {
        RCC_EXT_WriteStr(pCliEnv, "[");
        RCC_EXT_WriteStr(pCliEnv, kRCC_CMD_NO);
        RCC_EXT_WriteStr(pCliEnv, "] ");
        width -= sizeof(kRCC_CMD_NO) + 2;
    }

    if (NULL == pWrapper)
    {
        if (FLAG_SET(pHelpInfo, HELP_FLAG_FIXED_WIDTH))
            RCC_EXT_PrintString(pCliEnv, pText, width, leader);
        else
        {
            RCC_EXT_WriteStr(pCliEnv, pText);
            RCC_EXT_PrintString(pCliEnv, "", width, leader);
        }
    }
    else
    {
        label   = (NULL == pText) ? 0 : STRLEN(pText);
        length += label;

        if (FLAG_SET(pHelpInfo, HELP_FLAG_FIXED_WIDTH))
        {
            if (length > width)
                label -= length - width;
        }
        RCC_EXT_Write(pCliEnv, pText, label);

        if (FLAG_SET(pHelpInfo, HELP_FLAG_FIXED_WIDTH))
        {
            if (length < width)
            {
                RCC_EXT_Write(pCliEnv, pWrapper, 1);
            }
        }
        else
        {
            RCC_EXT_Write(pCliEnv, pWrapper, 1);
        }

        if (FLAG_SET(pHelpInfo, HELP_FLAG_FIXED_WIDTH))
            RCC_EXT_PrintString(pCliEnv, "", width - length, leader);
    }

    if (NULL != pHelpInfo->pDelimiter)
        RCC_EXT_WriteStr(pCliEnv, pHelpInfo->pDelimiter);

    /* if enabled, text will wrap at beginning of help text */
    HELP_INDENT_SET_M(pCliEnv);
}

/* default node help handler */

void RCC_HELP_NodeHandler(cli_env *pCliEnv, cmdNode *pNode)
{
    if (NULL != pNode->pKeyword)
        RCC_HELP_PrintLabel(pCliEnv, pNode, pNode->pKeyword, NULL);

    HELP_INDENT_SET_M(pCliEnv);

    /* the default help text */
 
    if (! NULL_STRING(pNode->pHelp))
        RCC_EXT_WriteStr(pCliEnv, pNode->pHelp);
    else
        /* or we do a generate command syntax */
        RCC_DB_ShowCommand(pCliEnv, pNode);
}

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean 
HELP_PrintHelp(cli_env *pCliEnv, cmdNode *pNode, 
               sbyte *query, Boolean printed)
{
    /* should never happen but just in case */
    if (NULL == pNode)
        return printed;

    if (! RC_ACCESS_Allowed(pNode->accessLvl, MMISC_GetAccess(pCliEnv)))
        return printed;

    if ( (NULL != query)            && 
         (NULL != pNode->pKeyword)  &&
         (0 != STRNCMP(pNode->pKeyword, query, STRLEN(query))))
        return printed;

    if ( HELP_SET(pCliEnv, HELP_FLAG_HIDE_BLANK)    &&
         NULL_STRING(pNode->pHelp)                  && 
         NULL_STRING(pNode->pHelpHandler) )
        return printed;

    if (! printed)
        RCC_EXT_WriteStrLine(pCliEnv, "");
            
    /* custom help handler overrides help text */

    if (NULL != pNode->pHelpHandler)
        pNode->pHelpHandler(pCliEnv, pNode, NULL);
    else
        HELP_NODE_HANDLER_FN(pCliEnv, pNode);

    HELP_INDENT_RESET_M(pCliEnv);
    RCC_EXT_WriteStrLine(pCliEnv, "");

    return TRUE;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void HELP_ShowHotKey(cli_env *pCliEnv, sbyte key)
{
    sbyte   keyText;

    if (0 < key && key <= 26)
    {
        keyText = CONTROL_KEY(key);
        RCC_EXT_WriteStr(pCliEnv, "Ctrl-");
        RCC_EXT_Write(pCliEnv, &keyText, 1);
    } 
    else
    {
        keyText = ESCAPE_KEY(key);
        RCC_EXT_WriteStr(pCliEnv, "Esc-");
        RCC_EXT_Write(pCliEnv, &keyText, 1);
    } 
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
HELP_PrintHelpLine(cli_env *pCliEnv, sbyte *helpText, ubyte keyCode)
{
    RCC_EXT_PrintString(pCliEnv, helpText, kRCC_HELP_KEYS_WIDTH, '.');
    HELP_ShowHotKey(pCliEnv, keyCode);
    RCC_EXT_WriteStrLine(pCliEnv, "");
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_HELP_EditHelp(cli_env *pCliEnv)
{
    sbyte4  index;

    RCC_EXT_WriteStrLine(pCliEnv, kRCC_MSG_KEYSTROKES); 

    for (index = 0; index < gEditKeysCount; index++)
    {
        HELP_PrintHelpLine(pCliEnv, 
                           gEditKeys[index].helpText, 
                           gEditKeys[index].key);
    }

    RCC_EXT_WriteStrLine(pCliEnv, "");
    return OK;
}

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean 
HELP_ChildHelp(cli_env *pCliEnv, cmdNode *pNode, sbyte *query, 
               Boolean global, Boolean printed)
{
#if 0
    sbyte4       idx      = 0;
    cmdNode     *pChild   = NULL;

    if (0 >= pNode->numChildren)
        return FALSE;

    pChild = pNode->pChildren;
    for (idx = 0; idx < pNode->numChildren; idx++, pChild++)
    {
        if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_NOPRINT))
            break;

        if (! MMISC_ValidGlobal(pChild, global))
            continue;
        
        printed |= HELP_PrintHelp(pCliEnv, pChild, query, printed);
    }

    return printed;
#else
    sbyte4       idx      = 0;
    cmdNode     *pChild   = NULL;
    helpInfo *  pHelpInfo = MMISC_GetHelpPtr(pCliEnv);
    sbyte4      width     = pHelpInfo->width;
    sbyte       leader    = pHelpInfo->leader;
    sbyte4      label     = 0;
    sbyte4      length    = 0; /* total length */
	int 		hasRequiredPara;

	if (OK != RCC_DB_ParseCmdString(pCliEnv))
		goto prt_help;
    
    /* match each token to a node or parameter */
    if (OK != RCC_DB_Parameterize(pCliEnv))
        goto prt_help;
	
	if (OK == RCC_DB_BuildParamList(pCliEnv, TRUE) && pNode->numHandlers )
	{
		cmdNode * pRoot = MMISC_GetCurrRoot( pCliEnv );

		if ( pRoot->pKeyword == NULL || strcmp( pRoot->pKeyword, pNode->pKeyword ) != 0 )
		{
			if (! printed)
        		RCC_EXT_WriteStrLine(pCliEnv, "");

			RCC_EXT_WriteStr( pCliEnv, " " );

			if (FLAG_SET(pHelpInfo, HELP_FLAG_FIXED_WIDTH))
	            RCC_EXT_PrintString(pCliEnv, "<CR>", width, leader);
	        else
	        {
	            RCC_EXT_WriteStr(pCliEnv, "<CR>" );
	            RCC_EXT_PrintString(pCliEnv, "", width, leader);
	        }

			if (NULL != pHelpInfo->pDelimiter)
	        	RCC_EXT_WriteStr(pCliEnv, pHelpInfo->pDelimiter);

			RCC_EXT_WriteStrLine( pCliEnv, "Press Enter key to exec this command" );

			printed = TRUE;
		}
	}

prt_help:
	
    pChild = pNode->pChildren;
    for (idx = 0; idx < pNode->numChildren; idx++, pChild++)
    {
        if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_NOPRINT))
            break;

        if (! MMISC_ValidGlobal(pChild, global))
            continue;
        
        printed |= HELP_PrintHelp(pCliEnv, pChild, query, printed);
    }

    return printed;

#endif
}

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean
HELP_PrintParamHelp(cli_env *pCliEnv, cmdNode *pNode, 
                    paramDefn *pParam, Boolean printed)
{
    sbyte       *pItem = NULL;
    sbyte       *pBrackets;
    HelpHandler *pHelpHandler;
        
    if (! RCC_DB_ParamAccess(pCliEnv, pParam))
          return FALSE;

    if (HELP_SET(pCliEnv, HELP_FLAG_HIDE_BLANK))
    {
        if (NULL == pParam->pParamInfo)
            return FALSE;

        if ( (NULL == pParam->pParamInfo->pHelpHandler) &&
              NULL_STRING(pParam->pParamInfo->pHelpStr) )
                return FALSE;
    }
    
    if (! printed)
        RCC_EXT_WriteStrLine(pCliEnv, "");

    if (RCC_HELP_KEYWORD(pParam))
    {
        pItem = pParam->pKeyword;
    }
    else
        pItem = CONVERT_GetDTProtoType(pParam->type);

    pBrackets = MPARM_HasKeyword(pParam) ? NULL : kRCC_PARAM_BRACKETS;

    if (NULL != pParam->pParamInfo)
    {
        /* custom help handler overrides help text */

        pHelpHandler = (HelpHandler *) pParam->pParamInfo->pHelpHandler;
        if (NULL != pHelpHandler)
            pHelpHandler(pCliEnv, pNode, pParam);
        else 
        {

            RCC_HELP_PrintLabel(pCliEnv, NULL, pItem, pBrackets);

            if (NULL != pParam->pParamInfo->pHelpStr)
                RCC_EXT_WriteStr(pCliEnv, pParam->pParamInfo->pHelpStr);
            else
            {
                RCC_EXT_WriteStr(pCliEnv, kRCC_PARAM_LEFT_BRACKET);
                RCC_DB_PrintType(pCliEnv, pParam);
                RCC_EXT_WriteStr(pCliEnv, kRCC_PARAM_RIGHT_BRACKET);
            }
        }
    }
    else
    {
                
        RCC_HELP_PrintLabel(pCliEnv, NULL, pItem, pBrackets);

        pItem = CONVERT_GetDTName(pParam->type);
        RCC_EXT_WriteStr(pCliEnv, pItem);
    }

    return TRUE;
}

/*-----------------------------------------------------------------------*/


RL_STATIC Boolean
HELP_ParamHelp(cli_env *pCliEnv, cmdNode *pNode, Boolean printed, Boolean *pRemaining)
{
    handlerDefn *pHandler;
    paramDefn   *pParam    = pNode->pParams;
    sbyte4       start     = 0;
    sbyte4       end       = pNode->numParams;
    sbyte4       index     = 0;
    Boolean      ordered;
	Boolean      orderOK;
    Boolean      printOK   = TRUE;
    Boolean      allParams = HELP_SET(pCliEnv, HELP_FLAG_PARAMS); 

    ordered  = RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_SOME) || 
               RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_FULL);

    *pRemaining = FALSE;

    /* if we're supposed to be ordered but don't have a handle,
       then we're still unordered */

    pHandler  = RCC_DB_CurrentHandler(pCliEnv);
    orderOK   = ordered && (NULL != pHandler) && ! allParams;

    if (orderOK)
        end = HANDLER_PARAM_COUNT(pHandler);

    for (index = start; index < end; index++, pParam++)
    {
        if (orderOK)
        {
            pParam = RCC_DB_OrderedParam(pCliEnv, index);

            if (NULL == pParam)
                continue;
        }
		else
		{
			/* have parameter ordering but don't know which handle is valid */
			if (ordered && SHOW_NEXT_M(pCliEnv, pNode))
			{
                    /* unnamed ordering should ignore keyword params) */
				if (! (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_SOME) && MPARM_HasKeyword(pParam)))
				{
					if (!allParams && !RCC_DB_ParamPossible(pCliEnv, pNode, pParam))
						continue;
				}
			}
		}

        if (! allParams && RCC_DB_IsAssigned(pCliEnv, pParam))
            continue;

        if (printOK && ! HELP_PrintParamHelp(pCliEnv, pNode, pParam, printed))
            continue;

        printed = TRUE;
        HELP_INDENT_RESET_M(pCliEnv);        
        RCC_EXT_WriteStrLine(pCliEnv, "");

        if (RCC_DB_ParamRequired(pCliEnv, pParam))
        {
            *pRemaining = TRUE;

            if (! printOK)
                break;

            if ( orderOK && SHOW_NEXT_M(pCliEnv, pNode))
                printOK = FALSE;
        }
    }

    return printed;
}

/*-----------------------------------------------------------------------*/

extern void
RCC_HELP_CustomHelpString(cli_env *pCliEnv, cmdNode *pNode, paramDefn *pParam)
{
    helpInfo *  pHelpInfo = MMISC_GetHelpPtr(pCliEnv);
    sbyte  *pLabel   = NULL;
    sbyte  *pInfo    = NULL;
    sbyte4  labelLen = 0;
    sbyte   leader   = pHelpInfo->leader;

    if ((NULL == pCliEnv) || (NULL == pParam) || (NULL == pParam->pParamInfo))
        return;

    pLabel = pInfo = pParam->pParamInfo->pHelpStr;

    while ((0 != *pInfo) && ('\t' != *pInfo))
    {
        labelLen++;
        pInfo++;
    }

    if (NULL != pHelpInfo->pPrefix)
       RCC_EXT_WriteStr(pCliEnv, pHelpInfo->pPrefix);

    RCC_EXT_Write(pCliEnv, pLabel, labelLen);
    RCC_EXT_PrintString(pCliEnv, "", kRCC_HELP_WIDTH - labelLen, leader);

    if (NULL != pHelpInfo->pDelimiter)
       RCC_EXT_WriteStr(pCliEnv, pHelpInfo->pDelimiter);

    RCC_EXT_WriteStr(pCliEnv, ++pInfo);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_HELP_ErrorHelp(cli_env *pCliEnv, cmdNode *pNode)
{
    helpInfo *  pHelpInfo   = MMISC_GetHelpPtr(pCliEnv);
    Boolean     blankLine   = (NULL != pNode);
    Boolean     printed     = FALSE;
    Boolean     metaHelp    = FALSE;

    if (NULL == pNode)
        pNode = RCC_DB_GetCommandNode(pCliEnv);

    if (NULL != pNode)
    {
        if (NULL != pHelpInfo->pQuote)
            RCC_EXT_WriteStr(pCliEnv, pHelpInfo->pQuote);

        RCC_DB_ShowCommand(pCliEnv, pNode);

        if (NULL != pHelpInfo->pUnquote)
            RCC_EXT_WriteStr(pCliEnv, pHelpInfo->pUnquote);

        RCC_EXT_WriteStrLine(pCliEnv, "");
        return OK;
    }

    if (FLAG_SET(pNode, kRCC_COMMAND_META))
    {
        pNode = RCC_DB_GetParentNode(pCliEnv, pNode);
        if (FLAG_SET(pNode, kRCC_COMMAND_META))
            metaHelp = TRUE;
        else
            blankLine = TRUE;
    }

    if (NULL == pNode)
        return RCC_ERROR_THROW(ERROR_RCC_NO_HELP);

    printed = HELP_Title(pCliEnv, pNode->pKeyword, printed);

    RCC_EXT_WriteStrLine(pCliEnv, "");

    if (NULL != pHelpInfo->pQuote)
        RCC_EXT_WriteStr(pCliEnv, pHelpInfo->pQuote);

    RCC_EXT_WriteStr(pCliEnv, pNode->pKeyword);

    if (NULL != pHelpInfo->pUnquote)
        RCC_EXT_WriteStr(pCliEnv, pHelpInfo->pUnquote);

    if (! NULL_STRING(pNode->pHelp))
        RCC_EXT_WriteStr(pCliEnv, pNode->pHelp);
    else
        RCC_DB_ShowCommand(pCliEnv, pNode);

    RCC_EXT_WriteStrLine(pCliEnv, "");

    if (printed)
        return OK;

    return RCC_ERROR_THROW(ERROR_RCC_NO_HELP);
}

/*-----------------------------------------------------------------------*/

/* no node keyword implies root node */

RL_STATIC Boolean
HELP_ThisHelp(cli_env *pCliEnv, cmdNode *pNode, Boolean printed)
{
    helpInfo * pHelpInfo = MMISC_GetHelpPtr(pCliEnv);
    sbyte4     cursorPos;
    sbyte4     diff;

    /* should never happen but just in case */

    if (NULL == pNode)
        return printed;

    if (NULL == pNode->pKeyword)
        return printed;

    if (! RC_ACCESS_Allowed(pNode->accessLvl, MMISC_GetAccess(pCliEnv)))
        return printed;

    if ( HELP_SET(pCliEnv, HELP_FLAG_HIDE_BLANK)    &&
         NULL_STRING(pNode->pHelp)                  && 
         NULL_STRING(pNode->pHelpHandler) )
        return printed;

    if (HELP_CLEAR(pCliEnv, HELP_FLAG_SAME_LINE))
    {
        if (! printed)
            RCC_EXT_WriteStrLine(pCliEnv, "");

        printed = TRUE;

        if (NULL != pHelpInfo->pNode)
            RCC_EXT_WriteStr(pCliEnv, pHelpInfo->pNode);
    }
                
    /* custom help handler overrides help text */

    if (NULL != pNode->pHelpHandler)
        pNode->pHelpHandler(pCliEnv, pNode, NULL);
    else
    {
        if (HELP_CLEAR(pCliEnv, HELP_FLAG_SAME_LINE))
            RCC_HELP_PrintLabel(pCliEnv, pNode, pNode->pKeyword, NULL);
        else
        {
            /* print help on same line as command */

            cursorPos = MEDIT_GetPromptLen(pCliEnv)  /* x offset */
                      + MEDIT_GetCursor(pCliEnv);

            diff = pHelpInfo->width - cursorPos + 1;

            if (diff > 0)
                RCC_EXT_PrintString(pCliEnv, "", diff, (char) pHelpInfo->leader);

            RCC_EXT_WriteStr(pCliEnv, pHelpInfo->pNode);
        }

        HELP_INDENT_SET_M(pCliEnv);

        /* the default help text */
 
        if (! NULL_STRING(pNode->pHelp))
            RCC_EXT_WriteStr(pCliEnv, pNode->pHelp);
        else
            /* or we do a generate command syntax */
            RCC_DB_ShowCommand(pCliEnv, pNode);
    }

    HELP_INDENT_RESET_M(pCliEnv);
    RCC_EXT_WriteStrLine(pCliEnv, "");

    return TRUE;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_HELP_RetrieveHelp(cli_env *pCliEnv, cmdNode *pNode, Boolean immediate)
{
    RLSTATUS    status      = OK;
    Boolean     blankLine   = (NULL != pNode);
    Boolean     printed     = FALSE;
    Boolean     metaHelp    = FALSE;
    Boolean     skip        = FALSE;
    Boolean     remaining   = FALSE;
	Boolean		thisHelp;
    paramDefn  *pParam      = NULL;
    cmdNode    *pRoot       = RCC_DB_GetRootNode();
    sbyte      *pQuery      = NULL;
    sbyte       priorChar;

    if (NULL == pNode)
        pNode = MMISC_GetCmdNode(pCliEnv);

    skip =  immediate && HELP_SET(pCliEnv, HELP_FLAG_SKIP_NODE);

    /* invoked by help command */
    if (! immediate)
    {
        printed = TRUE; /* already have newline from entering help command */
        pQuery  = RCC_DB_LastToken(pCliEnv);
    }
    else
    {
        /* invoked with "?" command */

        /* IOS behaviour is ? with no prior space prints command only */
        priorChar = RCC_EXT_GetPriorChar(pCliEnv);
        if ((' ' != priorChar) && (kCHAR_NULL != priorChar))
        {
            RCC_DB_Possibilities(pCliEnv);
            RCC_EXT_EnablePrint(pCliEnv, TRUE);
            RCC_DB_ShowInput(pCliEnv);
            return OK;
        }

        if (ERROR_GENERAL_NO_DATA == RCC_DB_ParseCmdString(pCliEnv))
            blankLine = TRUE;
        else
        {
            status = RCC_DB_Parameterize(pCliEnv);

            switch (status)
            {
            case ERROR_RCC_INVALID_PARAM:
            case ERROR_RCC_EXTRA_PARAMS:
            case ERROR_RCC_BAD_COMMAND:
            case ERROR_RCC_INVALID_NO:
            case ERROR_RCC_MISSING_NO:
            case ERROR_RCC_NO_HANDLER:
            case ERROR_CONVERSION_INCORRECT_TYPE:
            case ERROR_GENERAL_OUT_OF_RANGE:
                return status;
            case ERROR_RCC_AMBIGUOUS_COMMAND:
            case ERROR_GENERAL_DATA_AMBIG:
                pQuery = RCC_DB_LastToken(pCliEnv);
                break;
            case ERROR_RCC_NO_PARAM_DATA:
                pQuery = RCC_DB_LastToken(pCliEnv);
                pParam = RCC_DB_IncompleteParam(pCliEnv);
                break;
            case STATUS_RCC_NO_ERROR:
                break;
            case OK:
            default:
                break;
            }                
 
            status = OK;
            RCC_DB_BuildParamList(pCliEnv, FALSE); 
        }
        pNode = RCC_DB_GetCommandNode(pCliEnv);
    }

    if ((! immediate) && FLAG_SET(pNode, kRCC_COMMAND_META))
    {
        if ((NULL != pQuery)            && 
            (NULL != pNode->pKeyword)   &&
            (0 == COMPARE(pQuery, pNode->pKeyword)))
        pQuery = NULL;

        pNode = RCC_DB_GetParentNode(pCliEnv, pNode);
        if (FLAG_SET(pNode, kRCC_COMMAND_META))
            metaHelp = TRUE;
        else
            blankLine = TRUE;
    }

    if (NULL == pNode)
    {
        if (immediate && printed)
        {
            RCC_EXT_EnablePrint(pCliEnv, TRUE);
            RCC_DB_ShowInput(pCliEnv);
        }      
        return RCC_ERROR_THROW(ERROR_RCC_NO_HELP);
    }

    if ((NULL != pQuery)            && 
        (NULL != pNode->pKeyword)   &&
        (0 == COMPARE(pQuery, pNode->pKeyword)))
        pQuery = NULL;

    printed |= HELP_Title(pCliEnv, pNode->pKeyword, printed);

    /* parameter */

    pParam = RCC_DB_IncompleteParam(pCliEnv);

    if (NULL != pParam)
    {
        printed |= HELP_PrintParamHelp(pCliEnv, pNode, pParam, printed);
        HELP_INDENT_RESET_M(pCliEnv);
        RCC_EXT_WriteStrLine(pCliEnv, "");
        if (immediate && printed)
        {
            RCC_EXT_EnablePrint(pCliEnv, TRUE);
            RCC_DB_ShowInput(pCliEnv);
        }      
        return OK;
    }

	/* show help for the node in question */

    if (immediate)
		thisHelp = HELP_CLEAR(pCliEnv, HELP_FLAG_SKIP_NODE);
	else
		thisHelp = HELP_SET(pCliEnv, HELP_FLAG_THIS) ||
				   (0 == RCC_DB_GetParamCount(pCliEnv, pNode));

	//if (thisHelp)
	if (thisHelp && ( MMISC_GetCmdNode( pCliEnv ) != NULL && MMISC_GetCmdNode( pCliEnv ) != pNode ) ) /* hejianguo */
		printed = HELP_ThisHelp(pCliEnv, pNode, printed);

    /* if last item is a parameter, show help for it */

    if (NULL != pParam)
    {
        if (HELP_PrintParamHelp(pCliEnv, pNode, pParam, printed))
        {
            RCC_EXT_WriteStrLine(pCliEnv, "");
            printed = TRUE;
        }
    }

    /* 
     * we have some data or metadata, 
     * and there's no incomplete parameter
     * so show the parameters for this node
     */

    if ((!blankLine || metaHelp) && (NULL == pParam))
        printed |= HELP_ParamHelp(pCliEnv, pNode, printed, &remaining);
    
    /* don't show child nodes unless we don't care or no required params left */
    if (SHOW_NEXT_M(pCliEnv, pNode))
    {   
        if (! remaining)    
            printed |= HELP_ChildHelp(pCliEnv, pNode, pQuery, FALSE, printed);
    }
    else
        printed |= HELP_ChildHelp(pCliEnv, pNode, pQuery, FALSE, printed);

    /* global matches */

    if ((pNode == pRoot) && (NULL != pQuery) && (!printed || !immediate))
        printed = HELP_ChildHelp(pCliEnv, pRoot, pQuery, TRUE, printed);

    if ( immediate                                  &&
         HELP_SET(pCliEnv, HELP_FLAG_EXECUTABLE)    &&
         (0 < pNode->numHandlers)                   && 
         (OK == RCC_DB_CommandComplete(pCliEnv)) )
    {
        if (! printed)
            RCC_EXT_WriteStrLine(pCliEnv, "");

        RCC_EXT_WriteStrLine(pCliEnv, " <cr>");
        printed = TRUE;
    }
        
    HELP_INDENT_RESET_M(pCliEnv);

    if (immediate && printed)
    {
        RCC_EXT_EnablePrint(pCliEnv, TRUE);
        RCC_DB_ShowInput(pCliEnv);
    }

    return status;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_HELP_Globals(cli_env *pCliEnv)
{
    cmdNode    *pNode       = RCC_DB_GetRootNode();
            
    HELP_ChildHelp(pCliEnv, pNode, NULL, TRUE, FALSE);
        
    return OK;
}


#endif /* __RCC_ENABLED__ */
