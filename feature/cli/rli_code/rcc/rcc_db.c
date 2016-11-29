/*  
 *  rcc_db.c
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

#ifdef __SNMP_API_ENABLED__
#include "rcm_mibway.h"
#endif

#ifdef __ANSI_FILE_MANAGER_ENABLED__
#include "rc_ansifs.h"
#endif

#include <string.h>

#ifdef  __RCC_PARAMETER_ORDER_NONE__
# define DB_OPTIONAL_REQUIRED_FN DB_OptionalRequiredUnordered
# define DB_REQUIRED_FN          DB_RequiredUnordered
#else
# define DB_OPTIONAL_REQUIRED_FN DB_OptionalRequiredOrdered
# define DB_REQUIRED_FN          DB_RequiredOrdered
#endif

#ifdef __RCC_NO_EXEC__
#define RCC_EXEC_FN(pFn, pC, pP, pO)    OK
#else
#define RCC_EXEC_FN(pFn, pC, pP, pO)    pFn(pC, pP, pO)
#endif

#define kRCC_PFLAG_HAVE_COMMAND 0x0002
#define kRCC_PFLAG_NO_GLOBAL    0x0004
#define kRCC_PFLAG_NEW_LINE     0x0008
#define kRCC_PFLAG_AMBIGUOUS    0x0010
#define kRCC_PFLAG_META_COMMAND 0x0020
#define kRCC_PFLAG_IS_META      0x0040
#define kRCC_PFLAG_NO_REPEAT    0x0080
#define kRCC_PFLAG_IS_NO_CMD    0x0100
#define kRCC_PFLAG_NO_CHAIN     0x0200

#define kTOKEN_QUOTED           0x0001

#define kPARAM_INDEX_UNKNOWN    -1

#define CHAR_FORMAT "%-20s: \'%c\'"
#define TEXT_FORMAT "%-20s: %s"
#define NUMB_FORMAT "%-20s: %d"

#define HELP_SETTING(pCliEnv, x)    (HELP_SET(pCliEnv, x) ? "On" : "Off")
#define NULL_CHECK(x)               NULL == x ? "(empty)" : x

#define WRITE_CHAR(pEnv, pBuf, label, data)     \
    sprintf(pBuf, CHAR_FORMAT, label, data);    \
    RCC_EXT_WriteStrLine(pEnv, pBuf)

#define WRITE_TEXT(pEnv, pBuf, label, data)               \
    sprintf(pBuf, TEXT_FORMAT, label, NULL_CHECK(data));  \
    RCC_EXT_WriteStrLine(pEnv, pBuf)

#define WRITE_NUMB(pEnv, pBuf, label, data)               \
    sprintf(pBuf, NUMB_FORMAT, label, data);              \
    RCC_EXT_WriteStrLine(pEnv, pBuf);

#define WRITE_HELP(pEnv, pBuf, label, flag)                         \
    sprintf(pBuf, TEXT_FORMAT, label, HELP_SETTING(pCliEnv, flag)); \
    RCC_EXT_WriteStrLine(pEnv, pBuf);

/* keep global params separate from local params */

#define GLOBAL_CONFLICT_M(x, y)             \
    ((NULL == x) || (NULL == y)) ? FALSE :  \
    (FLAG_SET(x, kRCC_COMMAND_GLOBAL) != FLAG_SET(y, kRCC_COMMAND_GLOBAL))

#define EXTENDED_PARAM_M(x) \
    (FLAG_SET(x, kRCC_COMMAND_EXTEND) || FLAG_SET(x, kRCC_COMMAND_GLOBAL))

#define DB_ERROR(x)     ((OK != x) && (STATUS_RCC_TOKEN_MATCH != x))

typedef struct cliTask
{
    sbyte4              count;
    handlerDefn        *pHandler;
    OS_SPECIFIC_MUTEX   mMutex;
} cliTask;


#ifdef __RCC_DEBUG__

ubyte4  gDebugFlags = 0;

Boolean debugFeature (ubyte4 feature)
{
    return gDebugFlags & feature;
}

void debugFeatureSet (ubyte4 feature)
{
    gDebugFlags |= feature;
}

void debugFeatureClear (ubyte4 feature)
{
    gDebugFlags &= ~feature;
}

void debugExecShow (cmdNode * pNode)
{
    if (gDebugFlags & kRCC_DEBUG_SHOWEXEC)
        printf("   EXEC: %s\n", pNode->pKeyword);
}

#endif /* __RCC_DEBUG__ */


cliTask            mTaskStack[kRCC_TASK_STACK_SIZE];
OS_SPECIFIC_MUTEX  mTaskMutex;

#ifdef __RCC_DEBUG__
#define DEBUG_PARAM_LIST(pCliEnv)
#else
#define DEBUG_PARAM_LIST(pCliEnv)
#endif

#define BLANK_TEXT(x)               (NULL != x ? x : " *Blank* ")
#define kERROR_HANDLED              -1
#define PARAMDESCR_ID               DB_ParamID
#define PARAM_ID(x)                 x->id

#define IS_NO_HANDLER(handler)      FLAG_SET(handler, kRCC_HANDLER_NO)
#define INVALID_NO_NO(n, p, h)      (IS_NO_COMMAND(p) != IS_NO_HANDLER(h))
#define INVALID_NO_OK(n, p, h)      ((1 < n->numHandlers) && (IS_NO_COMMAND(p) != IS_NO_HANDLER(h)))

#ifdef __NO_MEANS_NO__ 
#define INVALID_NO      INVALID_NO_NO
#else
#define INVALID_NO      INVALID_NO_OK
#endif

#define ENUMLIST(x)              ((kDTenum == x) || (kDTlist == x))

#ifdef __EXEC_LEAF_NODE_ONLY__
# define kLEAF_NODE_ONLY    TRUE
#else
# define kLEAF_NODE_ONLY    FALSE
#endif

#ifdef __APPLY_NO_TO_ALL__
# define kAPPLY_NO_TO_ALL   TRUE
#else
# define kAPPLY_NO_TO_ALL   FALSE
#endif

#ifdef __NO_MEANS_NO__
# define kNO_MEANS_NO       TRUE
#else
# define kNO_MEANS_NO       FALSE
#endif

/*-----------------------------------------------------------------------*/

#define DB_RangeOK(x)           ((x >= 0) && (x < kRCC_MAX_TOKENS))
#define DB_GetTokenIndex(x)     (x->currentToken)
#define DB_SetTokenIndex(x,y)   (x->currentToken = y)
#define DB_GetTokenString(x,y)  (DB_RangeOK(y) ? x->tokens[y].pStart : NULL) 
#define DB_TokenType(x)         (x->tokens[x->currentToken].type)
#define DB_TokenFlags(x)        (x->tokens[x->currentToken].flags)
#define DB_GetTokenType(x,y)    (DB_RangeOK(y) ? x->tokens[y].type : kTT_INVALID)
#define DB_SetTokenType(x,y)    (x->tokens[x->currentToken].type = y)
#define DB_TokenNode(x)         (x->tokens[x->currentToken].pNode)
#define DB_SetTokenNode(x,y)    (x->tokens[x->currentToken].pNode = y)
#define DB_TokenLength(x)       (x->tokens[x->currentToken].length)
#define DB_TokenString(x)       (x->tokens[x->currentToken].pStart)
#define DB_TokenOffset(x)       (x->tokens[x->currentToken].offset)
#define DB_TokenCount(x)        (x->numTokens)
#define DB_FirstToken(x)        (x->currentToken = 0)
#define DB_LastToken(x)         DB_SetTokenIndex(x, x->numTokens - 1)
#define DB_InvalidToken(x)      (kTT_INVALID == DB_TokenType(x))
#define DB_GetCurrentNode(x)    (x->pCurrentNode)
#define DB_SetPriorNode(x,y)    (x->pPriorNode = y)
#define DB_GetPriorNode(x)      (x->pPriorNode)
#define DB_SetPriorIndex(x,y)   (x->fork = y)
#define DB_GetPriorIndex(x)     (x->fork)
#define DB_GetParamDesc(x)      (x->params)
#define DB_GetParamIndex(x)     (x->currentParam)
#define DB_SetParamIndex(x,y)   (x->currentParam = y)
#define DB_GetParamNode(x)      (x->pCmdNode)
#define DB_GetParamHandler(x)   (x->pHandler)
#define DB_SetParamHandler(x,y) (x->pHandler = y)


/*******************************************************************
 *                           MAIN CODE                             *
 *******************************************************************/

RL_STATIC void DB_SetCurrentNode(tokenTable *pTokens, cmdNode *pNode)
{
	pTokens->pCurrentNode = pNode;

	if (FLAG_SET(pTokens, kRCC_PFLAG_META_COMMAND) &&
		FLAG_CLEAR(pNode, kRCC_PFLAG_META_COMMAND))
	{
		CLEAR_FLAG(pTokens, kRCC_PFLAG_META_COMMAND);
    }
}


RL_STATIC void 
DB_SetParamNode(paramList *pParamList, cmdNode *pNode)
{
    if ((NULL == pParamList->pCmdNode) || (NULL == pNode))
        pParamList->pCmdNode = pNode;
    else
    {
        /* don't overwrite existing node! */
        if (pParamList->pCmdNode != pNode)
            DEBUG_MSG_1("Overwriting node: %x\n", pParamList->pCmdNode);

        return;
    }

    if (NULL == pNode)
        return;

    if (FLAG_SET(pNode,      kRCC_COMMAND_GLOBAL))
        SET_FLAG(pParamList, kRCC_COMMAND_GLOBAL);

    if (FLAG_SET(pNode,      kRCC_COMMAND_NO_TO_ALL))
        SET_FLAG(pParamList, kRCC_COMMAND_NO_TO_ALL);

    if (FLAG_SET(pParamList->pTokens, kRCC_PFLAG_IS_NO_CMD))
    {
        SET_FLAG(pParamList, kRCC_COMMAND_NO);
        if (FLAG_CLEAR(pParamList, kRCC_COMMAND_NO_TO_ALL))
            CLEAR_FLAG(pParamList->pTokens, kRCC_PFLAG_IS_NO_CMD);
    }
}

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean 
DB_NextToken(tokenTable *pTokens)
{
    if (pTokens->currentToken >= (pTokens->numTokens - 1))
        return FALSE;

    pTokens->currentToken++;
    return TRUE;
}

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean 
DB_PrevToken(tokenTable *pTokens)
{
    if (0 >= pTokens->currentToken)
        return FALSE;

    pTokens->currentToken--;
    return TRUE;
}

/*-----------------------------------------------------------------------*/

#ifdef __RCC_DEBUG__
RL_STATIC void
DB_ShowToken(tokenTable *pTokens)
{
    ubyte     index;
    sbyte    *text = "";
    tokenType type = kTT_INVALID;
    sbyte    *pString = "";

    struct flag_text
    {
        tokenType  type;
        sbyte     *text;
        Boolean    data;
    } flag_text[] =
    {
        {kTT_INVALID,   "Invalid flag", FALSE},
        {kTT_NO     ,   "'No' keyword", FALSE},
        {kTT_NODE   ,   "Node        ", TRUE},
        {kTT_KEYWORD,   "Keyword     ", TRUE},
        {kTT_DATA,      "Data        ", TRUE},
        {kTT_ABSOLUTE,  "            ", TRUE}
    };

    if (NULL == pTokens)
        return;

    for (index = 0; index < ARRAY_SIZE(flag_text); index++)
    {
        if (flag_text[index].type == DB_TokenType(pTokens))
        {
            type = flag_text[index].type;
            text = flag_text[index].text;
            if (TRUE == flag_text[index].data)
                pString = DB_TokenString(pTokens);
            break;
        }
    }

    printf("Index:%d Type:%s Value:%s\n", DB_GetTokenIndex(pTokens), text, pString);
}
#else
#define ShowTokenDebug(pTokens, type, dest)
#endif /* __RCC_DEBUG__ */

/*-----------------------------------------------------------------------*/

RL_STATIC void 
DB_SetToken(tokenTable *pTokens, tokenType type, void *dest)
{
#ifdef X__RCC_DEBUG__
    /* make sure we don't step on already good data */
    if (kTT_INVALID != DB_TokenType(pTokens)) {
        printf("\nOverwrite!\n");
        DB_ShowToken(pTokens);
        return;
    }
#endif /* __RCC_DEBUG__ */

    DB_TokenNode(pTokens) = dest;
    DB_TokenType(pTokens) = type;
}

/*-----------------------------------------------------------------------*/

/* string equality with current token */
RL_STATIC Boolean 
DB_TokenMatches(tokenTable *pTokens, sbyte *theWord)
{
    sbyte *tokenWord;

    if (NULL == pTokens || NULL == theWord)
        return FALSE;

    if (NULL == (tokenWord = DB_TokenString(pTokens)))
        return FALSE;

    return (0 == COMPARE(tokenWord, theWord));
}

/*-----------------------------------------------------------------------*/

/* partial string equality with current token */
RL_STATIC Boolean 
DB_TokenBegins(tokenTable *pTokens, sbyte *theWord)
{
    sbyte *tokenWord;

    if (NULL == pTokens || NULL == theWord)
        return FALSE;

    if (NULL == (tokenWord = DB_TokenString(pTokens)))
        return FALSE;

    return (0 == NCOMPARE(tokenWord, theWord, DB_TokenLength(pTokens)));
}

/*-----------------------------------------------------------------------*/

RL_STATIC paramList * 
DB_GetCurrentParamList(cli_env *pCliEnv)
{
    paramList *pParamList = MMISC_GetRootParam(pCliEnv);

    if (NULL == pParamList)
        return NULL;

    while (NULL != pParamList->pNext)
        pParamList = pParamList->pNext;

    return pParamList;
}

/*-----------------------------------------------------------------------*/

RL_STATIC tokenTable * 
DB_GetTokenTable(cli_env *pCliEnv)
{
    paramList *pParamList = DB_GetCurrentParamList(pCliEnv);

    RCC_ASSERT(NULL != pParamList);

    return pParamList->pTokens;
}

/*-----------------------------------------------------------------------*/

/* 
 *  return whole string that matches partial string 
 *  of enum or list value
 */
extern sbyte *
RCC_DB_FullEnum(paramDefn *pParam, sbyte *pData)
{
    sbyte4      count      = CONVERT_GetEnumCount(pParam->pParamInfo);
    sbyte       dLen       = STRLEN(pData);
    DTEnumInfo *pEnumTable = pParam->pParamInfo->pEnumTable;

	/* find it in the list */
	while (0 < count--)
	{
        if (0 == NCOMPARE(pData, pEnumTable->EnumString, dLen))
			return pEnumTable->EnumString;

        pEnumTable++;
	}
    return NULL;
}

/*-----------------------------------------------------------------------*/

RL_STATIC sbyte * 
DB_GetTokenNodeStr(tokenTable *pTokens)
{
    tokenType  type     = DB_TokenType(pTokens);
    cmdNode   *theNode  = NULL;
    paramDefn *theParam = NULL;
    sbyte     *reply    = NULL;

    switch (type)
    {
    case kTT_NODE:
        theNode = DB_TokenNode(pTokens);
        if (NULL != theNode)
            reply = theNode->pKeyword;
        break;
    case kTT_KEYWORD:
    case kTT_ABSOLUTE:
        theParam = DB_TokenNode(pTokens);
        if (NULL != theParam)
            reply = theParam->pKeyword;
        break;
    case kTT_DATA:
        theParam = DB_TokenNode(pTokens);
        if ((NULL != theParam) && ENUMLIST(theParam->type))
            reply = RCC_DB_FullEnum(theParam, DB_TokenString(pTokens));                    
        break;
    default:
        break;
    }
    return reply;
}

/*-----------------------------------------------------------------------*/

RL_STATIC paramID DB_ParamID(paramDescr *pParamDescr)
{
    if (NULL == pParamDescr)
        return 0;

    if (NULL == pParamDescr->pParamDefn)
        return 0;

    return pParamDescr->pParamDefn->id;
}

/*-----------------------------------------------------------------------*/
/* 
 * if a handler uses a parameter that initially meets criteria 
 * but later proves to be invalid, it should call this function
 * to show the user which parameter was bad
 */
extern void RCC_DB_InvalidParam(cli_env *pCliEnv, paramID id)
{
    paramList  *pParamList = DB_GetCurrentParamList(pCliEnv);
    paramDescr *pParamDesc = NULL;

    if (OK != RCC_DB_RetrieveParam(pParamList, NULL, id, &pParamDesc))
        return;

    MMISC_SetErrorPos(pCliEnv, pParamDesc->position);
}

/*-----------------------------------------------------------------------*/

extern sbyte * RCC_DB_BadParam(cli_env *pCliEnv)
{
    tokenTable  *pTokens    = DB_GetTokenTable(pCliEnv);

    DB_FirstToken(pTokens);
    while(1)
    {
        if (kTT_BAD_PARAM == DB_TokenType(pTokens))
            return DB_TokenString(pTokens);

        if (! DB_NextToken(pTokens))
            break;
    }
    return NULL;
}

/*-----------------------------------------------------------------------*/

/* delete all entries, leaving topmost entry blank and ready for use */

RL_STATIC void 
DB_DeleteParamList(cli_env *pCliEnv)
{
    paramList *pParamList = DB_GetCurrentParamList(pCliEnv);
    sbyte4     modeDepth  = MMISC_GetModeDepth(pCliEnv);

    if (NULL == pParamList)
        return;

    /* exiting intermediate modes / global commands */
    while (modeDepth < pParamList->modeDepth)
    {
        if (FLAG_CLEAR(pParamList, kRCC_COMMAND_EXTEND))
            FREEMEM(pParamList->pTokens);

        if (NULL == pParamList->pPrev)
            break;

        pParamList = pParamList->pPrev;
        FREEMEM(pParamList->pNext);
    }

    /* clear current */
    while (NULL != pParamList->pPrev)
    {
        if (pParamList->modeDepth != pParamList->pPrev->modeDepth)
            break;

        pParamList = pParamList->pPrev;
        FREEMEM(pParamList->pNext);
    }

    DB_SetParamNode(pParamList,    NULL);
    DB_SetParamHandler(pParamList, NULL); 
    pParamList->numParams = 0;
    pParamList->flags     = 0;
    MEMSET(pParamList->params, 0, sizeof(pParamList->params));

    if (NULL != pParamList->pTokens)
        MEMSET(pParamList->pTokens, 0, sizeof(tokenTable));

    pParamList->pTokens->pParamList = pParamList;
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS 
DB_NewParamList(cli_env *pCliEnv, Boolean extended)
{
    paramList  *pParamList  = DB_GetCurrentParamList(pCliEnv);
    paramList  *pNextParam  = NULL;
    tokenTable *pTokens     = NULL;
    sbyte4      depth       = MMISC_GetModeDepth(pCliEnv);
    ubyte4      flag        = 0;
    ubyte4      test;

    if (NULL == pParamList)
    {
        if (NULL == (pParamList = RC_CALLOC(sizeof(paramList), 1)))
            return RCC_ERROR_THROW(SYS_ERROR_NO_MEMORY);

        if (NULL == (pTokens = RC_CALLOC(sizeof(tokenTable), 1)))
        {
            RC_FREE(pParamList);
            return RCC_ERROR_THROW(SYS_ERROR_NO_MEMORY);
        }
            
        MMISC_SetRootParam(pCliEnv, pParamList);
        pParamList->pTokens = pTokens;
        pTokens->pParamList = pParamList;
        return OK;
    }


    if (kAPPLY_NO_TO_ALL || FLAG_SET(pParamList, kRCC_COMMAND_NO_TO_ALL))
    {
        flag |= kRCC_COMMAND_NO_TO_ALL;
        if (FLAG_SET(pParamList, kRCC_COMMAND_NO))
            flag |= kRCC_COMMAND_NO;
    }

    if (NULL == (pNextParam = RC_CALLOC(sizeof(paramList), 1)))
        return RCC_ERROR_THROW(SYS_ERROR_NO_MEMORY);

    if (extended)
    {
        if (NULL == pParamList)
            return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

        pTokens  = pParamList->pTokens;
        flag    |= kRCC_COMMAND_EXTEND;
    } 
    else 
    {
        if (NULL == (pTokens = RC_CALLOC(sizeof(tokenTable), 1)))
        {
            FREEMEM(pNextParam);
            return RCC_ERROR_THROW(SYS_ERROR_NO_MEMORY);
        }
        MMISC_SetModeDepth(pCliEnv, ++depth);
    }
    
    pTokens->pParamList   = pNextParam;
    pParamList->pNext     = pNextParam;
    pNextParam->pPrev     = pParamList;
    pNextParam->modeDepth = depth;
    pNextParam->pTokens   = pTokens;
    pNextParam->flags     = flag;

    /* 
     * carry through 'no' status in both paramlist and tokentable 
     * these flags need to be unified.
     */
	test = kRCC_COMMAND_NO | kRCC_COMMAND_NO_TO_ALL;
    if (test == (test & flag)) 
		SET_FLAG(pTokens, kRCC_PFLAG_IS_NO_CMD);
    
    return OK;
}

/*-----------------------------------------------------------------------*/

extern cmdNode * RCC_DB_GetCommandNode(cli_env *pCliEnv)
{
    paramList *pParamList = DB_GetCurrentParamList(pCliEnv);

    if (NULL == pParamList)
        return RCC_DB_GetRootNode();

    while ((NULL == DB_GetParamNode(pParamList)) && (NULL != pParamList->pPrev))
        pParamList = pParamList->pPrev;

    if (NULL != DB_GetParamNode(pParamList))
        return DB_GetParamNode(pParamList);
    else
        return RCC_DB_GetRootNode();
}


/*-----------------------------------------------------------------------*/

extern sbyte4 RCC_DB_GetParamCount(cli_env *pCliEnv, cmdNode *pNode)
{
    tokenTable  *pTokens  = DB_GetTokenTable(pCliEnv);

	return pTokens->nodeParamCount;
}

/*-----------------------------------------------------------------------*/

extern cmdNode * 
RCC_DB_GetParentNode(cli_env *pCliEnv, cmdNode *pNode)
{
    paramList *pParamList = MMISC_GetRootParam(pCliEnv);

    while (NULL != pParamList)
    {
        if (NULL == pParamList->pNext)
            break;

        if (DB_GetParamNode(pParamList->pNext) == pNode)
            return DB_GetParamNode(pParamList);

        pParamList = pParamList->pNext;
    }

    return RCC_DB_GetRootNode();
}

/*-----------------------------------------------------------------------*/

extern sbyte * RCC_DB_GetPrompt(cmdNode *pNode)
{
    return (NULL != pNode ? pNode->pPrompt : NULL);
}

/*-----------------------------------------------------------------------*/

extern cmdNode * RCC_DB_GetRootNode()
{
    extern cmdNode mRootCmdNode;

    return &mRootCmdNode;
}

/*-----------------------------------------------------------------------*/

extern Boolean RCC_DB_ParamAccess(cli_env *pCliEnv, paramDefn *pParam)
{
    if (NULL == pParam)
        return FALSE;

    if (NULL == pParam->pParamInfo)
        return TRUE;
    
    return RC_ACCESS_Allowed(pParam->pParamInfo->nAccessLevel, 
                             MMISC_GetAccess(pCliEnv));
}


/*******************************************************************
 *                      DEBUGGING TOOLS                            *
 *******************************************************************/


extern void
RCC_DB_ShowParentNodes(cli_env *pCliEnv)
{
    paramList *pParamList = MMISC_GetRootParam(pCliEnv);
    sbyte4     modeDepth  = MMISC_GetModeDepth(pCliEnv);
    cmdNode   *pNode      = NULL;
    sbyte4     index      = 0;
    sbyte     *pText      = NULL;
    sbyte      buffer[256];

    RCC_EXT_WriteStrLine(pCliEnv, "");
    while (NULL != pParamList)
    {
        index++;

        sprintf(buffer, "ModeDepth: %d Stack# %d / Depth# %d: Flag %2x ", 
                  modeDepth, index, pParamList->modeDepth, pParamList->flags);

        pNode = DB_GetParamNode(pParamList);
        pText = (NULL != pNode) ? pNode->pKeyword : " -oOo- ";

        RCC_EXT_WriteStr(pCliEnv, buffer);
        RCC_EXT_WriteStrLine(pCliEnv, pText);

        pParamList = pParamList->pNext;
    }
}

/*-----------------------------------------------------------------------*/

#ifdef __RCC_DEBUG__
extern void ShowTokenDebug(tokenTable *pTokens, tokenType type, void * dest)
{
    cmdNode   *theNode  = NULL;
    paramDefn *theParam = NULL;
    sbyte     *pBuff    = "* Empty *";

    if (kTT_NODE == type)
    {
        theNode = dest;
        pBuff = theNode->pKeyword;
    }

    if (kTT_KEYWORD == type)
    {
        theParam = dest;
        pBuff = theParam->pKeyword;
    }

    printf("\nSetting token # %d as type %d with data %s\n",
            DB_GetTokenIndex(pTokens), type, pBuff);
}
#else
#define ShowTokenDebug(pTokens, type, dest)
#endif /* __RCC_DEBUG__ */

/*-----------------------------------------------------------------------*/

#ifdef __RCC_DEBUG__
RL_STATIC void 
RCC_DB_ShowTokenResults(cli_env *pCliEnv)
{
    tokenTable *pTokens = DB_GetTokenTable(pCliEnv);

    RCC_EXT_WriteStrLine(pCliEnv, ""); 
    DB_FirstToken(pTokens);
    while (1) {
        DB_ShowToken(pTokens);
        if (! DB_NextToken(pTokens))
            break;
    }
    RCC_EXT_WriteStrLine(pCliEnv, ""); 
}
#else
#define RCC_DB_ShowTokenResults(pCliEnv)
#endif



/****************************************************************/
/*                    End of accessor methods                   */
/****************************************************************/


extern RLSTATUS RCC_DB_InitTasks(void)
{
    RLSTATUS   status;
    sbyte4     index;
    cliTask   *pTask = mTaskStack;


    for (index = 0; index < kRCC_TASK_STACK_SIZE; index++, pTask++)
    {
        pTask->pHandler = NULL;
        pTask->count    = 0;
        status = OS_SPECIFIC_MUTEX_CREATE(&pTask->mMutex);
        if (OK != status)
            return status;
    }

    return OS_SPECIFIC_MUTEX_CREATE(&mTaskMutex);
}

/*-----------------------------------------------------------------------*/

extern void 
RCC_DB_FreeTasks(void)
{
    sbyte4     index;
    cliTask   *pTask = mTaskStack;

    for (index = 0; index < kRCC_TASK_STACK_SIZE; index++, pTask++)
        OS_SPECIFIC_MUTEX_RELEASE(pTask->mMutex);

    OS_SPECIFIC_MUTEX_RELEASE(mTaskMutex);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_DB_QueueHandler(cli_env *pCliEnv, handlerDefn *pHandler)
{
    RLSTATUS         status         = OK;
    cmdNode         *pNode          = MMISC_GetExecNode(pCliEnv);
    HandlerFunction *pHandlerFunc   = pHandler->pHandlerFunc;
    paramList       *pParamList     = DB_GetCurrentParamList(pCliEnv);
    sbyte           *pOutputBuf     = MMISC_OutputBuffer(pCliEnv);
    cliTask         *pTempTask      = mTaskStack;
    cliTask         *pTask          = NULL;
    cliTask         *pNewTask       = NULL;
    sbyte4           index          = 0;

    if (! FLAG_SET(pNode, kRCC_COMMAND_QUEUE))
        return RCC_EXEC_FN(pHandlerFunc, pCliEnv, pParamList, pOutputBuf);

    /* initial registration */
    OS_SPECIFIC_MUTEX_WAIT(mTaskMutex);

    /* has a handler already registered? */
    for (index = 0; index < kRCC_TASK_STACK_SIZE; index++, pTempTask++)
    {
        if (pHandler == DB_GetParamHandler(pTempTask))
        {
            pTask = pTempTask;
            break;
        }

        /* grab a new task in case not yet registered */
        if ((NULL == DB_GetParamHandler(pTempTask)) && (NULL == pNewTask))
            pNewTask = pTempTask;
    }

    /* register new handler */
    if ((NULL == pTask) && (NULL != pNewTask))
    {
        pTask = pNewTask;
        pTask->pHandler = pHandler;
    }

    OS_SPECIFIC_MUTEX_RELEASE(mTaskMutex);

    /* stack was full */
    if (NULL == pTask)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    OS_SPECIFIC_MUTEX_WAIT(pTask->mMutex);

        pTask->count++;
        status = RCC_EXEC_FN(pHandlerFunc, pCliEnv, pParamList, pOutputBuf);
        pTask->count--;

        if (0 >= pTask->count)
            pTask->pHandler = NULL;

    OS_SPECIFIC_MUTEX_RELEASE(pTask->mMutex);

    return status;
}

/*-----------------------------------------------------------------------*/

/*   Tokenize CLI input  */
extern RLSTATUS 
RCC_DB_ParseCmdString(cli_env *pCliEnv)
{
    enum WordStatus {kWORD_NO, kWORD_START, kWORD_YES, kWORD_DONE};
    sbyte      *pCmd;
    EditType    tokenLength = MEDIT_GetLength(pCliEnv);
    sbyte4      index;
    sbyte       quoteChar;
    Length      offset = 0;
    tokenTable *pTokens;
    tokenNode  *pTokenNode;
    sbyte       wordStatus;
    sbyte       charIn;
    sbyte      *pBuf;
    sbyte      *pNext;
    sbyte       prior;
    sbyte       newChar;
    Boolean     whiteSpace = FALSE;
    Boolean     exitLoop   = FALSE;
    Boolean     endCommand = FALSE;
    Boolean     convert    = RCC_IsEnabled(pCliEnv, kRCC_FLAG_CONVERT);
    AliasTable *pAliases   = MMISC_AliasPtr(pCliEnv);
    CmdAlias   *pAlias     = pAliases->alias;
    sbyte       newText[kRCC_MAX_CMD_LEN];
    sbyte       tempBuf[kRCC_MAX_CMD_LEN];

    DB_DeleteParamList(pCliEnv);

    if (0 >= tokenLength)
        return RCC_ERROR_THROW(ERROR_GENERAL_NO_DATA);

    if (NULL == (pCmd = MEDIT_BufPtr(pCliEnv)))
        return RCC_ERROR_THROW(ERROR_GENERAL_NO_DATA);

    if (NULL == (pTokens = DB_GetTokenTable(pCliEnv)))
        return RCC_ERROR_THROW(ERROR_GENERAL_NO_DATA);

    wordStatus  = kWORD_NO;
    quoteChar   = kNoQuote;
    pTokenNode  = pTokens->tokens;

    /* apply any appropriate aliases -- making sure not in alias command! */

    if (RCC_NotEnabled(pCliEnv, kRCC_FLAG_ALIASED))
    {
        STRCPY(tempBuf, pCmd);
        for (index = 0; index < pAliases->numEntries; index++, pAlias++)
        {
            if (OK == RC_Replace(tempBuf, newText, kRCC_MAX_CMD_LEN, 
                                 pAlias->pName, pAlias->pText, TRUE))
            {
                STRCPY(tempBuf, newText);
            }
        }
        STRCPY(pTokens->buffer, tempBuf);
    }
    else
        STRCPY(pTokens->buffer, pCmd);

    tokenLength = STRLEN(pTokens->buffer);

    for (index = 0; index <= tokenLength; index++)
    {
        charIn = pTokens->buffer[index];
        switch(charIn)
        {
        case kSingleQuote:
        case kDoubleQuote:
            if (kWORD_YES == wordStatus) 
            {
                if (charIn == quoteChar)
                    wordStatus = kWORD_DONE;
                break;
            }
            wordStatus = kWORD_START;
            quoteChar  = charIn;
            break;
#ifdef kKEY_SEPARATOR
        case kKEY_SEPARATOR:
            endCommand = TRUE;
#endif

#ifdef kKEY_CONTINUE
        case kKEY_CONTINUE:
#endif

#ifdef kKEY_PIPE
        case kKEY_PIPE
#endif

#ifdef kKEY_EQUALS
        case kKEY_EQUALS:
#endif
        case kCR:
        case kLF:
        case kSpaceChar:
            if (kWORD_YES == wordStatus) {
                if (kNoQuote == quoteChar) {
                    wordStatus = kWORD_DONE;
                }
            } 
            else 
                wordStatus = kWORD_NO;
            break;
        case kRCC_CHAR_COMMENT:
            if (kNoQuote == quoteChar)
            {
                if (kWORD_YES == wordStatus)
                    wordStatus = kWORD_DONE;

                exitLoop = TRUE;
            }
            break;
        case kCHAR_NULL:
            if (kWORD_YES == wordStatus)
                wordStatus = kWORD_DONE;
            break;
        default:
#ifdef kRCC_IGNORE_CHARS
            whiteSpace = (NULL != STRCHR(kRCC_IGNORE_CHARS, charIn));
#endif /* kRCC_IGNORE_CHARS */

            if (kWORD_YES != wordStatus)
            {
                if (! whiteSpace)
                    wordStatus = kWORD_START;
            }
            else {
                if (whiteSpace && (kNoQuote == quoteChar))
                    wordStatus = kWORD_DONE;
            } 
            break;
        }

        if (kWORD_START == wordStatus)
        {
            offset = index;
            if (kNoQuote != quoteChar)
                offset++;
            wordStatus = kWORD_YES;
        }

        if (kWORD_DONE == wordStatus)
        {
            pTokens->buffer[index] = 0;

            pTokenNode->pStart = &pTokens->buffer[offset];
            pTokenNode->length = index - offset;
            pTokenNode->offset = offset;
            if (kNoQuote != quoteChar)
                pTokenNode->flags = kTOKEN_QUOTED;
            
            if (convert)
            {
                pBuf = pTokenNode->pStart;
                for (prior = 0; 0 != *pBuf; prior = *(pBuf++))
                {
                    if ('\\' != prior)
                        continue;
                    
                    switch (*pBuf)
                    {
                        case 'r':
                            newChar = '\r';
                            break;
                        case 'n':
                            newChar = '\n';
                            break;
                        case 'a':
                            newChar = '\a';
                            break;
                        case 't':
                            newChar = '\t';
                            break;
                        default:
                            newChar = 0;
                            break;
                    }

                    if (0 == newChar)
                        continue;

                    *(pBuf - 1) = newChar;

                    for (pNext = pBuf + 1; 0 != *pNext; pNext++)
                        *(pNext - 1) = *pNext;

                    pTokenNode->pStart[--pTokenNode->length] = 0;
                }
            }      
            pTokenNode++;

            pTokens->numTokens++;
            wordStatus   = kWORD_NO;
            quoteChar    = kNoQuote;
 
            if (endCommand)
            {
                pTokenNode->type = kTT_END_COMMAND;
                pTokens->numTokens++;
                endCommand = FALSE;
            }
        }

        if (exitLoop)
            break;

    } /* for each index */

    if (0 == pTokens->numTokens)
        return RCC_ERROR_THROW(ERROR_GENERAL_NO_DATA);

    return OK;
}

/*-----------------------------------------------------------------------*/

RLSTATUS
RCC_DB_AccessValidate(cli_env *pCliEnv, sbyte *pString, 
                      DTTypeInfo *pInfo, Boolean errMsg)
{
    extern DTTypeInfo mAccessInfo;
    DTTypeInfo full = mAccessInfo;
    sbyte   buffer[32];

#ifndef __RLI_ACCESS_LEVEL_MASK__
    return OK;
#else
    sprintf(buffer, "%s L=0 U=%d", mAccessInfo.pValidateStr,
                             __RLI_ACCESS_LEVEL_MASK__);
#endif
    
    full.pValidateStr = buffer;
    return CONVERT_Validate(pCliEnv, pString, &full, errMsg);
}

/*-----------------------------------------------------------------------*/

extern void 
RCC_DB_SetCommand(cli_env *pCliEnv, sbyte *pString, sbyte4 commandLen)
{
    sbyte     *pCmdBuff;

    if (0 >= commandLen)
        commandLen = STRLEN(pString);

    commandLen = RC_MIN(commandLen, kRCC_MAX_CMD_LEN);
    pCmdBuff   = MEDIT_BufPtr(pCliEnv);

    STRNCPY(pCmdBuff, pString, commandLen);
    pCmdBuff[commandLen] = 0;
    MEDIT_SetLength(pCliEnv, (EditType) commandLen);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
DB_ResetMatches(cli_env *pCliEnv)
{
    sbyte **matches = MMISC_MatchList(pCliEnv);

    MEMSET(matches, 0, sizeof(MMISC_MatchList(pCliEnv)));
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
DB_AddMatch(cli_env *pCliEnv, sbyte *pData)
{
    sbyte  **ppMatches = MMISC_MatchList(pCliEnv);
    sbyte4   index;

    if (NULL == pData)
        return;

    for (index = 0; index < kRCC_MATCH_LIST_SIZE; index++, ppMatches++)
    {
        if (NULL == *ppMatches)
        {
            *ppMatches = pData;
            break;
        }

        /* don't add if already there */

        if (0 == COMPARE(*ppMatches, pData))
            break;
    }
}

/*-----------------------------------------------------------------------*/

extern void
RCC_DB_ShowInput(cli_env *pCliEnv)
{
    sbyte  *cmdPtr = MEDIT_BufPtr(pCliEnv);

    RCC_TASK_PrintPrompt(pCliEnv);
    RCC_EXT_WriteStr(pCliEnv, cmdPtr);
}

/*-----------------------------------------------------------------------*/

/* at least extend text for the portion where all matches are the same */

RL_STATIC void
DB_MatchCommon(cli_env *pCliEnv)
{
    sbyte4      index;
    sbyte     **ppMatches = MMISC_MatchList(pCliEnv);
    sbyte      *pPrior    = *ppMatches;
    sbyte      *pSrc;
    sbyte      *pDest;
    sbyte4      length    = 0;
    sbyte4      common    = (NULL == pPrior) ? 0 : STRLEN(pPrior);
    tokenTable *pTokens   = DB_GetTokenTable(pCliEnv);
    sbyte4      tokenLen  = DB_TokenLength(pTokens);

    ppMatches++;
    for (index = 1; index < kRCC_MATCH_LIST_SIZE; index++, ppMatches++)
    {
        if (NULL == *ppMatches)
            break;

        pSrc   =  pPrior;
        pDest  = *ppMatches;
        length =  0;

        while (1)
        {
            if (('\0'== *pSrc) || ('\0' == *pDest))
                break;

#ifdef __RCC_CASE_INSENSITIVE__
            if (TOUPPER(*pSrc) != TOUPPER(*pDest))
#else
            if (*pSrc != *pDest)
#endif
                break;

            length++;
            pSrc++;
            pDest++;
        }

        if (length < common)
            common = length;
    }

    common -= tokenLen;
    if (0 < common)
        RCC_EXT_InsertText(pCliEnv, &pPrior[tokenLen], common);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void
DB_ShowMatches(cli_env *pCliEnv)
{
    sbyte4     index;
    sbyte    **matches  = MMISC_MatchList(pCliEnv);
    EditType   width    = MSCRN_GetWidth(pCliEnv);
    sbyte4     xPos     = 0;
    sbyte4     length;

    DB_MatchCommon(pCliEnv);

    RCC_EXT_WriteStrLine(pCliEnv, "");

    for (index = 0; index < kRCC_MATCH_LIST_SIZE; index++, matches++)
    {
        if (NULL == *matches)
            break;

        length = STRLEN(*matches);

        if (xPos + length > width)
        {
            RCC_EXT_WriteStrLine(pCliEnv, "");
            xPos = 0;
        }

        xPos += length;

        RCC_EXT_WriteStr(pCliEnv, *matches);

        while (0 != (xPos % kRCC_MATCH_COL_WIDTH))
        {
            RCC_EXT_Write(pCliEnv, " ", 1);
            if (width <= ++xPos + 1)
            {
                RCC_EXT_WriteStrLine(pCliEnv, "");
                xPos   = 0;
                break;
            }
        }
    }
    RCC_EXT_WriteStrLine(pCliEnv, "");
}

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean 
DB_ParamUsed(tokenTable *pTokens, paramDefn *pParam)
{
    sbyte4     index;
    tokenNode *token;
    paramList *pParamList;

    while (TRUE)
    {
        token = pTokens->tokens;
        for (index = 0; index < pTokens->numTokens; index++, token++)
        {
            if (kTT_INVALID == token->type)
                break;

            if (pParam == token->pNode)
                return TRUE;
        }
        pParamList = pTokens->pParamList;
        if ((NULL == pParamList) || (NULL == pParamList->pPrev))
            return FALSE;

        while (pTokens == pParamList->pTokens)
        {
            pParamList = pParamList->pPrev;
            if (NULL == pParamList)
                return FALSE;
        }
        pTokens = pParamList->pTokens;
    }
    return FALSE;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void
DB_ResetErrorText(cli_env *pCliEnv)
{
    sbyte *pErrorText = MMISC_GetErrorText(pCliEnv);

    *pErrorText = 0;
    MMISC_SetErrorLen(pCliEnv, 0);
}

/*-----------------------------------------------------------------------*/

extern void
RCC_DB_AppendErrorText(cli_env *pCliEnv, sbyte *pText, sbyte4 length)
{
    sbyte      *pBuf     = MMISC_GetErrorText(pCliEnv);
    sbyte4      bufLen   = MMISC_GetErrorLen(pCliEnv);
    sbyte4      textLen  = length > 0 ? length : STRLEN(pText);

    pBuf += bufLen;
    textLen = RC_MIN(textLen, (kRCC_ERROR_TEXT_SIZE - bufLen));
    STRNCPY(pBuf, pText, textLen);
    pBuf[textLen] = 0;
    MMISC_SetErrorLen(pCliEnv, (bufLen + textLen));
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
DB_RangeText(paramDefn *pParam, sbyte *pBuffer, sbyte4 bufLen)
{
    sbyte      *pFromText = NULL;
    sbyte      *pToText   = NULL;
    sbyte4      fromLen   = 0;
    sbyte4      toLen     = 0;
    DTTypeInfo *typeInfo  = NULL;

    if (NULL == pParam)
        return;

    RC_CONVERT_Suffix(pParam->pParamInfo, "L=", &pFromText, &fromLen);
    RC_CONVERT_Suffix(pParam->pParamInfo, "U=", &pToText,   &toLen);

    if ((NULL == pFromText) || (NULL == pToText))
    {
        if (NULL == (typeInfo = CONVERT_TypeToInfo(pParam->type)))
            return;

        RC_CONVERT_Suffix(typeInfo, "L=", &pFromText, &fromLen);
        RC_CONVERT_Suffix(typeInfo, "U=", &pToText,   &toLen);

        if ((NULL == pFromText) || (NULL == pToText))
            return;
    }

    if (bufLen < (fromLen + (signed) sizeof(kRCC_MSG_VALID_TO) - 1 + toLen))
    {
        *pBuffer = 0;
        return;
    }

    STRNCPY(pBuffer, pFromText, fromLen);
    pBuffer += fromLen;
    STRCPY(pBuffer, kRCC_MSG_VALID_TO);
    pBuffer += sizeof(kRCC_MSG_VALID_TO) - 1;
    STRNCPY(pBuffer, pToText, toLen);
    pBuffer += toLen;
    *pBuffer = 0;
}


/*-----------------------------------------------------------------------*/

RL_STATIC void 
DB_ErrorRange(cli_env *pCliEnv, paramDefn *pParam)
{
    sbyte      *pBuf     = MMISC_GetErrorText(pCliEnv);
    sbyte4      bufLen   = MMISC_GetErrorLen(pCliEnv);

    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ERROR_SET))
        return;

    RCC_EnableFeature(pCliEnv, kRCC_FLAG_ERROR_SET);

    pBuf += bufLen;

    DB_RangeText(pParam, pBuf, kRCC_ERROR_TEXT_SIZE - bufLen);
     
    RCC_DB_AppendErrorText(pCliEnv, pBuf, 0);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
DB_ErrorType(cli_env *pCliEnv, paramDefn *pParam)
{
    sbyte      *typeText = NULL;

    if (NULL == pParam)
        return;

    if (NULL == (typeText = CONVERT_GetDTName(pParam->type)))
        return;

    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ERROR_SET))
        return;

    RCC_EnableFeature(pCliEnv, kRCC_FLAG_ERROR_SET);
    RCC_DB_AppendErrorText(pCliEnv, kRCC_MSG_TYPE, 0);
    RCC_DB_AppendErrorText(pCliEnv, typeText, 0);    
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
DB_ErrorProto(cli_env *pCliEnv, paramDefn *pParam)
{
    sbyte      *typeText = NULL;

    if (NULL == pParam)
        return;

    if (NULL == (typeText = CONVERT_GetDTName(pParam->type)))
        return;

    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ERROR_SET))
        return;

    RCC_EnableFeature(pCliEnv, kRCC_FLAG_ERROR_SET);
    RCC_DB_AppendErrorText(pCliEnv, kRCC_MSG_TYPE, 0);
    RCC_DB_AppendErrorText(pCliEnv, typeText, 0);    
}

/*-----------------------------------------------------------------------*/

/* 
 * if enum is ambiguous, add all possible matches 
 * returns number of possible matches
 */
RL_STATIC int
DB_EnumMatch(cli_env *pCliEnv, paramDefn *pParam, sbyte *pData)
{
    sbyte4       i;
    sbyte4       count;
    sbyte4       dLen;
    sbyte4       added;
    DTTypeInfo  *pTypeInfo;
    DTEnumInfo  *pEnumTable;

    if ((NULL == pParam) || (NULL == pData))
        return 0;

    if (! ENUMLIST(pParam->type))
        return 0;

    if (NULL == (pTypeInfo = pParam->pParamInfo))
        return 0;

    if (NULL == (pEnumTable = pTypeInfo->pEnumTable))
        return 0;

    added = 0;
    dLen  = STRLEN(pData);
    count = CONVERT_GetEnumCount(pTypeInfo);
    for (i = 0; i < count; i++, pEnumTable++)
	{
        if (0 != NCOMPARE(pData, pEnumTable->EnumString, dLen))
            continue;

        DB_AddMatch(pCliEnv, pEnumTable->EnumString);
        added++;
	}
    return added;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void
DB_ErrorToken(cli_env *pCliEnv)
{
    tokenTable *pTokens = DB_GetTokenTable(pCliEnv);

    if (NULL == pTokens)
        return;

    MMISC_SetErrorPos(pCliEnv, DB_TokenOffset(pTokens));
}

/*-----------------------------------------------------------------------*/

/* 
 *  if validation fails because of ambiguous data,
 *  add partial matches to match list for display
 */

RL_STATIC RLSTATUS
DB_Validate(cli_env *pCliEnv, paramDefn *pParam, sbyte *data, Boolean error)
{
    RLSTATUS status = OK;
#ifdef __RCC_DEBUG__
    sbyte      *pKeyword    = pParam->pKeyword;
    sbyte      *pHelp       = (NULL != pParam->pParamInfo ? pParam->pParamInfo->pHelpStr : NULL);
#endif

#ifdef __RCC_NO_KEYWORD_ARGUMENTS__
    /* disallow command keywords to be used as string arguments */
    paramList  *pList   = DB_GetCurrentParamList(pCliEnv);
    tokenTable *pTokens = pList->pTokens;
    sbyte      *pText   = DB_TokenString(pTokens);
    cmdNode    *pNode;

    if (kDTstring == pParam->type)
    {
        for ( ; NULL != pList; pList = pList->pPrev)
        {
            if (NULL == (pNode = DB_GetParamNode(pList)))
                continue;

            if (0 != COMPARE(pNode->pKeyword, pText))
                continue;
            
            MMISC_SetErrorPos(pCliEnv, DB_TokenOffset(pTokens));
            return RCC_ERROR_THROW(ERROR_RCC_KEYWORD_AS_PARAM);
        }
    }
#endif /* __RCC_NO_KEYWORD_ARGUMENTS__ */

    /* old version */
    if (NULL == pParam->pParamInfo)
    {
        status = CONVERT_Valid(data, pParam->type);
        if (OK == status)
            status = STATUS_RCC_TOKEN_MATCH;
        
        return status;
    }

    if (kDTabsolute == pParam->type)
        return OK;

    status = CONVERT_Validate(pCliEnv, data, pParam->pParamInfo, error);

    if (error && (OK != status))
    {
        DB_ErrorToken(pCliEnv);

        /* get a more descriptive error message */
        switch (status)
        {
        case ERROR_GENERAL_DATA_AMBIG:
            if (ENUMLIST(pParam->type))
                DB_EnumMatch(pCliEnv, pParam, data);
            break;
        case ERROR_GENERAL_NOT_FOUND:
            if (ENUMLIST(pParam->type))
            {
                CONVERT_EnumStr(pParam->pParamInfo, 
                                MMISC_GetErrorText(pCliEnv), 
                                kRCC_ERROR_TEXT_SIZE, 
                                (kDTenum == pParam->type ? "|" : ","));
                status = ERROR_GENERAL_OUT_OF_RANGE;
            }
            else
                DB_ErrorProto(pCliEnv, pParam);
            break;
        case ERROR_GENERAL_OUT_OF_RANGE:
            DB_ErrorRange(pCliEnv, pParam);
            break;
        case ERROR_RCC_INVALID_PARAM:
        case ERROR_GENERAL_ILLEGAL_VALUE:
        case ERROR_CONVERSION_INCORRECT_TYPE:
            DB_ErrorType(pCliEnv, pParam);
            break;
        default:
/*
            status = ERROR_RCC_INVALID_VALUE;
*/
            break;
        }
    }

    if (OK == status)
        status = STATUS_RCC_TOKEN_MATCH;

    /* 
     * normally we only use "throw" when returning the orignal error,
     * not when we're passing up an error from below.
     * in this case the subroutines didn't use throw, 
     * and we know that status is an error.
     */
    return RCC_ERROR_THROW(status);
}

/*-----------------------------------------------------------------------*/

extern void RCC_DB_SetErrorPos(cli_env *pCliEnv, sbyte4 position)
{
    MMISC_SetErrorPos(pCliEnv, position);
}

/*-----------------------------------------------------------------------*/

RL_STATIC sbyte *
DB_MemberElement(paramDefn *pParam, sbyte *pText, Boolean exact)
{
    sbyte4      index;
    sbyte4      count;
    sbyte4      dlen;
    DTEnumInfo *pEnumTable;

    if (! ENUMLIST(pParam->type))
        return NULL;

    if (NULL == pParam->pParamInfo)
        return NULL;

    dlen       = STRLEN(pText);
    pEnumTable = pParam->pParamInfo->pEnumTable;
    count      = CONVERT_GetEnumCount(pParam->pParamInfo);
    for (index = 0; index < count; index++, pEnumTable++)
    {
        if (exact)
        {
            if (0 == COMPARE(pText, pEnumTable->EnumString))
                return pEnumTable->EnumString;
        }
        else
        {
            if (0 == NCOMPARE(pText, pEnumTable->EnumString, dlen))
                return pEnumTable->EnumString;
        }
    }
    return NULL;
}

/*-----------------------------------------------------------------------*/

RL_STATIC cmdNode *
DB_NodeMatch(cli_env *pCliEnv, cmdNode *pParent, 
             Boolean global, Boolean exact, sbyte4 *matchCount)
{ 
    sbyte4      index;
    sbyte4      globalMatch;
    cmdNode    *pChild     = pParent->pChildren;
    cmdNode    *pMatchNode = NULL;
    tokenTable *pTokens    = DB_GetTokenTable(pCliEnv);
    
    *matchCount = 0;

#ifdef __RCC_EXACT_NODES_ONLY__
    if (! exact)
        return OK;
#endif

    if (! exact && FLAG_SET(pParent, kRCC_COMMAND_EXACT_NODE))
        return NULL;

    for (index = 0; index < pParent->numChildren; index++, pChild++) 
    {
        if (! MMISC_ValidGlobal(pChild, global))
            continue;

        if (! RC_ACCESS_Allowed(pChild->accessLvl, MMISC_GetAccess(pCliEnv)))
		    continue;

        if (exact)
        {
            if (DB_TokenMatches(pTokens, pChild->pKeyword))
            {
                (*matchCount)++;
                pMatchNode = pChild;
                break;
            }
        }
        else
        {
            if (  DB_TokenBegins(pTokens, pChild->pKeyword) &&
                ! DB_TokenMatches(pTokens, pChild->pKeyword))
            {
                (*matchCount)++;
                pMatchNode = pChild;
                DB_AddMatch(pCliEnv, pChild->pKeyword);
            }
        }
    }

    /* check to see if partial match against 
       both "local" and "global" nodes */

    if ( FLAG_SET(pTokens, kRCC_PFLAG_NEW_LINE) && 
         ! (global || exact || (NULL == pMatchNode)) )
    {
        DB_NodeMatch(pCliEnv, RCC_DB_GetRootNode(), TRUE, FALSE, &globalMatch);
        *matchCount += globalMatch;
    }

    return pMatchNode;
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS   
DB_ValidParam(cli_env *pCliEnv, paramDefn *pParam,
              Boolean keyword, Boolean exact)
{
    RLSTATUS    status;
    tokenTable *pTokens     = DB_GetTokenTable(pCliEnv);
    Boolean     error       = FALSE;
#ifdef __RCC_DEBUG__
    sbyte      *pTokenText  = DB_TokenString(pTokens);
    sbyte      *pKeyword    = pParam->pKeyword;
    sbyte      *pHelp       = (NULL != pParam->pParamInfo ? pParam->pParamInfo->pHelpStr : NULL);
#endif
    
    if (! RCC_DB_ParamAccess(pCliEnv, pParam))
        return OK;

    if (keyword != MPARM_HasKeyword(pParam))
        return OK;

    if (keyword)
    {
        if (exact)
        {
            if (DB_TokenMatches(pTokens, pParam->pKeyword))
                return STATUS_RCC_TOKEN_MATCH;
        }
        else
        {
            if ( DB_TokenBegins(pTokens, pParam->pKeyword) &&
                !DB_TokenMatches(pTokens, pParam->pKeyword))
            {
                DB_AddMatch(pCliEnv, pParam->pKeyword);
                return STATUS_RCC_TOKEN_MATCH;
            }
        }
    }
    else
    {
        /* an unnamed string will match anything */

        if (kDTstring == pParam->type)
            return exact ? OK : STATUS_RCC_TOKEN_MATCH;

        status = DB_Validate(pCliEnv, pParam, DB_TokenString(pTokens), TRUE);
        if (OK != status)
            return status;

        /* last try: is it an enum or list member? */

        if (DB_MemberElement(pParam, DB_TokenString(pTokens), TRUE))
            return STATUS_RCC_TOKEN_MATCH;
    }
    
    return OK;
}

/*-----------------------------------------------------------------------*/

extern handlerDefn *
RCC_DB_CurrentHandler(cli_env *pCliEnv)
{
    paramList *pParams = DB_GetCurrentParamList(pCliEnv);

    if (NULL == pParams)
        return NULL;

    return (handlerDefn *) DB_GetParamHandler(pParams);
}

/*-----------------------------------------------------------------------*/

RL_STATIC paramDefn *
DB_LookupParam(cli_env *pCliEnv, cmdNode *pNode, paramEntry *pEntry, Boolean *pSameNode)
{
    sbyte4     index;
    paramDefn *pParam;
    paramList *pParamList = DB_GetCurrentParamList(pCliEnv);

	if (NULL == pEntry)
		return NULL;

    *pSameNode = TRUE;
    do 
    {
        pParam = pNode->pParams;
        for (index = 0; index < pNode->numParams; index++, pParam++)
        {
            if (pEntry->id == pParam->id)
                return pParam;
        }

        if (NULL != pParamList)
        {
            pParamList = pParamList->pPrev;
            if (NULL != pParamList)
                pNode = pParamList->pCmdNode;

            if (NULL == pNode)
                break;
        }
        *pSameNode = FALSE;

    } while (NULL != pParamList);

    return NULL;
}

/*-----------------------------------------------------------------------*/

/* return ordered parameter that must be matched */
RL_STATIC paramDefn *
DB_ThisParam(cli_env *pCliEnv, sbyte4 index, Boolean keyword, 
             Boolean *pRequired, Boolean * pSameNode)
{
    tokenTable  *pTokens  = DB_GetTokenTable(pCliEnv);
    cmdNode     *pNode    = DB_GetCurrentNode(pTokens);
    paramList   *pParams  = DB_GetCurrentParamList(pCliEnv);
    oHandler    *pHandler = (oHandler *) DB_GetParamHandler(pParams);
    paramEntry  *pEntry   = NULL;
    paramDefn   *pParam   = NULL;
    sbyte4       max      = 0;
#ifdef __RCC_DEBUG__
    sbyte      *pTokenText   = DB_TokenString(pTokens);
    sbyte4      tokenIndex   = DB_GetTokenIndex(pTokens);
    sbyte4      origIndex    = index;
    paramEntry *pOrigEntry   = NULL;
    sbyte4      dCount       = 0;
#endif

    *pRequired = FALSE;
    *pSameNode = FALSE;

    if (NULL == pNode)
        return NULL;

    /* if we don't have a handle, try the first one */
    if (NULL == pHandler)
        pHandler = (oHandler *) pNode->pHandlers;

    /* 
     * no handler for this node, but the parameter
     * may be required by a subsequent handler,
     * so just hand back the indexed node parameter
     */

    if (NULL == pHandler)
    {
        if (index >= pNode->numParams)
            return NULL;

        return &(pNode->pParams[index]);
    }

    if (NULL == (pEntry = HANDLER_PARAMS(pHandler)))
        return NULL;

    if (index >= (max = HANDLER_PARAM_COUNT(pHandler)))
        return NULL;

    if (keyword)
    {
        pEntry     += index;
        *pRequired  = FLAG_SET(pEntry, kRCC_PARAMETER_REQUIRED);

        return DB_LookupParam(pCliEnv, pNode, pEntry, pSameNode);
    }

    for ( ; 0 < max; max--, pEntry++)
    {
        if (NULL == (pParam = DB_LookupParam(pCliEnv, pNode, pEntry, pSameNode)))
            break;

        if (MPARM_HasKeyword(pParam))
            continue;

        if (0 > --index)
            return pParam;
    }
    
    return NULL;
}

/*-----------------------------------------------------------------------*/

extern paramDefn *
RCC_DB_OrderedParam(cli_env *pCliEnv, sbyte4 index)
{
    Boolean     required;
    Boolean     sameNode;
    tokenTable  *pTokens  = DB_GetTokenTable(pCliEnv);

    if (index < DB_GetParamIndex(pTokens))
        return NULL;

    return DB_ThisParam(pCliEnv, index, TRUE, &required, &sameNode);
}


/*-----------------------------------------------------------------------*/

/* get dtinfo validstr object, e.g., AL=x */
RL_STATIC sbyte4 
DB_Suffix(cli_env *pCliEnv, DTTypeInfo *pTypeInfo, sbyte *pBuf)
{
    sbyte4   value    = 0;
    sbyte4   length   = 0;
    sbyte   *pSuffix  = 0;
    sbyte   *pName    = NULL;
    sbyte    data[kMagicMarkupBufferSize];


    RC_CONVERT_Suffix(pTypeInfo, pBuf, &pSuffix, &length);
    if (0 >= length)
        return 0;

    if (IS_RAPIDMARK_START(pBuf) && 
        IS_RAPIDMARK_END(&pBuf[length - 2]))
    {
        if (NULL == (pName = RC_MALLOC(length - 3)))
            return 0;

        STRNCPY(pName, &pBuf[2], length - 4);
        DB_ReadData(pCliEnv, pName, data, NULL);
        CONVERT_StrTo(data, &value, kDTinteger);
    }
    else
    {
        if (NULL == (pName = RC_MALLOC(length + 1)))
            return 0;

        STRNCPY(pName, pSuffix, length);
        pName[length] = 0;
        CONVERT_StrTo(pName, &value, kDTinteger);
    }

    RC_FREE(pName);
    return value;
}


/* find position of invalid "no" */
RL_STATIC void 
DB_SetErrorNo(cli_env *pCliEnv, cmdNode *pNode)
{
    tokenTable  *pTokens = DB_GetTokenTable(pCliEnv);
    tokenNode   *pToken  = pTokens->tokens;
    sbyte4       index;
    sbyte4       loop;

    for (index = 0; index < pTokens->numTokens; index++, pToken++)
    {
        if (pToken->pNode != pNode)
            continue;

        for (loop = index; loop >= 0; loop--, pToken--)
        {
            if (kTT_NO != pToken->type)
                continue;

            MMISC_SetErrorPos(pCliEnv, pToken->offset);
            return;
        }
    }
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS
DB_SaveParameter(cli_env *pCliEnv, paramDefn *pParam, sbyte4 index)
{
    tokenTable *pTokens     = DB_GetTokenTable(pCliEnv);
    RLSTATUS    status      = OK;
    sbyte4      arraySize   = 0;
    sbyte4      arrayMin    = 0;
    sbyte4      arrayMax    = 0;
    Boolean     keyword     = MPARM_HasKeyword(pParam);
#ifdef __RCC_DEBUG__
    sbyte      *pTokenText  = DB_TokenString(pTokens);
    sbyte      *pKeyword    = pParam->pKeyword;
    sbyte      *pHelp       = (NULL != pParam->pParamInfo ? pParam->pParamInfo->pHelpStr : NULL);
#endif

    /* prevent param overflow */
    if (kRCC_MAX_PARAMS <= pTokens->nodeParamCount)
        return RCC_ERROR_THROW(ERROR_RCC_EXTRA_PARAMS);

    if (kDTabsolute == pParam->type)
        DB_SetToken(pTokens, kTT_ABSOLUTE, pParam);
    else
    {
        if (MPARM_HasKeyword(pParam))
        {
            DB_SetToken(pTokens, kTT_KEYWORD, pParam);

            /* get the data associated with this keyword */
            if (! DB_NextToken(pTokens))
            {
                /* meta commands won't actually use data anyway */
                if (FLAG_SET(pTokens, kRCC_PFLAG_IS_META))
                    return STATUS_RCC_TOKEN_MATCH;

                DB_ErrorToken(pCliEnv);
                return RCC_ERROR_THROW(ERROR_RCC_NO_PARAM_DATA);
            }
        }

        if (FLAG_SET(pParam, kRCC_PARAMETER_ARRAY))
        {
            if (NULL != pParam->pParamInfo)
            {
                arrayMin = DB_Suffix(pCliEnv, pParam->pParamInfo, "AL=");
                arrayMax = DB_Suffix(pCliEnv, pParam->pParamInfo, "AU=");
            }
        }

        while (1)
        {
#ifdef __RCC_DEBUG__
            pTokenText = DB_TokenString(pTokens);
#endif
            /* make sure data is valid */
            status = DB_Validate(pCliEnv, pParam, DB_TokenString(pTokens), TRUE);
            if (STATUS_RCC_TOKEN_MATCH != status)
            {
                if (0 == arraySize)
                    return status;

                DB_SetToken(pTokens, kTT_INVALID, NULL);
                DB_PrevToken(pTokens);
                break;
            }

            if (FLAG_CLEAR(pParam, kRCC_PARAMETER_ARRAY))
            {
                DB_SetToken(pTokens, kTT_DATA, pParam);
                break;
            }

            if (0 == arraySize++)
                DB_SetToken(pTokens, kTT_ARRAY_START, pParam);
            else
                DB_SetToken(pTokens, kTT_ARRAY,       pParam);

            if (arraySize >= arrayMax)
                break;

            if (! DB_NextToken(pTokens))
                break;

        }

        /* array underflow */

        if (arraySize < arrayMin)
        {
            DB_ErrorToken(pCliEnv);
            return RCC_ERROR_THROW(ERROR_RCC_NO_PARAM_DATA);
        }
    }

    if ((RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_FULL)) ||
        (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_SOME) && ! keyword))
        DB_SetParamIndex(pTokens, index);

    pTokens->allParamCount++;
    pTokens->nodeParamCount++;

    return STATUS_RCC_TOKEN_MATCH;
}

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean
DB_SameNode(cmdNode *pNode, paramDefn *pParam)
{
    sbyte4  index;
    paramDefn *pMatch;

    pMatch = pNode->pParams;
    for (index = 0; index < pNode->numParams; index++, pMatch++)
        if (pMatch == pParam)
            return TRUE;

    return FALSE;
}


/* always returns param -- doesn't have to be entered */

RL_STATIC Boolean
DB_ParamById(paramList *pParamList, cmdNode *pNode, 
             paramID id, paramDefn ** ppParam)
{
    sbyte        index;
    paramDefn   *pParam;
    Boolean      isGlobal;
    Boolean      sameNode = TRUE;
    
    *ppParam = NULL;
    isGlobal = FLAG_SET(pParamList, kRCC_COMMAND_GLOBAL);
    
    for ( ; NULL != pParamList; pParamList = pParamList->pPrev)
    {
        if (isGlobal != FLAG_SET(pParamList, kRCC_COMMAND_GLOBAL))
            continue;

        pParam = pNode->pParams;
        for (index = 0; index < pNode->numParams; index++, pParam++)
        {
            if (id != pParam->id)
                continue;
                
            *ppParam = pParam;
            return sameNode;
        }

        sameNode = FALSE;
    }

    return sameNode; /* shouldn't happen */
}

/*-----------------------------------------------------------------------*/

/* if handler uses parent parameters, make sure they're satisfied first 
 * returns FALSE if missing required -- otherwise, number of required/optional matched
 */

RL_STATIC Boolean
DB_HandlerParentParams(cli_env *pCliEnv, cmdNode *pNode, oHandler *pHandler, 
                       sbyte4 *pRequired, sbyte4 *pOptional)
{
    sbyte4       index;
    sbyte4       required    = 0;
    sbyte4       optional    = 0;
    Boolean      sameNode;
    paramDefn   *pParam      = NULL;
    paramEntry  *pEntry      = NULL;

    *pRequired = 0;
    *pOptional = 0;

    if (NULL == (pEntry = HANDLER_PARAMS(pHandler)))
        return FALSE;

    for (index = 0; index < HANDLER_PARAM_COUNT(pHandler); pEntry++, index++)
    {
        pParam = DB_LookupParam(pCliEnv, pNode, pEntry, &sameNode);
        if (sameNode)
            continue;

        if FLAG_SET(pEntry, kRCC_PARAMETER_REQUIRED)
            required++;
        else
            optional++;

        if (NULL == pParam)
            continue;

        if FLAG_SET(pEntry, kRCC_PARAMETER_REQUIRED)
            (*pRequired)++;
        else
            (*pOptional)++;
    }

    return (required == *pRequired);
}

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean
DB_HandlerParam(cli_env *pCliEnv, cmdNode *pNode, 
                Boolean keyword, Boolean exact)
{
    sbyte4       index;
    sbyte4       matches     = 0;
    sbyte4       offset      = 0;
    sbyte4       optional    = 0;
    sbyte4       required    = 0;
    sbyte4       optMax      = 0;
    sbyte4       reqMax      = 0;
    sbyte4       theOffset   = 0;
    sbyte4       paramCount  = 0;
    paramDefn   *pParam      = NULL;
    paramDefn   *pTheParam   = NULL;
    paramList   *pParams     = DB_GetCurrentParamList(pCliEnv);
    tokenTable  *pTokens     = pParams->pTokens;
    oHandler    *pHandler    = (oHandler *) pNode->pHandlers;
    handlerDefn *pTheHandler = NULL;
    Boolean      hasNo       = FALSE;
    Boolean      sameNode;
    paramEntry  *pEntry      = NULL;

    for (index = 0; index < pNode->numHandlers; index++, pHandler++)
    {
        hasNo |= IS_NO_HANDLER(pHandler);

        /* make sure of proper 'no' form*/
        if (INVALID_NO(pNode, pParams, pHandler))
            continue;

        /* pertinent parent parameters must be present */
        if (! DB_HandlerParentParams(pCliEnv, pNode, pHandler, &required, &optional))
            continue;

        if (  (required <  reqMax) ||
             ((required == reqMax) && (optional < optMax)))
            continue;

        if (NULL == (pEntry = HANDLER_PARAMS(pHandler)))
            continue;

        offset  = DB_GetParamIndex(pTokens) + 1;
        pEntry += offset;

        for ( ; offset < HANDLER_PARAM_COUNT(pHandler); pEntry++, offset++)
        {
            pParam = DB_LookupParam(pCliEnv, pNode, pEntry, &sameNode);

            if (NULL == pParam)
                break;

            if (keyword != MPARM_HasKeyword(pParam))
                continue;

            if (! DB_SameNode(pNode, pParam))
                continue;

            if (DB_ParamUsed(pTokens, pParam))
                continue;

            if (STATUS_RCC_TOKEN_MATCH == 
                DB_ValidParam(pCliEnv, pParam, keyword, exact))
            {
                if (pTheParam != pParam)
                    paramCount++;

                pTheParam   = pParam;
                pTheHandler = (handlerDefn *) pHandler;
                theOffset   = offset;
                matches++;

                /* if this is a top score, then this is 
                 * the ONE match we care about 
                 */

                if (  (required >  reqMax) ||
                     ((required == reqMax) && (optional > optMax)))
                {
                    matches++;
                    required = reqMax;
                    optional = optMax;
                }

                break;
            }

            /* not a valid param -- if it's optional, that's okay */

            if (FLAG_SET(pEntry, kRCC_PARAMETER_REQUIRED))
            {
                /* full ordering - anything */
                if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_FULL))              
                    break;

                /* unnamed ordering w/o keyword */
                if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_SOME) && ! keyword)              
                    break;
            }
        }
    }

    if (1 == matches)
        DB_SetParamHandler(pParams, (handlerDefn *) pTheHandler);

    if (1 == paramCount)
        return DB_SaveParameter(pCliEnv, pTheParam, theOffset);

    return OK;
}

/*-----------------------------------------------------------------------*/

/* find handler for this node */

RL_STATIC RLSTATUS
DB_HandlerMatch(cli_env *pCliEnv, cmdNode *pNode, Boolean exact)
{
    RLSTATUS     status;
    paramList   *pParams = DB_GetCurrentParamList(pCliEnv);
    
    /* no handlers -- don't bother */
    if (0 == pNode->numHandlers)
        return OK;

    /* this node does not support 'no' commands */
    if ( FLAG_SET(pParams, kRCC_COMMAND_NO) && 
         FLAG_CLEAR(pNode, kRCC_COMMAND_NO) )
    {
        DB_SetErrorNo(pCliEnv, pNode);
        return RCC_ERROR_THROW(ERROR_RCC_INVALID_NO);
    }

    /* only one handler -- we're done */
    if (1 == pNode->numHandlers)
    {
        DB_SetParamHandler(pParams, (handlerDefn *) pNode->pHandlers);
        return OK;
    }

    /* keyword */
    if (OK != (status = DB_HandlerParam(pCliEnv, pNode, TRUE, exact)))
        return status;

    /* no keyword */
    return DB_HandlerParam(pCliEnv, pNode, FALSE, exact);
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS
DB_ParamMatch(cli_env *pCliEnv, Boolean keyword, Boolean exact, 
              paramDefn **ppParam, sbyte4 *pMatches)
{
    RLSTATUS    status      = OK;
    tokenTable *pTokens     = DB_GetTokenTable(pCliEnv);
    cmdNode    *pNode       = DB_GetCurrentNode(pTokens);
    paramDefn  *pParam      = pNode->pParams;
    sbyte4      index       = 0;
    sbyte4      param_index = DB_GetParamIndex(pTokens);
    Boolean     ordered;
    Boolean     unnamed;
    Boolean     full;
    Boolean     sameNode;
    Boolean     required    = FALSE;
#ifdef __RCC_DEBUG__
    sbyte      *pTokenText  = DB_TokenString(pTokens);
#endif

#ifdef __RCC_EXACT_KEYWORDS_ONLY__
    if (keyword && ! exact)
        return OK;
#endif

    if (! exact && FLAG_SET(pNode, kRCC_COMMAND_EXACT_PARAM))
        return OK;

    if (0 > param_index)
        param_index = 0;

    *ppParam  = NULL;
    *pMatches = 0;

    unnamed = RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_SOME);
    full    = RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_FULL);
    ordered = unnamed || full;

    if (ordered) 
    {
        for (; ; param_index++)
        {
            /* is null if we don't have a handler yet */
            pParam = DB_ThisParam(pCliEnv, param_index, keyword, &required, &sameNode);
            if (NULL == pParam)
                return OK;

            if (! sameNode)
                continue;

            if (DB_ParamUsed(pTokens, pParam))
                continue;

            if (STATUS_RCC_TOKEN_MATCH ==
                DB_ValidParam(pCliEnv, pParam, keyword, exact))
            {
                (*pMatches)++;
                *ppParam = pParam;
                if (keyword && ! exact)
                    DB_AddMatch(pCliEnv, pParam->pKeyword);

                continue;
            }

            if (unnamed && MPARM_HasKeyword(pParam))
                continue;

            if (required)
                return OK;
        }
    }
    else
    {
        for (index = 0; index < pNode->numParams; index++, pParam++)
        {
            if (DB_ParamUsed(pTokens, pParam))
                continue;

            if (STATUS_RCC_TOKEN_MATCH != 
                DB_ValidParam(pCliEnv, pParam, keyword, exact))
                continue;

            (*pMatches)++;

            if (NULL == *ppParam)
                *ppParam = pParam;
        
            /* there can be only one exact match */

            if (exact) 
                break;
        }

        if (1 == *pMatches)
            status = STATUS_RCC_TOKEN_MATCH;

    }

    return status;
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS 
DB_NodeOrParam(cli_env *pCliEnv)
{
    RLSTATUS    status       = OK;
    tokenTable *pTokens      = DB_GetTokenTable(pCliEnv);
    sbyte4      exactNode    = 0;
    sbyte4      partialNode  = 0;
    sbyte4      exactParam   = 0;
    sbyte4      partialParam = 0;
    cmdNode    *pNode        = DB_GetCurrentNode(pTokens);
    paramDefn  *pParam       = NULL;
#ifdef __RCC_DEBUG__
    sbyte      *pTokenText   = DB_TokenString(pTokens);
#endif

    /* already know it's ambiguous, no need to try again */
    if (FLAG_SET(pTokens, kRCC_PFLAG_AMBIGUOUS))
        return OK;

    /* this is not applicable until we have the first command */
    if (FLAG_CLEAR(pTokens, kRCC_PFLAG_HAVE_COMMAND))
        return OK;

    /* no conflict either if no params or no child nodes */
    if ((0 >= pNode->numParams) || (0 >= pNode->numChildren))
        return OK;

    /* exact node name match */
    DB_NodeMatch(pCliEnv, pNode, FALSE, TRUE, &exactNode);
    if (1 != exactNode)
    {
        /* look for partial node match */
        DB_NodeMatch(pCliEnv, pNode, FALSE, FALSE, &partialNode);
    }

    /* exact keyword parameter */
    status = DB_ParamMatch(pCliEnv, TRUE, TRUE, &pParam, &exactParam);
    if (DB_ERROR(status))
        return OK; /* status? */

    if (1 != exactParam)
    {
        /* partial keyword parameter */
        status = DB_ParamMatch(pCliEnv, TRUE, FALSE, &pParam, &partialParam);
        if (DB_ERROR(status))
            return status;
    }

    if (0 == exactParam)
    {
        /* unnamed param -- enum, perhaps? */
        status = DB_ParamMatch(pCliEnv, FALSE, TRUE, &pParam, &exactParam);
        if (DB_ERROR(status))
            return status;
    }

    /* we have a winner: this could be a node or a param */
    if (((1 == exactParam)   && (1 == exactNode)) ||
        ((1 == partialParam) && (1 == partialNode)))
    {
        SET_FLAG(pTokens, kRCC_PFLAG_AMBIGUOUS);
        DB_SetPriorNode(pTokens, pNode);
        DB_SetPriorIndex(pTokens, DB_GetTokenIndex(pTokens));
        return OK;
    }

    if ((1 == exactParam) || (1 == exactNode))
        status = OK;
    else if ((0 < partialParam) && (0 < partialNode))
        status = ERROR_GENERAL_DATA_AMBIG;
    else if (1 < partialParam)
        status = ERROR_RCC_AMBIGUOUS_PARAM;
    else if (1 < partialNode)
        status = ERROR_RCC_AMBIGUOUS_COMMAND;

    if (OK != status)
        DB_ErrorToken(pCliEnv);

    return status;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
DB_SetErrorNode(cli_env *pCliEnv, cmdNode *pNode)
{
    tokenTable  *pTokens = DB_GetTokenTable(pCliEnv);
    tokenNode   *pToken  = pTokens->tokens;
    sbyte4       index;
    sbyte4       offset;

    for (index = 0; index < pTokens->numTokens; index++, pToken++)
    {
        if (pToken->pNode == pNode)
        {
            offset = pToken->offset + pToken->length + 1;
            MMISC_SetErrorPos(pCliEnv, offset);
            break;
        }
    }
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS 
DB_IsNode(cli_env *pCliEnv, Boolean global, Boolean exact)
{
    tokenTable  *pTokens    = DB_GetTokenTable(pCliEnv);
    sbyte4       matchCount = 0;
    sbyte4       matches    = 0;
    sbyte4       index;
    cmdNode     *pMatchNode = NULL;
    cmdNode     *pParent    = NULL;
    Boolean      matched    = FALSE;
    Boolean      metaParent = FALSE;
    Boolean      multiple   = FALSE;
    Boolean      isNo;
    paramList   *pParams    = NULL;
    RLSTATUS     status     = OK;
    handlerDefn *pHandler;
    handlerDefn *pMatchHandle = NULL;

#ifdef __RCC_DEBUG__
    sbyte       *pTokenText = DB_TokenString(pTokens);
#endif

#ifdef __DISABLE_COMMAND_CHAINING__
    if (FLAG_SET(pTokens, kRCC_PFLAG_HAVE_COMMAND))
        return OK;
#endif

    if (global)
        pParent = RCC_DB_GetRootNode();
    else
        pParent = DB_GetCurrentNode(pTokens);

    if (! global)
        metaParent = FLAG_SET(pParent, kRCC_COMMAND_META);

retry:

    /* if we have_command is it same effective flag? */
    if (global && FLAG_SET(pTokens, kRCC_PFLAG_NO_GLOBAL))
        return OK;


    pMatchNode = DB_NodeMatch(pCliEnv, pParent, global, exact, &matches);

    if (exact)
        matched = (NULL != pMatchNode);
    else
    {
        matchCount += matches;

        if (1 < matchCount)
        {
            DB_ErrorToken(pCliEnv);
            status = ERROR_RCC_AMBIGUOUS_COMMAND;
        }

        matched = ((NULL != pMatchNode) && (1 == matchCount));
    }

    if (! matched)
    {
        if (metaParent)
        {
            pParent = MMISC_GetCurrRoot(pCliEnv);
            metaParent = FALSE;
            goto retry;
        }
        return status;
    }

    /* deal with command chaining */

    if (FLAG_SET(pTokens, kRCC_PFLAG_NO_CHAIN))
    {
        DB_ErrorToken(pCliEnv);
        return RCC_ERROR_THROW(ERROR_RCC_NO_CHAIN);
    }

    if (FLAG_SET(pMatchNode, kRCC_COMMAND_NO_CHAIN))
        SET_FLAG(pTokens, kRCC_PFLAG_NO_CHAIN);
    
    /* check for command aliasing */

    if (RCC_NotEnabled(pCliEnv, kRCC_FLAG_ALIASED))
    {
        /*
         * if we are doing "alias aliasedword" to remove "aliasedword",
         * then we need to start over and not apply aliases to input 
         * text, because "alias aliasedword" will already be treated
         * as "alias aliasedwordreplacement" 
         */
        if (HAS_STRING(MMISC_AliasCmd(pCliEnv)) &&
            0 == COMPARE(pMatchNode->pKeyword, MMISC_AliasCmd(pCliEnv)))
        {
            RCC_EnableFeature(pCliEnv, kRCC_FLAG_ALIASED);
            DB_SetToken(pTokens, kTT_NODE, pMatchNode);
            return STATUS_RCC_NO_ERROR;
        }
    }

    pParams = DB_GetCurrentParamList(pCliEnv);
    if (FLAG_SET(pTokens, kRCC_PFLAG_HAVE_COMMAND))
    {
        DB_NewParamList(pCliEnv, TRUE);
        pParams = pParams->pNext;
    }

    /* if there was node/param ambiguity before and we've made it to the next node, 
     * then it's safe to say that the ambiguity is resolved
     */
    if (NULL == DB_GetPriorNode(pTokens))
        CLEAR_FLAG(pTokens, kRCC_PFLAG_AMBIGUOUS);

    MMISC_SetCmdNode(pCliEnv, pMatchNode);
    DB_SetToken(pTokens, kTT_NODE, pMatchNode);
    DB_SetCurrentNode(pTokens, pMatchNode);
    DB_SetParamIndex(pTokens, kPARAM_INDEX_UNKNOWN);
    DB_SetParamHandler(pParams, NULL);

    pTokens->nodeParamCount = 0;

    SET_FLAG  (pTokens, kRCC_PFLAG_HAVE_COMMAND);
    CLEAR_FLAG(pTokens, kRCC_PFLAG_NEW_LINE);
        
    if (FLAG_SET(pMatchNode, kRCC_COMMAND_META))
    {
        SET_FLAG(pTokens, kRCC_PFLAG_META_COMMAND);
        SET_FLAG(pTokens, kRCC_PFLAG_IS_META);
    }
    else
    {
        SET_FLAG(pTokens, kRCC_PFLAG_NO_GLOBAL);
    }

    /* set handler if we know there's only one */
    if (1 == pMatchNode->numHandlers)
    {

#ifdef __NO_MEANS_NO__
        if ( FLAG_SET(pTokens, kRCC_PFLAG_IS_NO_CMD) &&
            ! IS_NO_HANDLER(pMatchNode->pHandlers))
        {
            DB_SetErrorNo(pCliEnv, pMatchNode);
            return RCC_ERROR_THROW(ERROR_RCC_INVALID_NO);
        }
#endif

        if ( FLAG_CLEAR(pTokens, kRCC_PFLAG_IS_NO_CMD) &&
             IS_NO_HANDLER(pMatchNode->pHandlers))
        {
            MMISC_SetErrorPos(pCliEnv, DB_TokenOffset(pTokens));
            return RCC_ERROR_THROW(ERROR_RCC_MISSING_NO);
        }

        DB_SetParamHandler(pParams, pMatchNode->pHandlers);
    }
    else
    {
        /* only handler of it's kind? */

        isNo     = FLAG_SET(pTokens, kRCC_PFLAG_IS_NO_CMD);
        pHandler = pMatchNode->pHandlers;
        for (index = 0; index < pMatchNode->numHandlers; index++, pHandler++)
        {
            if (isNo != IS_NO_HANDLER(pHandler))
                continue;

            if (NULL != pMatchHandle)
                multiple = TRUE;

            pMatchHandle = pHandler;
        }

        DB_SetParamHandler(pParams, multiple ? NULL : pMatchHandle);
    }

    MMISC_SetCmdNode(pCliEnv, pMatchNode);
    DB_SetParamNode(pParams, pMatchNode);

    return STATUS_RCC_TOKEN_MATCH;
}


/*-----------------------------------------------------------------------*/

extern Boolean 
RCC_DB_IsAssigned(cli_env *pCliEnv, void *item)
{
    sbyte4      index;
    tokenTable *pTokens = DB_GetTokenTable(pCliEnv);
    tokenNode  *token;

    if (NULL == pTokens)
        return FALSE;

    token = pTokens->tokens;
    for (index = 0; index < pTokens->numTokens; index++, token++)
    {
        if (kTT_INVALID == token->type)
            break;

        if (item == token->pNode)
            return TRUE;
    }
    return FALSE;
}

/*-----------------------------------------------------------------------*/

/*
    !!!!! what if param used only by child node? -- bad design by cust? !!!!!

    Parameter match scenarios

    regardless of ordering scheme, if only one handler for node, 
    assign to param list before ever evaluating params

    Unordered - best fit wins
    -------------------------
    - match against named parameters first
    - match against non-string (numeric, enum, etc)
    - match first unassigned unnamed string

    Full ordering
    -------------
    - if one handler set handler before proceding
      if multiple handlers:
        - determine handler against first parameter
          first parameter is first required param, or,
          if only optional, first valid param
      if multiple first param match, iterate until unique
    - handler assigned
    - match directly against next handler param
      if invalid and optional, skip and retry

    Unnamed
    -------
    - determine handler against first param
    - iterate until handler determined
    - handler assigned
    - evaluate against unassigned named parameters
    - evaluate against next required param
 
*/

RL_STATIC RLSTATUS 
DB_IsParameterOrderFull(cli_env *pCliEnv, Boolean exact)
{
    RLSTATUS     status;
    sbyte4       max;
    sbyte4       index;
    sbyte4       offset;
    Boolean      sameNode;
    handlerDefn *pHandler;
    paramEntry  *pEntry;
    paramDefn   *pParam      = NULL;
    tokenTable  *pTokens     = DB_GetTokenTable(pCliEnv);
    cmdNode     *pNode       = DB_GetCurrentNode(pTokens);
    sbyte       *pData       = DB_TokenString(pTokens);

    /* need a node first */
    if (FLAG_CLEAR(pTokens, kRCC_PFLAG_HAVE_COMMAND))
        return OK;

    /* if no handlers for this node, then we'll need to 
     * validate against the node and hope that a child node
     * has a handler that uses the param
     */
    if (0 == pNode->numHandlers)
    {
        pParam = pNode->pParams;
        for (index = 0; index < pNode->numParams; index++, pParam++)
        {
            if (STATUS_RCC_TOKEN_MATCH ==
                DB_ValidParam(pCliEnv, pParam, MPARM_HasKeyword(pParam), exact))
                return DB_SaveParameter(pCliEnv, pParam, kPARAM_INDEX_UNKNOWN);
        }
        return OK;
    }

    if (NULL == (pHandler = RCC_DB_CurrentHandler(pCliEnv)))
        return DB_HandlerMatch(pCliEnv, pNode, exact);

    /* we have a handler assigned */
    pEntry  = HANDLER_PARAMS(pHandler);
    offset  = DB_GetParamIndex(pTokens) + 1;
    max     = HANDLER_PARAM_COUNT(pHandler);
    pEntry  = HANDLER_PARAMS(pHandler);
    pEntry += offset;

    for (index = offset; index < max; index++, pEntry++)
    {
        pParam   = DB_LookupParam(pCliEnv, pNode, pEntry, &sameNode);

        if (NULL == pParam)
        {
            MMISC_SetErrorPos(pCliEnv, DB_TokenOffset(pTokens));
            return RCC_ERROR_THROW(ERROR_RCC_INVALID_PARAM);
        }

        /* this should be redundant */
        if (DB_ParamUsed(pTokens, pParam))
            continue;

        status = DB_ValidParam(pCliEnv, pParam, MPARM_HasKeyword(pParam), exact);
        if (STATUS_RCC_TOKEN_MATCH == status)
            return DB_SaveParameter(pCliEnv, pParam, index);

        if (FLAG_CLEAR(pEntry, kRCC_PARAMETER_REQUIRED))
            continue;

        if (status != OK)
            return status;

        if (MPARM_HasKeyword(pParam))
            return OK;

        status = DB_Validate(pCliEnv, pParam, pData, TRUE);
        return (STATUS_RCC_TOKEN_MATCH == status) ? OK : status;
    }

    return OK;
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS 
DB_IsParameterOrderUnnamed(cli_env *pCliEnv, Boolean exact)
{
    RLSTATUS     status      = OK;
    sbyte4       matches     = 0;
    sbyte4       index;
    sbyte4       max;
    sbyte4       offset;
    handlerDefn *pHandler;
    paramEntry  *pEntry;
    Boolean      sameNode;
    paramDefn   *pParam      = NULL;
    tokenTable  *pTokens     = DB_GetTokenTable(pCliEnv);
    cmdNode     *pNode       = DB_GetCurrentNode(pTokens);
    Boolean      required;
    sbyte       *pData       = DB_TokenString(pTokens);

    /* need a node first */
    if (FLAG_CLEAR(pTokens, kRCC_PFLAG_HAVE_COMMAND))
        return OK;

    /* if no handlers for this node, then we'll need to 
     * validate against the node and hope that a child node
     * has a handler that uses the param
     */
    if (0 == pNode->numHandlers)
    {
        pParam = pNode->pParams;
        for (index = 0; index < pNode->numParams; index++, pParam++)
        {
            if (STATUS_RCC_TOKEN_MATCH ==
                DB_ValidParam(pCliEnv, pParam, MPARM_HasKeyword(pParam), exact))
                return DB_SaveParameter(pCliEnv, pParam, kPARAM_INDEX_UNKNOWN);
        }
        return OK;
    }

    if (NULL == (pHandler = RCC_DB_CurrentHandler(pCliEnv)))
        return DB_HandlerMatch(pCliEnv, pNode, exact);

    /* keyword match */
    status = DB_ParamMatch(pCliEnv, TRUE, exact, &pParam, &matches);
    if (STATUS_RCC_TOKEN_MATCH == status)
        return DB_SaveParameter(pCliEnv, pParam, kPARAM_INDEX_UNKNOWN);

    if (OK != status)
        return status;

    if (1 < matches)
    {
        DB_ErrorToken(pCliEnv);
        return RCC_ERROR_THROW(ERROR_RCC_AMBIGUOUS_PARAM);
    }

    if (1 == matches)
        return DB_SaveParameter(pCliEnv, pParam, kPARAM_INDEX_UNKNOWN);

    /* 
     * match parameters without keywords - not string params, 
     * e.g., enums, lists
     */

    if (NULL != pHandler)
    {
        offset  = DB_GetParamIndex(pTokens);
        if (kPARAM_INDEX_UNKNOWN == offset)
            offset = 0;

        max     = HANDLER_PARAM_COUNT(pHandler);
        pEntry  = HANDLER_PARAMS(pHandler);
        pEntry += offset;

        for (index = offset; index < max; index++, pEntry++)
        {
            required = FLAG_SET(pEntry, kRCC_PARAMETER_REQUIRED);
            pParam   = DB_LookupParam(pCliEnv, pNode, pEntry, &sameNode);

            if (NULL == pParam)
                continue;

            if (MPARM_HasKeyword(pParam))
                continue;

            if (DB_ParamUsed(pTokens, pParam))
                continue;

            if (STATUS_RCC_TOKEN_MATCH ==
                DB_ValidParam(pCliEnv, pParam, MPARM_HasKeyword(pParam), exact))
                return DB_SaveParameter(pCliEnv, pParam, index);

            if (ENUMLIST(pParam->type) && ! exact)
            {
                if (1 < DB_EnumMatch(pCliEnv, pParam, pData))
                {
                    DB_ErrorToken(pCliEnv);
                    return RCC_ERROR_THROW(ERROR_RCC_AMBIGUOUS_PARAM);
                }
            }

            if (required)
                return OK;
        }
    }

    if (NULL == pHandler)
    {
        offset = DB_GetParamIndex(pTokens);
        max    = pNode->numParams;
        pParam = pNode->pParams;
 
        for (index = 0; index < max; index++, pParam++)
        {
            if (MPARM_HasKeyword(pParam))
                continue;

            if (0 < offset--)
                continue;

            if (STATUS_RCC_TOKEN_MATCH == DB_ValidParam(pCliEnv, pParam, MPARM_HasKeyword(pParam), exact))
                return DB_SaveParameter(pCliEnv, pParam, kPARAM_INDEX_UNKNOWN);
        }
    }

    return OK;
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS 
DB_IsParameterOrderNone(cli_env *pCliEnv, Boolean exact)
{
    RLSTATUS     status      = OK;
    sbyte4       matchCount  = 0;
    paramDefn   *pParam      = NULL;
    tokenTable  *pTokens     = DB_GetTokenTable(pCliEnv);

#ifdef __RCC_DEBUG__
    sbyte       *tokenText   = DB_TokenString(pTokens); /* debugging only */
#endif

    /* need a node first */
    if (FLAG_CLEAR(pTokens, kRCC_PFLAG_HAVE_COMMAND))
        return OK;

    status = DB_ParamMatch(pCliEnv, TRUE, exact, &pParam, &matchCount);
    if (STATUS_RCC_TOKEN_MATCH == status)
        return DB_SaveParameter(pCliEnv, pParam, kPARAM_INDEX_UNKNOWN);

    if (OK != status)
        return status;

    if (1 < matchCount)
    {
        DB_ErrorToken(pCliEnv);
        return RCC_ERROR_THROW(ERROR_RCC_AMBIGUOUS_PARAM);
    }

    if (1 == matchCount)
        return DB_SaveParameter(pCliEnv, pParam, kPARAM_INDEX_UNKNOWN);

    /* match parameters without keywords */
    status = DB_ParamMatch(pCliEnv, FALSE, exact, &pParam, &matchCount);
    if (STATUS_RCC_TOKEN_MATCH == status)
        return DB_SaveParameter(pCliEnv, pParam, kPARAM_INDEX_UNKNOWN);

    return status;
}

/*-----------------------------------------------------------------------*/

/* exit current level */
extern RLSTATUS RCC_DB_Exit(cli_env *pCliEnv, Boolean exitAll)
{
    sbyte4       modeDepth  = MMISC_GetModeDepth(pCliEnv);
    cmdNode     *pNewRoot   = NULL;
#ifdef __SNMP_API_ENABLED__
    Boolean      flushed     = FALSE;
#endif

    while (1)
    {
#ifdef __SNMP_API_ENABLED__
        if (! flushed)
        {
            OCSNMP_UndoSetList(pCliEnv);
            flushed = TRUE;
        }
#endif
        if (0 > --modeDepth)
            modeDepth = 0;
        
        MMISC_SetModeDepth(pCliEnv, modeDepth);

        DB_DeleteParamList(pCliEnv);

        if (! exitAll)
            break;

        if (0 >= modeDepth)
            break;
    }

    pNewRoot = RCC_DB_GetCommandNode(pCliEnv);
    MMISC_SetCurrRoot(pCliEnv, pNewRoot);
    RCC_DisableFeature(pCliEnv, kRCC_FLAG_MYPROMPT);
    RCC_EnableFeature(pCliEnv, kRCC_FLAG_NEWMODE);
    RCC_UTIL_UpdatePrompt(pCliEnv);
    return OK;
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS
DB_OtherFork(cli_env *pCliEnv)
{
    tokenTable  *pTokens = DB_GetTokenTable(pCliEnv);
    sbyte4       offset  = DB_GetPriorIndex(pTokens);
    IsParameter *pfParam = MMISC_GetParamCheck(pCliEnv);
    cmdNode     *pNode   = DB_GetPriorNode(pTokens);
    paramList   *pParamList;

    RLSTATUS     status;

    if (FLAG_CLEAR(pTokens, kRCC_PFLAG_AMBIGUOUS) ||
        FLAG_SET  (pTokens, kRCC_PFLAG_NO_REPEAT))
        return OK;

    /* erase failed portion */
    DB_SetTokenType(pTokens, kTT_INVALID);
    DB_SetTokenNode(pTokens, NULL);
             
    while (offset < DB_GetTokenIndex(pTokens)) 
    {
        if (! DB_PrevToken(pTokens))
            break;             

        DB_SetTokenType(pTokens, kTT_INVALID);
        DB_SetTokenNode(pTokens, NULL);
    }

    /* ambiguous token wasn't a node - is it a param? */
    SET_FLAG(pTokens, kRCC_PFLAG_NO_REPEAT);
    DB_SetCurrentNode(pTokens, pNode);
    DB_SetPriorNode(pTokens, NULL);

    /* delete other fork's param list entries */
    pParamList = DB_GetCurrentParamList(pCliEnv);
    while (NULL != pParamList)
    {
        if (pNode == pParamList->pCmdNode)
            break;

        if (FLAG_CLEAR(pParamList, kRCC_COMMAND_EXTEND))
            FREEMEM(pParamList->pTokens);

        if (NULL == pParamList->pPrev)
            break;

        pParamList = pParamList->pPrev;
        FREEMEM(pParamList->pNext);        
    }
    pTokens->pParamList = pParamList;

    if (OK != (status = pfParam(pCliEnv, TRUE)))
        return status;

    return pfParam(pCliEnv, FALSE); 
}

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean DB_BuiltIn(tokenTable *pTokens)
{
    if (kTOKEN_QUOTED == DB_TokenFlags(pTokens))
        return FALSE;

    /* to support future use of ";" for multi commands per line */
    if (kTT_END_COMMAND == DB_TokenType(pTokens))
    {
        SET_FLAG(pTokens, kRCC_PFLAG_NEW_LINE);
        return TRUE;
    }

    /* a "no" command? */
    if (DB_TokenMatches(pTokens, kRCC_CMD_NO))
    {
        DB_SetToken(pTokens, kTT_NO, NULL);
        SET_FLAG(pTokens, kRCC_PFLAG_IS_NO_CMD);
        return TRUE;
    }
    return FALSE;
}

/*-----------------------------------------------------------------------*/

    /* print entry */
static void
printPrefix(cli_env * pCliEnv, ubyte4 mask, sbyte4 depth)
{
    sbyte4 index;

    for (index = 1; index < depth; index++)
    {
        if (mask & 1)
            RCC_EXT_WriteStr(pCliEnv, "|   ");
        else
            RCC_EXT_WriteStr(pCliEnv, "    ");

        mask = mask >> 1;
    }
}

/*-----------------------------------------------------------------------*/

extern void
RCC_DB_ShowTree(cli_env *pCliEnv, cmdNode *pNode, sbyte4 depth, 
                ubyte4 mask, ubyte4 flags)
{
    sbyte4   index;
    ubyte4   tempMask = mask;
    cmdNode *pChild;

    if (NULL == pNode)
        return;

    /* spacer between entries */
    printPrefix(pCliEnv, mask, depth);

    if ((0 < depth) && (OK != RCC_EXT_WriteStrLine(pCliEnv, "|")))
        return;

    /* print entry */
    printPrefix(pCliEnv, mask, depth);

    if (0 < depth)
        RCC_EXT_WriteStr(pCliEnv, "+---");

    if (kTREE_FLAG_SYNTAX & flags)
    {
        MEDIT_SetIndent(pCliEnv, MEDIT_GetXPos(pCliEnv)); 
        RCC_DB_ShowCommand(pCliEnv, pNode);
        MEDIT_SetIndent(pCliEnv, 0); 
        RCC_EXT_WriteStrLine(pCliEnv, "");
    }
    else
    {
        if (OK != RCC_EXT_WriteStrLine(pCliEnv, pNode->pKeyword))
            return;
    }

    pChild = pNode->pChildren;
    for (index = 1; index <= pNode->numChildren; index++, pChild++)
    {
        tempMask = mask;

        if (index != pNode->numChildren)
            tempMask |= 1 << depth;

        RCC_DB_ShowTree(pCliEnv, pChild, depth + 1, tempMask, flags);
    }
}

/*-----------------------------------------------------------------------*/
RL_STATIC RLSTATUS DB_Match(cli_env *pCliEnv, Boolean exact)
{
    RLSTATUS     status;
    IsParameter *pfParam = MMISC_GetParamCheck(pCliEnv);

    /* Local command? */
    if (OK != (status = DB_IsNode(pCliEnv, FALSE, exact)))
        return status;

    /* Global command? */
    if (OK != (status = DB_IsNode(pCliEnv, TRUE, exact)))
        return status;

    return pfParam(pCliEnv, exact);
}

/*-----------------------------------------------------------------------*/

/* does the given parameter jibe with possible handlers/ordering? */
extern Boolean
RCC_DB_ParamPossible(cli_env *pCliEnv, cmdNode *pNode, paramDefn *pParam)
{
    tokenTable  *pTokens     = DB_GetTokenTable(pCliEnv);
	oHandler    *pHandler    = (oHandler *) pNode->pHandlers;
	paramEntry  *pEntry;
	paramDefn   *pTest;
	sbyte4		 index;
    sbyte4       loop;
    sbyte4       start;
	Boolean		 sameNode;

    start = pTokens->nodeParamCount;
	for (index = 0; index < pNode->numHandlers; index++, pHandler++)
	{
		pEntry  = pHandler->pParams;
		pEntry += start;
        for (loop = start; loop < pHandler->paramCount; loop++, pEntry++)
        {
    		pTest = DB_LookupParam(pCliEnv, pNode, pEntry, &sameNode);
	    	if (pTest == pParam)
		    	return TRUE;

            if (FLAG_SET(pEntry, kRCC_PARAMETER_REQUIRED))
                break;
        }
	}

	return FALSE;
}

/*-----------------------------------------------------------------------*/

/* Assign each token to a node, parameter keyword or parameter data */
extern RLSTATUS RCC_DB_Parameterize(cli_env *pCliEnv)
{
    tokenTable  *pTokens     = DB_GetTokenTable(pCliEnv);
    RLSTATUS     status      = OK;
    sbyte4       paramMax;
    sbyte4       paramCurrent;
    sbyte       *tokenText; /* debugging only */
    cmdNode     *pNode;

    /* ignore if empty line */
    if (0 >= DB_TokenCount(pTokens))
        return OK;

    DB_FirstToken(pTokens);
    DB_SetCurrentNode(pTokens, MMISC_GetCurrRoot(pCliEnv));
    DB_ResetErrorText(pCliEnv);
    SET_FLAG(pTokens, kRCC_PFLAG_NEW_LINE);

    while (1)
    {
        if ((OK != status) && (STATUS_RCC_TOKEN_MATCH != status))
            break;

        if ((! DB_InvalidToken(pTokens)) && (! DB_NextToken(pTokens)))
            break;

        if (FLAG_CLEAR(pTokens, kRCC_PFLAG_AMBIGUOUS))
            DB_ResetMatches(pCliEnv);

        if (DB_BuiltIn(pTokens))
            continue;

        tokenText = DB_TokenString(pTokens); /* debugging only */

        /* yuck: is it a case of node _or_ param? */ 
        if (OK != (status = DB_NodeOrParam(pCliEnv)))
            continue;
        
        /* Exact matches */

        if (OK != (status = DB_Match(pCliEnv, TRUE)))
            continue;

        /* Partial matches */

        if (OK != (status = DB_Match(pCliEnv, FALSE)))
            continue;

        /* if nothing more to the meta command itself,
           get the node it is refering to */
        if (FLAG_SET(pTokens, kRCC_PFLAG_META_COMMAND))
        {
            CLEAR_FLAG(pTokens, kRCC_PFLAG_META_COMMAND);
            DB_SetCurrentNode(pTokens, MMISC_GetCurrRoot(pCliEnv));
            status = DB_IsNode(pCliEnv, FALSE, TRUE);
            continue;
        }

        /* we have a "bad" token - try the other fork if ambiguous */
        if (OK != (status = DB_OtherFork(pCliEnv)))
            continue;

        /*      an unmatched token 
         *
         * mark the problem token for error handling
         *
         */

        pNode = DB_GetCurrentNode(pTokens);
        MMISC_SetCmdNode(pCliEnv, pNode);
        DB_ErrorToken(pCliEnv);

        if (FLAG_CLEAR(pTokens, kRCC_PFLAG_HAVE_COMMAND))
            return RCC_ERROR_THROW(ERROR_RCC_BAD_COMMAND);

        /* do we have a handler and know how many params were expected? */
        if ((NULL != pTokens->pParamList) && (NULL != pTokens->pParamList->pHandler))
        {
            paramMax     = HANDLER_PARAM_COUNT(pTokens->pParamList->pHandler);
            paramCurrent = DB_GetParamIndex(pTokens);

            if (paramCurrent + 1 >= paramMax)
                return RCC_ERROR_THROW(ERROR_RCC_EXTRA_PARAMS);

            return RCC_ERROR_THROW(ERROR_RCC_INVALID_PARAM); 
        }

        if (pTokens->pCurrentNode->numParams > pTokens->nodeParamCount)
            return RCC_ERROR_THROW(ERROR_RCC_INVALID_PARAM);

        return RCC_ERROR_THROW(ERROR_RCC_EXTRA_PARAMS);

    } /* while (1) */

    /* meta command, e.g., help, will deal w/ ambiguity */
    if ((ERROR_RCC_AMBIGUOUS_COMMAND == status) &&
        FLAG_SET(pTokens, kRCC_PFLAG_META_COMMAND))
        return OK;

    if (STATUS_RCC_TOKEN_MATCH == status)
        return OK;
        
    if (OK != status)
        return status;

    if (FLAG_SET(pTokens, kRCC_PFLAG_AMBIGUOUS))
        return RCC_ERROR_THROW(ERROR_GENERAL_DATA_AMBIG);
   
    return status;
}

/*-----------------------------------------------------------------------*/

extern void RCC_DB_PrintType(cli_env *pCliEnv, paramDefn  *pParam)
{
    sbyte  *type;
    sbyte   pString[kRCC_ENUM_MSG_SIZE];

    switch (pParam->type)
    {
    case kDTinteger:
        DB_RangeText(pParam, pString, kRCC_ENUM_MSG_SIZE);
        if (HAS_STRING(pString))
            RCC_EXT_WriteStr(pCliEnv, pString);
        else
        {
            type = CONVERT_GetDTName(pParam->type);
            if (NULL != type)
                RCC_EXT_WriteStr(pCliEnv, type);
        }
        break;
    case kDTenum:
        CONVERT_EnumStr(pParam->pParamInfo, pString, kRCC_ENUM_MSG_SIZE, " | ");
        RCC_EXT_WriteStr(pCliEnv, pString);
        break;
    case kDTlist:
        CONVERT_ListToStr(-1, pString, pParam->pParamInfo);
        RCC_EXT_WriteStr(pCliEnv, pString);
        break;
    default:
        type = CONVERT_GetDTName(pParam->type);
        if (NULL != type)
            RCC_EXT_WriteStr(pCliEnv, type);
        break;
    }
}

/*-----------------------------------------------------------------------*/

extern void RCC_DB_PrintRM(cli_env *pCliEnv, sbyte *pRapidMark)
{
    sbyte  pBuffer[kMagicMarkupBufferSize];
    Length bufSize = kMagicMarkupBufferSize;

    if (OK != RCC_RCB_ReadValueFromRCB(pCliEnv, pRapidMark, NULL, 
                                       pBuffer, &bufSize))
    {
        /* clear failed rapidmark message */
        DB_ResetErrorText(pCliEnv);
        return;
    }

    RCC_EXT_Write(pCliEnv, pBuffer, bufSize);
    RCC_EXT_WriteStrLine(pCliEnv, "");
}

/*-----------------------------------------------------------------------*/

extern sbyte4 
RCC_DB_GetArraySize(paramDescr *pParamDesc)
{
    if (NULL == pParamDesc)
        return -1;

    return pParamDesc->arraySize;
}


/*-----------------------------------------------------------------------*/

extern sbyte* 
RCC_DB_GetArrayElement(paramList *pParamList, paramDescr *pParamDesc, sbyte4 offset)
{
    if (NULL == pParamDesc)
        return NULL;

    if ((0 > offset) || (offset >= pParamDesc->arraySize))
        return NULL;

    offset += pParamDesc->arrayStart;
    return DB_GetTokenString(pParamList->pTokens, offset);
}


/*-----------------------------------------------------------------------*/

extern Boolean 
RCC_DB_IsNoCommand(paramList *pParamList)
{
    return FLAG_SET(pParamList, kRCC_COMMAND_NO);
}

/*-----------------------------------------------------------------------*/

extern void 
RCC_DB_ErrorLine(cli_env *pCliEnv)
{
    sbyte4      offset  = MMISC_GetErrorPos(pCliEnv);
    sbyte       lineChar;

    if (kERROR_HANDLED == offset)
        return;

    offset += MEDIT_GetPromptLen(pCliEnv);

    lineChar = RCC_IsEnabled(pCliEnv, kRCC_FLAG_DASHES) ? '-' : ' ';

    RCC_EXT_PrintString(pCliEnv, "", offset, lineChar);
    RCC_EXT_WriteStrLine(pCliEnv, "^");

    MMISC_SetErrorPos(pCliEnv, kERROR_HANDLED);
}


extern RLSTATUS 
RCC_DB_EnvironmentReset(cli_env *pCliEnv)
{
    RLSTATUS    status = OK;
    cli_info   *pCli;

    if (NULL == pCliEnv)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    MMISC_SetCurrRoot(pCliEnv,          RCC_DB_GetRootNode());
    MMISC_SetParamCheck(pCliEnv,        PARAMETER_DEFAULT);
    RCC_EXT_OutputReset(pCliEnv);

    /* don't memset/restore? just reapply defaults? */
    pCli       = CLIENV(pCliEnv);

#ifndef __DISABLE_STRUCTURES__
    status = Cache_Destruct(pCliEnv, SESSION_CACHE_GET_M(pCliEnv));
    if( OK > status )
        return status;

    status = Cache_Construct(SESSION_CACHE_GET_M(pCliEnv), kCacheReadWrite);
    if( OK > status )
        return status;
#endif

#ifdef __SNMP_API_ENABLED__
    status = OCSNMP_ConstructSnmpEnviron(pCliEnv);
    if( OK > status )
        return status;
#endif

#ifdef __ANSI_FILE_MANAGER_ENABLED__
#ifdef kRCC_DEFAULT_PATH
    ANSIFS_Chdir(pCliEnv, kRCC_DEFAULT_PATH);
#endif
#endif

    return status;

} /* RCC_DB_EnvironmentReset */


/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_DB_EnvironmentCreate(cli_env **ppCliEnv, CliChannel *pChannel, 
                         sbyte *pBuffer, sbyte4 size)
{
    RLSTATUS      status;
    sbyte        *pInputBuffer      = NULL;
    cli_info     *pCliInfo          = NULL;
    environment  *pNewEnv           = NULL;
    OS_SPECIFIC_SOCKET_HANDLE sock  = 0;

    if (NULL == ppCliEnv)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    if (NULL != (pNewEnv = *ppCliEnv))
        return OK;

    if (NULL != pChannel)
        sock = pChannel->sock;

    if (OK != (status = ENVIRONMENT_Construct(sock, &pNewEnv)))
        return status;

    pCliInfo     = RC_CALLOC(sizeof(*pCliInfo), 1);
    pInputBuffer = RC_MALLOC(kRCC_MAX_CMD_LEN);
    SESSION_CONSUMER_SET_M(pNewEnv, pCliInfo);

    if ((NULL == pCliInfo) || (NULL == pInputBuffer))
    {
        status = SYS_ERROR_NO_MEMORY;
        goto createError;
    }

	MMISC_EnvBladeSet(pNewEnv,      kBTcli);

    MMISC_SetChannel(pNewEnv, pChannel);
    MEDIT_SetBufPtr(pNewEnv,  pInputBuffer);
    MEDIT_SetBufSize(pNewEnv, kRCC_MAX_CMD_LEN);

#ifndef __DISABLE_STRUCTURES__
    status = Cache_Construct(SESSION_CACHE_GET_M(pNewEnv), kCacheReadWrite);
    if( OK > status )
        goto createError;
#endif

#ifdef __SNMP_API_ENABLED__
    status = OCSNMP_ConstructSnmpEnviron(pNewEnv);
    if( OK > status )
        goto createError;
#endif

    if (OK != (status = RCC_HIST_InitHistInfo(pNewEnv)))
        goto createError;

    if (OK != (status = DB_NewParamList(pNewEnv, FALSE)))
        goto createError;

    if (NULL != pChannel)
        pChannel->env = pNewEnv;

    *ppCliEnv = pNewEnv;

#ifdef __ANSI_FILE_MANAGER_ENABLED__
#ifdef kRCC_DEFAULT_PATH
    ANSIFS_Init(kRCC_DEFAULT_PATH);
#endif
#endif

    return status;

createError:

    RCC_DB_EnvironmentDestroy(pNewEnv);
    return RCC_ERROR_THROW(status);
}

/*-----------------------------------------------------------------------*/

/* show system status */

void RCC_DB_SystemSettings(cli_env * pCliEnv)
{
    sbyte     access[128];
    sbyte     buff[128];
    helpInfo *pHelpInfo = MMISC_GetHelpPtr(pCliEnv);
    sbyte    *termType  = MCONN_TermType(pCliEnv);
    EditType  width     = MSCRN_GetWidth(pCliEnv);
    EditType  height    = MSCRN_GetHeight(pCliEnv);
    sbyte    *pWrap;
    sbyte    *pOrder;

    extern DTTypeInfo mAccessInfo;

    pWrap = RCC_IsEnabled(pCliEnv, kRCC_FLAG_HARDWRAP) ? "On" : "Off";
    CONVERT_AccessToStr(MMISC_GetAccess(pCliEnv), access, &mAccessInfo);
    RCC_EXT_OutputReset(pCliEnv);

    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_NONE))
        pOrder = "none";
    else if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_SOME))
        pOrder = "unnamed";
    else if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_FULL))
        pOrder = "full";
    else 
        pOrder = "unknown";

    RCC_EXT_WriteStrLine(pCliEnv, "");
    RCC_EXT_WriteStrLine(pCliEnv, "General Settings");
    RCC_EXT_WriteStrLine(pCliEnv, "----------------");

    WRITE_TEXT(pCliEnv, buff, "User name",     MMISC_Login(pCliEnv));
    WRITE_TEXT(pCliEnv, buff, "Access",        access);
    WRITE_TEXT(pCliEnv, buff, "Terminal Type", termType);
    WRITE_NUMB(pCliEnv, buff, "Screen width",  width);
    WRITE_NUMB(pCliEnv, buff, "Screen height", height);
    WRITE_TEXT(pCliEnv, buff, "Hard wrap",     pWrap);
    WRITE_TEXT(pCliEnv, buff, "Ordering",      pOrder);

    RCC_EXT_WriteStrLine(pCliEnv, "");
    RCC_EXT_WriteStrLine(pCliEnv, "Help Variables");
    RCC_EXT_WriteStrLine(pCliEnv, "--------------");

    WRITE_NUMB(pCliEnv, buff, "Width",          pHelpInfo->width);
    WRITE_TEXT(pCliEnv, buff, "Node delimiter", pHelpInfo->pNode);
    WRITE_TEXT(pCliEnv, buff, "Delimiter",      pHelpInfo->pDelimiter);
    WRITE_TEXT(pCliEnv, buff, "Title",          pHelpInfo->pTitle);
    WRITE_TEXT(pCliEnv, buff, "Quote",          pHelpInfo->pQuote);
    WRITE_TEXT(pCliEnv, buff, "Unquote",        pHelpInfo->pUnquote);
    WRITE_CHAR(pCliEnv, buff, "Leader",         pHelpInfo->leader);

    RCC_EXT_WriteStrLine(pCliEnv, "");
    RCC_EXT_WriteStrLine(pCliEnv, "Help Switches");
    RCC_EXT_WriteStrLine(pCliEnv, "-------------");

    WRITE_HELP(pCliEnv, buff, "Show 'no'",         HELP_FLAG_SHOW_NO);
    WRITE_HELP(pCliEnv, buff, "Skips blank",       HELP_FLAG_HIDE_BLANK);
    WRITE_HELP(pCliEnv, buff, "Next param(s)",     HELP_FLAG_NEXT);
    WRITE_HELP(pCliEnv, buff, "Show executable",   HELP_FLAG_EXECUTABLE);

    WRITE_HELP(pCliEnv, buff, "Help on error",     HELP_FLAG_ERROR);
    WRITE_HELP(pCliEnv, buff, "Show node help",    HELP_FLAG_THIS);
    WRITE_HELP(pCliEnv, buff, "Shows all params",  HELP_FLAG_PARAMS);
    WRITE_HELP(pCliEnv, buff, "Fixed label width", HELP_FLAG_FIXED_WIDTH);
    WRITE_HELP(pCliEnv, buff, "Hanging indents",   HELP_FLAG_INDENT);
    WRITE_HELP(pCliEnv, buff, "Skip node help",    HELP_FLAG_SKIP_NODE);
    WRITE_HELP(pCliEnv, buff, "Node on same line", HELP_FLAG_SAME_LINE);

    RCC_EXT_OutputReset(pCliEnv);
}

/*-----------------------------------------------------------------------*/

/* this is different from showmatches because that depends upon real 
 * ambiguities, whereas this considers an exact match and partial matches
 * to be valid candidates
 */

extern void
RCC_DB_Possibilities(cli_env *pCliEnv)
{
    tokenTable *pTokens;
    sbyte      *pText;
    sbyte4      index;
    sbyte4      tokenLen;
    paramDefn  *pParam;
    cmdNode    *pParent; 
    cmdNode    *pNode; 
    cmdNode    *pChild;

    /* otherwise, constantly repeating "fills" screen */
    RCC_EXT_OutputReset(pCliEnv);

    pParent = MMISC_GetCurrRoot(pCliEnv);

    /* tokenize command input */
    if (OK != RCC_DB_ParseCmdString(pCliEnv))
        return;

    /* match each token to a node or parameter */

    RCC_DB_Parameterize(pCliEnv);
    RCC_DisableFeature(pCliEnv, kRCC_FLAG_ALIASED);

    pTokens = DB_GetTokenTable(pCliEnv);
    DB_LastToken(pTokens);

    pText    = DB_TokenString(pTokens);
    tokenLen = STRLEN(pText);
    pNode    = DB_GetCurrentNode(pTokens);

    /* current node */
    if ((NULL != pNode) && (NULL != pNode->pKeyword))
    {
        if ((pNode != pParent) && (0 == NCOMPARE(pText, pNode->pKeyword, tokenLen)))
            DB_AddMatch(pCliEnv, pNode->pKeyword);

        pParam = pNode->pParams;
        for (index = 0; index < pNode->numParams; index++, pParam++)
        {
            if (! MPARM_HasKeyword(pParam))
            {
                DB_EnumMatch(pCliEnv, pParam, pParam->pKeyword);
                continue;
            }

            if (0 != NCOMPARE(pText, pParam->pKeyword, tokenLen))
                continue;

            DB_AddMatch(pCliEnv, pParam->pKeyword);
        }
    }

    /* 
     * only applicable if top token, otherwise only params 
     * and children are valid 
     */

    if (0 == DB_GetTokenIndex(pTokens))
    {
        if (NULL != pParent)
        {
            pChild = pParent->pChildren;
            for (index = 0; index < pParent->numChildren; index++, pChild++)
            {
                if (FLAG_SET(pChild, kRCC_COMMAND_GLOBAL))
                    continue;

                if (pNode == pChild)
                    continue;

                if (0 != NCOMPARE(pText, pChild->pKeyword, tokenLen))
                    continue;

                DB_AddMatch(pCliEnv, pChild->pKeyword);
            }
        }

        /* globals */

        if (NULL != (pNode = RCC_DB_GetRootNode ()))
        {
            pChild = pNode->pChildren;
            for (index = 0; index < pNode->numChildren; index++, pChild++)
            {
                if (FLAG_CLEAR(pChild, kRCC_COMMAND_GLOBAL))
                    continue;

                if (0 != NCOMPARE(pText, pChild->pKeyword, tokenLen))
                    continue;

                DB_AddMatch(pCliEnv, pChild->pKeyword);
            }
        }
    }

    DB_ShowMatches(pCliEnv);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_DB_ExpandToken(cli_env *pCliEnv, Boolean sameLine)
{
    RLSTATUS    status;
    tokenTable *pTokens;
    EditType    cursorPos   = MEDIT_GetCursor(pCliEnv);
    sbyte      *cmdString   = NULL;
    sbyte      *match       = NULL;
    sbyte4      tokenLength;

    /* otherwise, constantly repeating "fills" screen */
    RCC_EXT_OutputReset(pCliEnv);

    /* make sure we're expanding _some_ text */
    if (sameLine && ((0 >= cursorPos) || 
       (' ' == RCC_EXT_GetPriorChar(pCliEnv))))
        return RCC_ERROR_THROW(RCC_ERROR);

    /* tokenize command input */
    if (OK != (status = RCC_DB_ParseCmdString(pCliEnv)))
        return status;

    /* match each token to a node or parameter */
    status = RCC_DB_Parameterize(pCliEnv);
    RCC_DisableFeature(pCliEnv, kRCC_FLAG_ALIASED);

    if ((ERROR_GENERAL_DATA_AMBIG    == status) || 
        (ERROR_RCC_AMBIGUOUS_PARAM   == status) ||
        (ERROR_RCC_AMBIGUOUS_COMMAND == status))
    {
        DB_ShowMatches(pCliEnv);
        RCC_DB_ShowInput(pCliEnv);
        return status;
    }

    pTokens = DB_GetTokenTable(pCliEnv);

    /* find which token is being expanded */
    DB_LastToken(pTokens);
    while (cursorPos < DB_TokenOffset(pTokens))
    {
        if (! DB_PrevToken(pTokens))
            break;
    }

    tokenLength = DB_TokenLength(pTokens);
    cmdString   = DB_GetTokenNodeStr(pTokens);

    if (NULL == cmdString)
        cmdString = match;

    /* nothing to expand */
    if (NULL == cmdString)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    if (sameLine)
    {
        cmdString += tokenLength;
        RCC_EXT_InsertText(pCliEnv, cmdString, STRLEN(cmdString));
        RCC_EXT_InsertText(pCliEnv, " ", 1);
    }
    else
    {
        RCC_EXT_WriteStrLine(pCliEnv, "");
        RCC_EXT_WriteStrLine(pCliEnv, cmdString);
        RCC_EXT_WriteStrLine(pCliEnv, "");
        RCC_DB_ShowInput(pCliEnv);
    }
    
    return OK;
}

/*-----------------------------------------------------------------------*/

extern void 
RCC_DB_EnvironmentDestroy(cli_env *pCliEnv)
{
    HistInfo    *pHistory;
    paramList   *pParam; 
    paramList   *pNextParam;
    sbyte       *pBuffer;
    cli_info    *pCli;

    if (NULL == (pCli = CLIENV(pCliEnv)))
        return;

    pHistory   = MHIST_History(pCliEnv);
    pParam     = MMISC_GetRootParam(pCliEnv); 
    pNextParam = NULL;
    pBuffer    = MEDIT_GetBufPtr(pCliEnv);

    while (NULL != pParam)
    {
        pNextParam = pParam->pNext;
        if (FLAG_CLEAR(pParam, kRCC_COMMAND_EXTEND))
            FREEMEM(pParam->pTokens);
        FREEMEM(pParam);
        pParam = pNextParam;
    }

    RCC_EXT_FreeOutput(pCliEnv);
    FREEMEM(pHistory->pHistBuff);
    FREEMEM(pCli);
    FREEMEM(pBuffer);

#ifdef __SNMP_API_ENABLED__
    /* flush uncommited snmp writes */
    OCSNMP_UndoSetList(pCliEnv);
#endif

    ENVIRONMENT_Destruct(&pCliEnv);
}

/*-----------------------------------------------------------------------*/

/* 
 *  Convenience routine to automatically convert parameter data to string
 *  If rapidmark name is included, it will write the data to that rapidmark
 */
extern RLSTATUS 
RCC_DB_ParamValue(cli_env *pCliEnv, paramID id, sbyte *pArgs, 
                  sbyte *pRapidMark, void *pOutput)
{
    RLSTATUS    status;
    paramDescr *pParamDescr;
    paramList  *pParamList = DB_GetCurrentParamList(pCliEnv);

    status = RCC_DB_RetrieveParam(pParamList, NULL, id, &pParamDescr);
    if (OK != status)
        return status;

    if (NULL != pRapidMark)
    {
        status = RCC_RCB_WriteValueToRCB( pCliEnv, pRapidMark, pArgs, pParamDescr->pValue); 
        if ( OK > status )
            return status;
    }
    return CONVERT_StrTo(pParamDescr->pValue, pOutput, pParamDescr->pParamDefn->type);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS
RCC_DB_RetrieveParam(paramList *pParamList, sbyte *pKeyword, 
                     paramID   id, paramDescr **ppParamDescr )
{
    sbyte       index;
    sbyte       loop;
    paramList   *pTempList;
    paramDefn   *pParam;
    paramDescr  *pParamDescr;
    cmdNode     *pNode;
    Boolean      isGlobal;
    
    isGlobal = FLAG_SET(pParamList, kRCC_COMMAND_GLOBAL);
    
    *ppParamDescr = NULL;

    pNode = DB_GetParamNode(pParamList);
    for (pTempList = pParamList; NULL != pTempList; pTempList = pTempList->pPrev)
    {
        if (isGlobal != FLAG_SET(pTempList, kRCC_COMMAND_GLOBAL))
            continue;

        pParamDescr = pTempList->params;

        for ( index = 0; index < pTempList->numParams; index++ )
        {
            if (id == PARAMDESCR_ID(pParamDescr))
            {
                *ppParamDescr = pParamDescr;
                return OK;
            }
            pParamDescr++;
        }
    }

    /* not a parameter on the stack -- find the default and add it */
    for (pTempList = pParamList; NULL != pTempList; pTempList = pTempList->pPrev)
    {
        /* no nodes should be null! we'll play it safe anyway */
        if (NULL == (pNode = DB_GetParamNode(pTempList)))
            continue;

        if (isGlobal != FLAG_SET(pTempList, kRCC_COMMAND_GLOBAL))
            continue;

        pParam = pNode->pParams;

        for (index = 0; index < pNode->numParams; index++, pParam++)
        {
            if (id != PARAM_ID(pParam))
                continue;

            /* append to existing param descriptors */
            pParamDescr = DB_GetParamDesc(pTempList);
            for (loop = 0; loop < pTempList->numParams; loop++)
                pParamDescr++;

            /* no default structures exist */
            if (NULL == pParam->pParamInfo)
                return RCC_ERROR_THROW(ERROR_GENERAL_NOT_FOUND);

            if (NULL_STRING(pParam->pParamInfo->pDefaultStr))
                return RCC_ERROR_THROW(ERROR_GENERAL_NOT_FOUND);

            pTempList->numParams++;
            pParamDescr->pValue     = pParam->pParamInfo->pDefaultStr;
            pParamDescr->pParamDefn = pParam;
            SET_FLAG(pParamDescr, kRCC_PARAMETER_DEFAULT);

            *ppParamDescr = pParamDescr;
            return OK;
        }
    }

    return RCC_ERROR_THROW(ERROR_GENERAL_NOT_FOUND);
}

/*-----------------------------------------------------------------------*/

#ifndef __RCC_PARAMETER_ORDER_NONE__

RL_STATIC paramDescr *
DB_GetParam(paramList *pParamList, paramID id)
{
    sbyte        index;
    paramList   *pTempList   = pParamList;
    paramDescr  *pParamDescr = NULL;
    Boolean      isGlobal    = FLAG_SET(pParamList, kRCC_COMMAND_GLOBAL);
    
    for ( ; NULL != pTempList; pTempList = pTempList->pPrev)
    {
        if (isGlobal != FLAG_SET(pTempList, kRCC_COMMAND_GLOBAL))
            continue;

        pParamDescr = pTempList->params;

        for (index = 0; index< pTempList->numParams; index++, pParamDescr++ )
        {
            if (NULL == pParamDescr->pParamDefn)
                continue;
                
            if (id == pParamDescr->pParamDefn->id)
                return pParamDescr;
        }
    }

    return NULL;
}

#endif /* __RCC_PARAMETER_ORDER_NONE__ */

/*-----------------------------------------------------------------------*/

extern void 
RCC_DB_ShowParameters(cli_env *pCliEnv)
{
    sbyte4      index;
    paramDefn  *pParamDefn;
    paramDescr *pParamDescr;
    sbyte      *pKeyword;
    sbyte      *pValue;
    paramList  *pList = MMISC_GetRootParam(pCliEnv);
    cmdNode    *pNode;
    sbyte       buffer[256];
        
    RCC_EXT_WriteStrLine(pCliEnv, "");
    RCC_EXT_WriteStrLine(pCliEnv, kRCC_MSG_PARAMS_AVAIL);

    while (NULL != pList)
    {
        pParamDescr = DB_GetParamDesc(pList);
        for (index = 0; index < pList->numParams; index++, pParamDescr++)
        {
            if (NULL == pParamDescr) {
                RCC_EXT_WriteStrLine(pCliEnv, kRCC_MSG_NULL_PARAM);
                continue;
            }
            pParamDefn = pParamDescr->pParamDefn;
            pValue     = BLANK_TEXT(pParamDescr->pValue);
            pKeyword   = BLANK_TEXT(pParamDefn->pKeyword);
            pNode      = DB_GetParamNode(pList);

            sprintf(buffer, "Node:%12s Keyword:%12s Value:%s\r\n",
                      pNode->pKeyword, pKeyword, pValue);
            RCC_EXT_WriteStr(pCliEnv, buffer);
        }
        pList = pList->pNext;
    }

    RCC_EXT_WriteStrLine(pCliEnv, "");
}

/*-----------------------------------------------------------------------*/

extern sbyte * 
RCC_DB_LastToken(cli_env *pCliEnv)
{
    tokenTable  *pTokens = DB_GetTokenTable(pCliEnv);

    if (NULL == pTokens)
        return NULL;

    DB_LastToken(pTokens);
    return DB_TokenString(pTokens);
}

/*-----------------------------------------------------------------------*/

/* return current incomplete param (keyword w/o data) */

extern paramDefn * 
RCC_DB_IncompleteParam(cli_env *pCliEnv)
{
    tokenTable  *pTokens    = DB_GetTokenTable(pCliEnv);

    if (kTT_KEYWORD != DB_TokenType(pTokens))
        return NULL;

    return DB_TokenNode(pTokens);
}

/*-----------------------------------------------------------------------*/

#ifdef __RCC_PARAMETER_ORDER_NONE__
RL_STATIC int BitCount(int x)
{
    int i;

    for (i = 0; 0 != x; x = x >> 1)
        i += (x & 1);

    return i;
}

/*-----------------------------------------------------------------------*/
    /* mark parameters as used */

RL_STATIC void
DB_MarkAsUsed(paramList *pParamList)
{
    paramID      valid;
    sbyte4       index;
    paramDescr  *pParamDescr;
    uHandler    *pHandler  = (uHandler *) DB_GetParamHandler(pParamList);

    valid = pHandler->optlParamMask | pHandler->reqdParamMask;
    for ( ; NULL != pParamList; pParamList = pParamList->pPrev)
    {
        pParamDescr = pParamList->params;
        for (index = 0; index < pParamList->numParams; index++, pParamDescr++)
        {
            if (valid & pParamDescr->pParamDefn->id)
                SET_FLAG(pParamDescr, kRCC_PARAMETER_IS_USED);
        }
    }
}

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean   
DB_OptionalRequiredUnordered(paramList *pParamList, handlerDefn *pHand, 
                          sbyte4 *reqCount, sbyte4 *optCount)
{
    uHandler *   pHandler = (uHandler *) pHand;
    BitMask      available;
    paramDescr * pParamDescr;
    paramDefn *  pParam;
    sbyte4       index;
    cmdNode *    pNode;

    available = 0;
    for ( ; NULL != pParamList ; pParamList = pParamList->pPrev)
    {
        /* tally up parameters available */

        pParamDescr = pParamList->params;
        for (index = 0; index < pParamList->numParams; index++, pParamDescr++)
            available |= pParamDescr->pParamDefn->id;

        /* add params w/ default values */

        if (NULL == (pNode = pParamList->pCmdNode))
            continue;

        pParam = pNode->pParams;
        for (index = 0; index < pNode->numParams; index++, pParam++)
        {
            if (NULL == pParam->pParamInfo)
                continue;
                
            if (NULL != pParam->pParamInfo->pDefaultStr)
                available |= pParam->id;
        }

    }

    /* must have required parameters met */
    if (pHandler->reqdParamMask != (available & pHandler->reqdParamMask))
        return FALSE;

    *reqCount = BitCount(pHandler->reqdParamMask & available);
    *optCount = BitCount(pHandler->optlParamMask & available);

    return TRUE;
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS
DB_RequiredUnordered(handlerDefn *pHand, paramDefn *pParam, 
                     Boolean *pRequired)
{
    uHandler *   pHandler = (uHandler *) pHand;

    if (pHandler->reqdParamMask & pParam->id)
    {
        *pRequired = TRUE;
        return OK;
    }
            
    if (pHandler->optlParamMask & pParam->id)
    {
        *pRequired = FALSE;
        return OK;
    }

    return RCC_ERROR_THROW(ERROR_RCC_INVALID_PARAM);
}

/*-----------------------------------------------------------------------*/

#else /* ! __RCC_PARAMETER_ORDER_NONE__ */

/* dummy "function" -- not used w/ parameter ordering */
#define DB_MarkAsUsed(pParamList)


/*-----------------------------------------------------------------------*/

RL_STATIC Boolean   
DB_OptionalRequiredOrdered(paramList *pParamList, handlerDefn *pHandler, 
                          sbyte4 *reqCount, sbyte4 *optCount)
{
    sbyte4       index;
    paramEntry  *pEntry      = pHandler->pParams;
    paramDescr  *pParamDescr = NULL;
    
    *reqCount = 0;
    *optCount = 0;    
    for (index = 0; index < pHandler->paramCount; index++, pEntry++)
    {
        if (NULL == pEntry)
            break;

        pParamDescr = DB_GetParam(pParamList, pEntry->id);
        if (FLAG_SET(pEntry, kRCC_PARAMETER_REQUIRED))
        {
            if (NULL == pParamDescr)
                return FALSE;
            else
                (*reqCount)++;
        }
        else if (NULL != pParamDescr)
            (*optCount)++;

        /* mark param as being used by a handler */
        if (NULL != pParamDescr)
            SET_FLAG(pParamDescr, kRCC_PARAMETER_IS_USED);
    }

    return TRUE;
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS
DB_RequiredOrdered(handlerDefn *pHandler, paramDefn *pParam, 
                   Boolean *pRequired)
{
    sbyte4      index;
    paramEntry *pEntry = pHandler->pParams;

    for (index = 0; index < pHandler->paramCount; index++, pEntry++)
    {
        if (pParam->id != pEntry->id)
            continue;

        *pRequired = FLAG_SET(pEntry, kRCC_PARAMETER_REQUIRED);
        return OK;
    }

    return RCC_ERROR_THROW(ERROR_RCC_INVALID_PARAM);
}


#endif /* ! __RCC_PARAMETER_ORDER_NONE__ */

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS  
DB_GetHandler(cli_env *pCliEnv, paramList *pParamList)
{
    RLSTATUS     status      = OK;
    cmdNode     *pNode;
    handlerDefn *pHandler;
    handlerDefn *pCmdHandler = NULL;
    sbyte4       index       = 0;
    sbyte4       optCount    = -1;
    sbyte4       reqCount    = -1;
    sbyte4       maxReq      = -1;
    sbyte4       maxOpt      = -1;
    Boolean      hasNoHandler = FALSE;

    if (NULL == pParamList)
        return OK;

    if (NULL == (pNode = DB_GetParamNode(pParamList)))
        return OK;

    pHandler = DB_GetParamHandler(pParamList);

    /* node must have 'Allow "No" form' enabled if this is a no command */
    if (IS_NO_COMMAND(pParamList) && FLAG_CLEAR(pNode, kRCC_COMMAND_NO))
    {
        DB_SetErrorNo(pCliEnv, pNode);
        return RCC_ERROR_THROW(ERROR_RCC_INVALID_NO);
    }

    if (0 == pNode->numHandlers)
    {
        if (kNO_MEANS_NO && IS_NO_COMMAND(pParamList))
        {
            DB_SetErrorNo(pCliEnv, pNode);
            return RCC_ERROR_THROW(ERROR_RCC_INVALID_NO);
        }

        return OK;
    }

    /* if we already have a handler, verify we have the params */
    if (NULL != pHandler)
    {
        if (INVALID_NO(pNode, pParamList, pHandler))
        {
            DB_SetErrorNo(pCliEnv, pNode);
            return RCC_ERROR_THROW(ERROR_RCC_INVALID_NO);
        }

        if (FLAG_CLEAR(pNode, kRCC_COMMAND_META) && 
            ! DB_OPTIONAL_REQUIRED_FN(pParamList, pHandler, 
                                      &reqCount, &optCount))
        {
            DB_SetErrorNode(pCliEnv, pNode);
            return RCC_ERROR_THROW(ERROR_RCC_MISSING_PARAM);
        }

        DB_MarkAsUsed(pParamList);
        return OK;
    }

    pHandler = pNode->pHandlers;
    for (index = 0; index < pNode->numHandlers; index++, pHandler++)
    {
        /* if just one handler than it will take no or non-no */
        if (INVALID_NO(pNode, pParamList, pHandler))
            continue;

        hasNoHandler |= IS_NO_HANDLER(pHandler);

        if (! DB_OPTIONAL_REQUIRED_FN(pParamList, pHandler, &reqCount, &optCount))
            continue;

        if ( (reqCount >  maxReq) ||
            ((reqCount >= maxReq) && (optCount > maxOpt)))
        {
            maxReq      = reqCount;
            maxOpt      = optCount;
            pCmdHandler = pHandler;
        }
    }

    /* if we didn't get a handler then must be missing req'd param */
    if (NULL == pCmdHandler)
    {
        DB_SetErrorNode(pCliEnv, pNode);
        if (IS_NO_COMMAND(pParamList) && ! hasNoHandler)
        {
            DB_SetErrorNo(pCliEnv, pNode);
            return RCC_ERROR_THROW(ERROR_RCC_INVALID_NO);
        }

        return RCC_ERROR_THROW(ERROR_RCC_MISSING_PARAM);
    }
    DB_SetParamHandler(pParamList, pCmdHandler);
    DB_MarkAsUsed(pParamList);

    return status;

} /* DB_GetHandler */


extern RLSTATUS RCC_DB_SystemOrder(cli_env *pCliEnv, ubyte4 order)
{
    IsParameter *pfParamCheck = NULL;

#ifdef __RCC_PARAMETER_ORDER_NONE__
    return RCC_ERROR_THROW(ERROR_RCC_BAD_COMMAND);
#endif
    
    switch (order)
    {
        case kRCC_FLAG_ORDER_NONE:
            pfParamCheck = DB_IsParameterOrderNone;
            break;
        case kRCC_FLAG_ORDER_SOME:
            pfParamCheck = DB_IsParameterOrderUnnamed;
            break;
        case kRCC_FLAG_ORDER_FULL:
            pfParamCheck = DB_IsParameterOrderFull;
            break;
        default:
            return RCC_ERROR_THROW(RCC_ERROR);
    }
    RCC_DisableFeature(pCliEnv, kRCC_FLAG_ORDER_MASK);
    RCC_EnableFeature(pCliEnv, order);
    MMISC_SetParamCheck(pCliEnv, pfParamCheck);

    return OK;
}


RL_STATIC Boolean 
DB_ValidHandlerParam(cli_env *pCliEnv, cmdNode *pNode, paramDefn * pParam)
{
    paramList  *pParamList  = DB_GetCurrentParamList(pCliEnv);
    oHandler   *pHandler;
    paramEntry *pEntry;
    sbyte4      index;
    Boolean     sameNode;

#ifdef  __RCC_PARAMETER_ORDER_NONE__
    return FALSE;
#endif

    pHandler = (oHandler *) DB_GetParamHandler(pParamList);
    if (NULL == pHandler)
        return FALSE;

    pEntry   = HANDLER_PARAMS(pHandler);
    for (index = 0; index < HANDLER_PARAM_COUNT(pHandler); index++, pEntry++)
    {
        if (pParam == DB_LookupParam(pCliEnv, pNode, pEntry, &sameNode))
            return TRUE;
    }
    return FALSE;
}


/* make sure we don't have any extra parameters */

RLSTATUS DB_ExtraParams(cli_env *pCliEnv, paramList *pParamList)
{
    sbyte4      index;
    paramDescr *pParamDescr;
    
    for ( ; NULL != pParamList; pParamList = pParamList->pPrev)
    {
        /* this node allows extra parameters */
        if (FLAG_SET(pParamList->pCmdNode, kRCC_COMMAND_IGNORE))
            continue;

        pParamDescr = pParamList->params;
        for (index = 0; index < pParamList->numParams; index++, pParamDescr++)
        {
            if ((FLAG_SET(pParamDescr, kRCC_PARAMETER_IS_USED))   ||
                (FLAG_SET(pParamDescr, kRCC_PARAMETER_DEFAULT)))
                continue;

            /* double-check handler uses it */
            if (DB_ValidHandlerParam(pCliEnv, pParamList->pCmdNode, pParamDescr->pParamDefn))
                continue;

            MMISC_SetErrorPos(pCliEnv, pParamDescr->position);
            return RCC_ERROR_THROW(ERROR_RCC_EXTRA_PARAMS);
        }
    }
    
    return OK;    
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS
RCC_DB_CommandComplete(cli_env *pCliEnv)
{
    return DB_GetHandler(pCliEnv, DB_GetCurrentParamList(pCliEnv));
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_DB_BuildParamList(cli_env *pCliEnv, Boolean exec)
{
    RLSTATUS     status      = OK;
    paramList   *pParamList  = DB_GetCurrentParamList(pCliEnv);
    tokenTable  *pTokens     = DB_GetTokenTable(pCliEnv);
    paramDescr  *pParamDescr = NULL;
    paramDefn   *pParam      = NULL;
    cmdNode     *pNode       = NULL;
    Boolean      noToAll     = kAPPLY_NO_TO_ALL;
    Boolean      haveNode    = FALSE;
    Boolean      noForm      = FALSE;
    Boolean      metaNode    = FALSE;
    Boolean      global      = FALSE;
    Boolean      isArray     = FALSE;
    Boolean      leafExec    = exec && ! kLEAF_NODE_ONLY;
    Boolean      loop        = TRUE;
    Boolean      hasHandler  = FALSE;
    sbyte4       noPosition  = 0;
    sbyte       *pValue;

    if (NULL == pParamList)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    /* back up to first applicable param */
    while (NULL != pParamList->pPrev)
    {
        if (FLAG_CLEAR(pParamList, kRCC_COMMAND_EXTEND))
            break;
            
        pParamList = pParamList->pPrev;
    }

    pParamDescr = DB_GetParamDesc(pParamList);
    DB_FirstToken(pTokens);
    while (loop) 
    {
        switch (DB_TokenType(pTokens)) 
        {
        case kTT_ARRAY_START:
            if (isArray)
                pParamDescr++;
            isArray = TRUE;
            pParamDescr->arrayStart = DB_GetTokenIndex(pTokens);            
            pParamDescr->arraySize  = 0;
        case kTT_ARRAY:
            pParamDescr->arraySize++;
        case kTT_ABSOLUTE:
        case kTT_DATA:
            pValue = DB_TokenString(pTokens);
            pParam = DB_TokenNode(pTokens);

            /* if data is abreviated enum value, replace w/ full enum */
            if (kDTenum == pParam->type)
                pValue = DB_MemberElement(pParam, pValue, FALSE);

            pParamDescr->pParamDefn = pParam;
            pParamDescr->pValue     = pValue;
            pParamDescr->position   = DB_TokenOffset(pTokens);

            if (kTT_ARRAY != DB_TokenType(pTokens))
            {
                pParamList->numParams++;
                if (kTT_ARRAY_START != DB_TokenType(pTokens))
                {
                    pParamDescr++;
                    isArray = FALSE;
                }
            }
            break;
        case kTT_KEYWORD:
            if (isArray)
                pParamDescr++;
            isArray = FALSE;
            break;
        case kTT_NO:
            noForm     = TRUE;
            noPosition = DB_TokenOffset(pTokens);
            break;
        case kTT_END_COMMAND:
        case kTT_NODE:
            pNode = DB_TokenNode(pTokens);
            MMISC_SetCmdNode(pCliEnv, pNode);
            DB_SetParamNode(pParamList, pNode);

            /* flags for successive nodes */
            metaNode |= FLAG_SET(pNode, kRCC_COMMAND_META);
            global   |= FLAG_SET(pNode, kRCC_COMMAND_GLOBAL);

            if (FLAG_SET(pNode, kRCC_COMMAND_META))
                leafExec = exec;

            if (FLAG_SET(pNode, kRCC_COMMAND_NO_TO_ALL))
                noToAll  = TRUE;

            /* 
             * Don't get the handler until all parameters are found
             * -- when we hit another node or run out of tokens.
             */
            if (haveNode)
            {
                if (leafExec)
                {
                    status     = DB_GetHandler(pCliEnv, pParamList);
                    hasHandler = (NULL != DB_GetParamHandler(pParamList));
                    if (OK != status)
                        break;
                }
                if (metaNode)
                    exec = FALSE;

                leafExec = exec && !kLEAF_NODE_ONLY;

                if (NULL == pParamList->pNext)
                    DB_NewParamList(pCliEnv, TRUE);

                pParamList  = pParamList->pNext;
                pParamDescr = DB_GetParamDesc(pParamList);
            }

            if (noForm)
                SET_FLAG(pParamList, kRCC_COMMAND_NO);

            if (!leafExec)
                SET_FLAG(pParamList, kRCC_COMMAND_NO_EXEC);

            if (global)
                SET_FLAG(pParamList, kRCC_COMMAND_GLOBAL);

            RCC_ASSERT(NULL != DB_GetParamNode(pParamList));

            /* reset 'no' unless applying to all nodes */
            noForm   &= noToAll;
            haveNode  = TRUE;

            break;
        default:
            break;
        }

        loop = DB_NextToken(pTokens);
        if (0 < noPosition)
        {
            if ((kTT_NODE != DB_TokenType(pTokens)) || !loop)
            {
                MMISC_SetErrorPos(pCliEnv, noPosition);
                return RCC_ERROR_THROW(ERROR_RCC_INVALID_NO);
            }
        }

        noPosition = 0;
    }

    if (exec)
    {
        status     = DB_GetHandler(pCliEnv, pParamList);
        hasHandler = (NULL != DB_GetParamHandler(pParamList));
        CLEAR_FLAG(pParamList, kRCC_COMMAND_NO_EXEC);
    }
    else
    {
        SET_FLAG(pParamList, kRCC_COMMAND_NO_EXEC);
    }

    if (exec && (OK == status))
        status = DB_ExtraParams(pCliEnv, pParamList);

#ifdef __RCC_MUST_HAVE_HANDLER__
    if ((OK == status) && ! hasHandler)
        return RCC_ERROR_THROW(ERROR_RCC_NO_HANDLER);
#endif

    if (OK != status)
        DB_DeleteParamList(pCliEnv);

    return status;
}

/*-----------------------------------------------------------------------*/

RL_STATIC paramDefn *
DB_NodeParam(cmdNode *pNode, paramID id)
{
    sbyte4       index;
    paramDefn   *pParam = NULL;

    if (NULL == pNode)
        return NULL;
        
    pParam = pNode->pParams;
    for (index = 0; index < pNode->numParams; index++, pParam++)
    {
        if (id & pParam->id)
            return pParam;
    }

    return NULL;
}

/*-----------------------------------------------------------------------*/

/* Find node associated with a parameter bitmask */
extern cmdNode *
RCC_DB_NodeMask(cli_env *pCliEnv, paramID id)
{
    cmdNode    *pNode      = NULL;
    paramList  *pParamList = MMISC_GetRootParam(pCliEnv);
    paramDefn  *pParam;
    sbyte4      index;

    while (NULL != pParamList)
    {
        pNode  = DB_GetParamNode(pParamList);
        pParam = pNode->pParams;
        for (index = 0; index < pNode->numParams; index++, pParam++)
        {
            if (id == (id & pParam->id))
                return pNode;
        }

        pParamList = pParamList->pNext;
    }
    return NULL;
}

/*-----------------------------------------------------------------------*/

/* Find required parameter associated with id */
extern paramDefn *
RCC_DB_PriorParam(cli_env *pCliEnv, paramID id)
{
    cmdNode     *pNode      = NULL;
    paramDefn   *pParam     = NULL;
    paramList   *pParamList = MMISC_GetRootParam(pCliEnv);

    while (NULL != pParamList)
    {
        pNode = DB_GetParamNode(pParamList);

        if (NULL != (pParam = DB_NodeParam(pNode, id)))
            break;

        pParamList = pParamList->pNext;
    }

    return pParam;
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS 
DB_ExecuteWrite(cli_env *pEnv, sbyte *pInput, sbyte4 bufSize)
{
    LineOut *pOutput = MMISC_OutputPtr(pEnv);
    sbyte   *pBuffer = NULL;

    if (NULL == pOutput)
        return ERROR_GENERAL_NULL_POINTER;

    if (NULL_STRING(pInput))
        return OK;

    if (-1 == bufSize)
        bufSize = STRLEN(pInput);

    pBuffer = pOutput->pBuffer + pOutput->length;
    bufSize = RC_MIN(bufSize, (pOutput->maxSize - pOutput->length));

    STRNCPY(pBuffer, pInput, bufSize);

    pOutput->length += bufSize;
    pOutput->offset  = pOutput->length;

    return OK;
}

/*-----------------------------------------------------------------------*/

/* 
 * For an external call to the system. Creates a working environment, sends any
 * resulting output in the output buffer (along with the size of output) and
 * closes the working environment.
 * 
 * Usage would be for an external application which would like to make a call
 * into the CLI.
 */

extern RLSTATUS
RCC_DB_Execute(sbyte *pCmd, sbyte *pOutBuff, sbyte4 outSize, sbyte4 *returnSize)
{
    RLSTATUS  status;
    cli_env  *pCliEnv    = NULL;
        
    *returnSize = 0;

    if ((NULL == pOutBuff) || (0 >= outSize))
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    if (OK != (status = RCC_DB_InitEnvironment(&pCliEnv, NULL, pOutBuff, outSize)))
        return status;

    if (OK != (status = RCC_UTIL_Init(pCliEnv)))
    {
        RCC_TASK_Cleanup(pCliEnv);
        return status;
    }

    MCONN_SetConnType(pCliEnv,    kRCC_CONN_EXTERNAL);
    MCONN_SetWriteHandle(pCliEnv, DB_ExecuteWrite);
    MCONN_SetReadHandle(pCliEnv,  NULL);

    RCC_EnableFeature(pCliEnv, kRCC_FLAG_RAW);
    RCC_TELNETD_AddSession(pCliEnv);

    RCC_DB_SetCommand(pCliEnv, pCmd, STRLEN(pCmd));

    status = RCC_DB_Process_CLI(pCliEnv);

    /* null end of output */
    *returnSize = MPRIN_GetOutputBufferSize(pCliEnv);
    pOutBuff[*returnSize] = 0;

    RCC_TASK_Cleanup(pCliEnv);

    return RCC_ERROR_THROW(status);
}

/*-----------------------------------------------------------------------*/

/* 
 * Executes a command while inside the system. The current use is for
 * scripting to excute commands as if coming from the user.
 */

extern RLSTATUS 
RCC_DB_ExecuteCommand(cli_env *pCliEnv, sbyte *pCmd)
{
    RCC_DB_SetCommand(pCliEnv, pCmd, STRLEN(pCmd));

    return RCC_DB_Process_CLI(pCliEnv);
}

/*-----------------------------------------------------------------------*/

extern sbyte4
RCC_DB_ParamName(cli_env *pCliEnv, paramDefn *pParam, Boolean optional)
{
    sbyte4  length = 0;
    sbyte * pBuf   = MMISC_GetErrorText(pCliEnv);
                    
    if (optional)
    {
        RCC_EXT_WriteStr(pCliEnv, "[");
        length++;
    }

    if (MPARM_HasKeyword(pParam))
    {
        RCC_EXT_WriteStr(pCliEnv, pParam->pKeyword);
        length += STRLEN(pParam->pKeyword);

        if (kDTabsolute != pParam->type)
        {
            RCC_EXT_WriteStr(pCliEnv, " ");
            length++;
        }
    }
    else
    {
        RCC_EXT_WriteStr(pCliEnv, "<");
        length++;

        *pBuf = 0;
        switch (pParam->type)
        {
            case kDTenum:
            case kDTlist:
                CONVERT_EnumStr(pParam->pParamInfo, pBuf, kRCC_ERROR_TEXT_SIZE, 
                                (kDTenum == pParam->type ? "|" : ","));
                break;
            case kDTuinteger:
            case kDTinteger:
                DB_RangeText(pParam, pBuf, kRCC_ERROR_TEXT_SIZE);
                break;
            default:
                break;
        }

        if (0 != *pBuf)
        {
            RCC_EXT_WriteStr(pCliEnv, pBuf);
            length += STRLEN(pBuf);
        }

/*
#ifndef __RCC_HIDE_UNAMED_KEYWORD__
        if (HAS_STRING(pParam->pKeyword) && ! MPARM_HasKeyword(pParam) && ! ENUMLIST(pParam->type))
            RCC_EXT_WriteStr(pCliEnv, pParam->pKeyword);
        else
#endif
            RCC_DB_PrintType(pCliEnv, pParam);
*/

        RCC_EXT_WriteStr(pCliEnv, ">");
        length++;
    }

    if (optional)
    {
        RCC_EXT_WriteStr(pCliEnv, "]");
        length++;
    }

    return length;
}

/*-----------------------------------------------------------------------*/

extern void 
RCC_DB_PrintParam(cli_env *pCliEnv, paramDefn *pParam, Boolean optional)
{                
    if (optional)
        RCC_EXT_WriteStr(pCliEnv, "[");

    if (MPARM_HasKeyword(pParam))
    {
        RCC_EXT_WriteStr(pCliEnv, pParam->pKeyword);

        if (kDTabsolute != pParam->type)
            RCC_EXT_WriteStr(pCliEnv, " ");
    }

    if (kDTabsolute != pParam->type)
    {
#ifdef kRCC_PARAM_LEFT_BRACKET
        RCC_EXT_WriteStr(pCliEnv, kRCC_PARAM_LEFT_BRACKET);
#endif

#ifndef __RCC_HIDE_UNAMED_KEYWORD__
        if (HAS_STRING(pParam->pKeyword) && ! MPARM_HasKeyword(pParam) && ! ENUMLIST(pParam->type))
            RCC_EXT_WriteStr(pCliEnv, pParam->pKeyword);
        else
#endif
            RCC_DB_PrintType(pCliEnv, pParam);

#ifdef kRCC_PARAM_RIGHT_BRACKET
        RCC_EXT_WriteStr(pCliEnv, kRCC_PARAM_RIGHT_BRACKET);
#endif
    }

    if (optional)
        RCC_EXT_WriteStr(pCliEnv, "]");

    RCC_EXT_WriteStr(pCliEnv, " ");
}

/*-----------------------------------------------------------------------*/

extern Boolean
RCC_DB_ParamRequired(cli_env *pCliEnv, paramDefn *pParam)
{
    Boolean      required;
    paramList   *pParams  = DB_GetCurrentParamList(pCliEnv);
    handlerDefn *pHandler = DB_GetParamHandler(pParams);

    if (NULL == pHandler)
        return TRUE;

    DB_REQUIRED_FN(pHandler, pParam, &required);
    return required;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
DB_ShowParams(cli_env *pCliEnv, cmdNode *pNode, 
              handlerDefn *pHandler, Boolean same,
              Boolean check, Boolean keyword)
{
    sbyte4       index;
    paramEntry  *pEntry      = NULL;
    paramList   *pParamList  = DB_GetCurrentParamList(pCliEnv);
    paramDefn   *pParam;
    Boolean      required;
    Boolean      sameNode;

    if (NULL == (pEntry = HANDLER_PARAMS(pHandler)))
        return;
    
    for (index = 0; index < HANDLER_PARAM_COUNT(pHandler); index++, pEntry++)
    {
        sameNode = DB_ParamById(pParamList, pNode, pEntry->id, &pParam);

        if (NULL == pParam)
            continue;

        if (same != sameNode)
            continue;

        if (check & (keyword != MPARM_HasKeyword(pParam)))
            continue;

        required =  FLAG_SET(pEntry, kRCC_PARAMETER_REQUIRED);

        if (!sameNode)
            RCC_EXT_Write(pCliEnv, "{", 1);

        RCC_DB_PrintParam(pCliEnv, pParam, !required);

        if (!sameNode)
            RCC_EXT_Write(pCliEnv, "} ", 2);
     }
}

/*-----------------------------------------------------------------------*/


RL_STATIC Boolean 
doppelganger(handlerDefn *pHandler)
{
    handlerDefn *pPrior = pHandler - 1;

#ifdef  __RCC_PARAMETER_ORDER_NONE__
    return ((pPrior->reqdParamMask == pHandler->reqdParamMask)   &&
            (pPrior->optlParamMask == pHandler->optlParamMask));
#else
    sbyte4           index;
    paramEntry      *pParam1 = pPrior->pParams;
    paramEntry      *pParam2 = pHandler->pParams;

    if (pPrior->paramCount != pHandler->paramCount)
        return FALSE;

    for (index = 0; index < pHandler->paramCount; index++)
    {
        if ( pParam1->id!= pParam2->id)
            return FALSE;

        pParam1++;
        pParam2++;
    }
    return TRUE;
#endif
    
}

/*-----------------------------------------------------------------------*/

extern void 
RCC_DB_ShowCommand(cli_env *pCliEnv, cmdNode *pNode)
{
    sbyte4       index;
    sbyte4       loop;
    sbyte4       indent;
    handlerDefn *pHandler = pNode->pHandlers;
    paramDefn   *pParam;
    Boolean      required;

    if (NULL == pNode)
        return;

    if (0 == pNode->numHandlers)
    {
        RCC_EXT_WriteStr(pCliEnv, pNode->pKeyword);
        return;
    }

    indent = MEDIT_GetIndent(pCliEnv);
    for (index = 0; index < pNode->numHandlers; index++, pHandler++)
    {
        if (0 < index) 
        {
            if (doppelganger(pHandler))
                continue;

            RCC_EXT_WriteStrLine(pCliEnv, "");
        }

        if (HELP_SET(pCliEnv, HELP_FLAG_SHOW_NO) && IS_NO_HANDLER(pHandler))
        {
            RCC_EXT_WriteStr(pCliEnv, kRCC_CMD_NO);
            RCC_EXT_WriteStr(pCliEnv, " ");
        }

        RCC_EXT_WriteStr(pCliEnv, pNode->pKeyword);
        RCC_EXT_WriteStr(pCliEnv, " ");
        MEDIT_SetIndent(pCliEnv, MEDIT_GetXPos(pCliEnv));

        /* show parent params */
        DB_ShowParams(pCliEnv, pNode, pHandler, FALSE, FALSE, FALSE);

        if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_NONE))
        {
            pParam = pNode->pParams;
            for (loop = 0; loop < pNode->numParams; loop++, pParam++)
            {
                if (OK == DB_REQUIRED_FN(pHandler, pParam, &required))
                    RCC_DB_PrintParam(pCliEnv, pParam, !required);
            }
        }

        if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_SOME))
        {
            /* named params */
            DB_ShowParams(pCliEnv, pNode, pHandler, TRUE, TRUE, TRUE);

            /* unnamed params */
            DB_ShowParams(pCliEnv, pNode, pHandler, TRUE, TRUE, FALSE);
        }

        if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ORDER_FULL))
        {
            DB_ShowParams(pCliEnv, pNode, pHandler, TRUE, FALSE, TRUE);
        }

        MEDIT_SetIndent(pCliEnv, indent); /* reset indent */
    }
}

/*-----------------------------------------------------------------------*/

extern void 
RCC_DB_ShowNode(cli_env *pCliEnv)
{
    paramList *pParams  = DB_GetCurrentParamList(pCliEnv);
    cmdNode   *execNode = MMISC_GetExecNode(pCliEnv);

    RCC_EXT_WriteStr(pCliEnv, "Executing node:");
    RCC_EXT_WriteStr(pCliEnv, execNode->pKeyword);
    if (RCC_DB_IsNoCommand(pParams))
        RCC_EXT_WriteStrLine(pCliEnv, " (no form)");
    else
        RCC_EXT_WriteStrLine(pCliEnv, "");
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS  
DB_ExecuteHandlers(cli_env *pCliEnv)
{
    paramList   *pParamList  = DB_GetCurrentParamList(pCliEnv);
    handlerDefn *pHandler    = NULL;
    cmdNode     *pNode       = NULL;
    RLSTATUS     status      = OK;

    RCC_ASSERT(NULL != pParamList);

    /* back up to first applicable param */
    while (NULL != pParamList)
    {
        if (FLAG_CLEAR(pParamList, kRCC_COMMAND_EXTEND))
            break;

        if (NULL == pParamList->pPrev)
            break;
                        
        pParamList = pParamList->pPrev;
    }

    for ( ; NULL != pParamList ; pParamList = pParamList->pNext)
    {
        if (FLAG_SET(pParamList, kRCC_COMMAND_NO_EXEC))
            continue;

        if (NULL == (pNode = DB_GetParamNode(pParamList)))
            continue;

        if (NULL == (pHandler = DB_GetParamHandler(pParamList)))
        {
            debugExecShow(pNode);
            continue;
        }

        /* Node being executed */
        if (FLAG_CLEAR(pNode, kRCC_COMMAND_META))
            MMISC_SetCmdNode(pCliEnv, pNode);

        /* reset for "more" pagination */
        RCC_EXT_OutputReset(pCliEnv);

        debugExecShow(pNode);

        status = RCC_DB_QueueHandler(pCliEnv, pHandler);

        if (OK != status)
            break;

#ifdef __SNMP_API_ENABLED__
        if ( (0 == MMISC_GetModeDepth(pCliEnv)) || 
            RCC_IsEnabled(pCliEnv, kRCC_FLAG_SNMPAUTO))
        {
            RCC_CMD_Snmp_Commit(pCliEnv);
        }
#endif

    }

    return status;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
DB_UpdateCliEnv(cli_env *pCliEnv)
{
    cmdNode   *pOldRoot    = MMISC_GetCurrRoot(pCliEnv);
    cmdNode   *pNewRoot    = RCC_DB_GetCommandNode(pCliEnv);
    paramList *pParamList  = DB_GetCurrentParamList(pCliEnv);

    if (NULL == pNewRoot)
    {
#ifdef __RCC_DEBUG__
        RCC_EXT_WriteStrLine(pCliEnv, "ERROR: can't get new root!");
#endif
        return;
    }

    /* enter intermediate mode? */
    if (   FLAG_CLEAR(pParamList, kRCC_COMMAND_GLOBAL)  &&  /* not a global command         */
           FLAG_SET(pNewRoot,     kRCC_COMMAND_MODE)    &&  /* has intermediate mode set    */
           FLAG_CLEAR(pParamList, kRCC_COMMAND_NO_EXEC) &&  /* are we exec'ing or eval'ing  */
           FLAG_CLEAR(pParamList, kRCC_COMMAND_NO)      &&  /* 'no' commands are exempt     */
          (pOldRoot != pNewRoot)                        &&  /* not the same node            */
           RCC_IsEnabled(pCliEnv, kRCC_FLAG_MODE) )         /* intermediate modes allowed   */
    {
        MMISC_SetCurrRoot(pCliEnv, pNewRoot);
        DB_NewParamList(pCliEnv, FALSE);
        RCC_EnableFeature(pCliEnv, kRCC_FLAG_NEWMODE);
    }
}

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS
DB_ParseEngine(cli_env *pCliEnv)
{
    RLSTATUS  status       = OK;
    Boolean   exit         = FALSE;

#ifdef kRCC_CMD_BANG
    if (OK != (status = RCC_HIST_ExecHistory(pCliEnv)))
        return status;
#endif /* kRCC_CMD_BANG */

    /* tokenize command input */
    if (OK != (status = RCC_DB_ParseCmdString(pCliEnv)))
        return status;
    
    /* match each token to a node or parameter */
    if (OK != (status = RCC_DB_Parameterize(pCliEnv)))
        return status;

    /* all incoming data tagged -- build list for execution */
    if (OK != (status = RCC_DB_BuildParamList(pCliEnv, TRUE)))
        return status;

    status = DB_ExecuteHandlers(pCliEnv);
    switch (status)
    {
    case STATUS_RCC_EXIT:
        RCC_DB_Exit(pCliEnv, FALSE);
        exit = TRUE;
        break;
    case STATUS_RCC_EXIT_ALL:
        RCC_DB_Exit(pCliEnv, TRUE);
        exit = TRUE;
        break;
    default:
        if (OK != status)
            return status;
        break;
    }

    if (! exit)
        DB_UpdateCliEnv(pCliEnv);

    DEBUG_PARAM_LIST(pCliEnv);

    return OK;
}

/*-----------------------------------------------------------------------*/


extern RLSTATUS RCC_DB_Process_CLI(cli_env *pCliEnv)
{
    RLSTATUS  status;
    
    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_EXEC))
    {
        if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_HISTORY))
            RCC_HIST_AddHistLine (pCliEnv);
    }
    else
        RCC_HIST_AddHistLine (pCliEnv);

    status = DB_ParseEngine(pCliEnv);

    /* aliasing requires 2 tries */
    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_ALIASED))
    {
        status = DB_ParseEngine(pCliEnv);
        RCC_DisableFeature(pCliEnv, kRCC_FLAG_ALIASED);
    }

    /* make sure printing is enabled */

    RCC_DisableFeature(pCliEnv, kRCC_FLAG_NOPRINT);

    /* allow new errors */

    RCC_DisableFeature(pCliEnv, kRCC_FLAG_ERROR_SET);

    RCC_UTIL_UpdatePrompt(pCliEnv); 

    return status;
}


#endif /*  __RCC_ENABLED__ */
