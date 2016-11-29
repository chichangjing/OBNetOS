/*  
 *  rcc_hist.c
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

#ifndef RL_STATIC
#define RL_STATIC static
#endif

/*
    The history buffer uses an array as a circular buffer.
    It would be nice to abstract out all array references
    and circular "pointer" management....

    Also, since we're dealing with an array, it would probably
    be cleaner to keep counter refs zero-based and translate
    to and from one-based notation at the UI.
*/


/*-----------------------------------------------------------------------*/


/* translate history index (e.g., !23) to buffer index */

RL_STATIC sbyte4 
HIST_Hist2Buff(HistInfo *pHistory, sbyte4 index)
{
    sbyte4 offset;
    sbyte4 least;

    /* empty buffer? */
    if (0 >= pHistory->iNumCmds)
        return -1;

    least = LEAST_RECENT_HIST(pHistory);

    /* out of range? */
    if ((least > index) || (index > pHistory->iNumCmds))
        return -1;

    /* buffer not wrapped yet? */
#if 0
    if (pHistory->iNumCmds < pHistory->iMaxHistCmds)
        return index - 1;
#endif

    offset = pHistory->bufferIndex - (pHistory->iNumCmds - index);
    if (0 > offset)
        offset += pHistory->iMaxHistCmds;

    return offset;
}

/*-----------------------------------------------------------------------*/

/* Text ptr for current history */
RL_STATIC sbyte * HIST_GetHistoryCmd(HistInfo *pHistory)
{
    sbyte       *pBuf;
    sbyte4       offset;
    CmdHistBuff *histCmdBuff;
    sbyte       *pTemp = MHIST_TempBuf(pHistory);
    
    if (pHistory->iCurHistCmd > pHistory->iNumCmds)
        return pTemp;

    offset = HIST_Hist2Buff(pHistory, pHistory->iCurHistCmd);
    if (0 > offset)
        return NULL;

    histCmdBuff = &(pHistory->pHistBuff[offset]);
    pBuf = histCmdBuff->histCmd;
    return pBuf;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_HIST_InitHistInfo(cli_env *pCliEnv)
{
    HistInfo *history;
    Length    histSize = kRCC_HISTORY_BUFFER_SIZE * sizeof(CmdHistBuff);

	if (NULL == pCliEnv)
		return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    history = MHIST_History(pCliEnv);

	history->iMaxHistCmds = kRCC_HISTORY_BUFFER_SIZE;
	history->iCurHistCmd  = 0;
    history->iNumCmds     = 0;

    if (NULL == (history->pHistBuff = RC_MALLOC(histSize)))
        return RCC_ERROR_THROW(ERROR_MEMMGR_NO_MEMORY);

	memset( (char*)(history->pHistBuff), '\0', histSize);

    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_HIST_ResizeHistBuff(cli_env *pCliEnv, sbyte4 NumCmds)
{
    sbyte4       newBuffIndex;
    sbyte4       start;
    sbyte4       count;
    sbyte4       dest;
    Length       segmentSize;
    HistInfo    *pHistory;
    CmdHistBuff *pOldBuff;
    CmdHistBuff *pNewBuff;

	if (NULL == pCliEnv)
		return RCC_ERROR_THROW(ERROR_RCC_FAILURE);

    pHistory = MHIST_History(pCliEnv);

    if (pHistory->iMaxHistCmds == NumCmds)
        return OK;

    if (NULL == (pNewBuff = RC_MALLOC(NumCmds * sizeof(CmdHistBuff))))
        return RCC_ERROR_THROW(ERROR_MEMMGR_NO_MEMORY);

    pOldBuff = pHistory->pHistBuff;

    if (pHistory->iMaxHistCmds < NumCmds)
    {
        /* shrink history buffer - retain only most recent commands */

        /* grab most recent half of circular buffer */
        start = pHistory->bufferIndex - NumCmds;
        if (0 > start)
            start = 0;

        count        = pHistory->bufferIndex - start + 1;
        dest         = NumCmds - count;
        pOldBuff    += start;
        pNewBuff    += dest;
        segmentSize  = count * sizeof(CmdHistBuff);
        MEMCPY(pNewBuff, pOldBuff, segmentSize);

        /* grab whatever is left of the second half of the buffer */

        if (count < NumCmds)
        {
            count        = NumCmds - count;
            start        = pHistory->iMaxHistCmds - count;
            dest        -= count;
            pOldBuff    += start;
            pNewBuff    += dest;
            segmentSize  = count * sizeof(CmdHistBuff);
            MEMCPY(pNewBuff, pOldBuff, segmentSize);
        }
        newBuffIndex = NumCmds;
    }
    else
    {
        /* expand history buffer */        
        
        /* special case: buffer ready to wrap */
        if (HIST_BUFFER_FULL(pHistory))
        {
            segmentSize = pHistory->iMaxHistCmds * sizeof(CmdHistBuff);
            MEMCPY(pNewBuff, pOldBuff, segmentSize);
        }
        else
        {
            /* grab least recent half of circular buffer */
            start        = pHistory->bufferIndex + 1;
            count        = pHistory->iMaxHistCmds - start + 1;
            pOldBuff    +=  start;
            segmentSize  = count * sizeof(CmdHistBuff);
            MEMCPY(pNewBuff, pOldBuff, segmentSize);

            /* grab most recent half of the buffer */
            dest         = count;
            count        = pHistory->bufferIndex +1;
            pNewBuff    += dest;
            segmentSize = count * sizeof(CmdHistBuff);
            MEMCPY(pNewBuff, pOldBuff, segmentSize);
        }
        newBuffIndex = pHistory->iMaxHistCmds - 1;
    }

    FREEMEM(pOldBuff);
    pHistory->pHistBuff    = pNewBuff;
    pHistory->bufferIndex  = newBuffIndex;
    pHistory->iMaxHistCmds = NumCmds;

    return OK;
}

/*-----------------------------------------------------------------------*/

extern void RCC_HIST_AddHistLine (cli_env *pCliEnv)
{
    sbyte       *pSrc  = NULL;
    sbyte       *pDest = NULL;
    HistInfo    *pHistory;

    if (NULL == pCliEnv)
        return;

    if (1 > MEDIT_GetLength(pCliEnv))
        return;

#ifdef kRCC_CMD_BANG 
    if (kRCC_CMD_BANG == *MEDIT_BufPtr(pCliEnv))
        return;
#endif

    pHistory = MHIST_History(pCliEnv);
    pSrc     = MEDIT_BufPtr(pCliEnv);
    if (NULL_STRING(pSrc))
        return;

    if (0 < pHistory->iNumCmds) {
        if (HIST_BUFFER_FULL(pHistory))
            pHistory->bufferIndex = 0;
        else
            pHistory->bufferIndex++;
    }

    pDest = HistBuffPtr(pHistory, pHistory->bufferIndex);
    //if (NULL_STRING(pSrc) || NULL_STRING(pDest))
  if (NULL == pDest)  
        return;

    STRCPY(pDest, pSrc);

    pHistory->iCurHistCmd = ++(pHistory->iNumCmds) + 1;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_HIST_DispHistBuff (cli_env *pCliEnv)
{
    sbyte4    count = 0;
    sbyte4    index;
    sbyte4    number;
    sbyte    *pBuf;
    HistInfo *pHistory;
    sbyte     buffer[16];

    if (NULL == pCliEnv)
    	return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    pHistory = MHIST_History(pCliEnv);

    if (0 >= pHistory->iNumCmds)
        return OK;

    /* buffer has not even filled/wrapped */
    if (pHistory->iNumCmds < pHistory->iMaxHistCmds)
    {
        for (index = 0; index <= pHistory->bufferIndex; index++)
		{
            pBuf   = HistBuffPtr(pHistory, index);
            number = index + 1;
            sprintf(buffer, "%4d ", number);
            RCC_EXT_WriteStr(pCliEnv, buffer);
            RCC_EXT_WriteStrLine(pCliEnv, pBuf);
		}
        return OK;
    }

    /* first half of buffer */
    count = LEAST_RECENT_HIST(pHistory);
    for (index = pHistory->bufferIndex + 1; 
         index < pHistory->iMaxHistCmds;
         index++, count++)
    {
        pBuf = HistBuffPtr(pHistory, index);
        sprintf(buffer, "%4d ", count);
        RCC_EXT_WriteStr(pCliEnv, buffer);
        RCC_EXT_WriteStrLine(pCliEnv, pBuf);
    }

    /* second half of buffer */
    for (index = 0; index <= pHistory->bufferIndex; index++, count++)
    {
        pBuf = HistBuffPtr(pHistory, index);
        sprintf(buffer, "%4d ", count);
        RCC_EXT_WriteStr(pCliEnv, buffer);
        RCC_EXT_WriteStrLine(pCliEnv, pBuf);
    }
    return OK;
}

/*-----------------------------------------------------------------------*/

/* for scrolling through history list with up/down arrows */
RL_STATIC void 
HIST_ChangeHistory(HistInfo *pHistory, sbyte4 offset)
{
    sbyte4 newHistory;
    
    newHistory = pHistory->iCurHistCmd + offset;

    if ((1 > newHistory) || (newHistory > (pHistory->iNumCmds + 1)))
        return;
    
    if (newHistory < LEAST_RECENT_HIST(pHistory))
        return;

    pHistory->iCurHistCmd = newHistory;
}

/*-----------------------------------------------------------------------*/

extern void RCC_HIST_Scroll(cli_env *pCliEnv, sbyte4 offset)
{
    HistInfo  *pHistory = MHIST_History(pCliEnv);
    sbyte     *pBuf     = MEDIT_BufPtr(pCliEnv);
    sbyte     *pTemp    = MHIST_TempBuf(pHistory);
    sbyte     *pHistoryCmd;
    
    /* otherwise, constantly repeating "fills" screen */
    RCC_EXT_OutputReset(pCliEnv);

    if (pHistory->iCurHistCmd > pHistory->iNumCmds) 
        STRCPY(pTemp, pBuf);

    HIST_ChangeHistory(pHistory, offset);
    pHistoryCmd = HIST_GetHistoryCmd(pHistory);
    if (NULL == pHistoryCmd)
        return;

    RCC_EXT_EraseLine(pCliEnv);
    RCC_DB_SetCommand(pCliEnv, pHistoryCmd, STRLEN(pHistoryCmd));
    RCC_EXT_WriteStr(pCliEnv, pHistoryCmd);
    RCC_EXT_EnablePrint(pCliEnv, TRUE);
}

/*-----------------------------------------------------------------------*/

RL_STATIC sbyte4
HIST_MatchHistory(HistInfo *pHistory, sbyte *pCommand)
{
    sbyte4     index;
    sbyte4     histSize;
    sbyte4     max;
    sbyte     *pHistoryCmd;
            
    histSize = STRLEN(pCommand);
    
    for (index = pHistory->bufferIndex; index >= 0; index--)
    {
        pHistoryCmd = HistBuffPtr(pHistory, index);
        if (0 == STRNCMP(pHistoryCmd, pCommand, histSize))
            return index;
    }

    max = RC_MIN(pHistory->iMaxHistCmds, pHistory->iNumCmds);
    for (index = max; index > pHistory->bufferIndex; index--)
    {
        pHistoryCmd = HistBuffPtr(pHistory, index);
        if (0 == STRNCMP(pHistoryCmd, pCommand, histSize))
            return index;
    }
    
    return -1;
}

/*-----------------------------------------------------------------------*/

#ifdef kRCC_CMD_BANG
extern RLSTATUS RCC_HIST_ExecHistory(cli_env *pCliEnv)
{
    HistInfo  *pHistory     = MHIST_History(pCliEnv);
    sbyte4     historyIndex;
    sbyte4     realIndex    = -1;
    sbyte     *pHistoryCmd  = NULL;
    sbyte     *pInputCmd    = MEDIT_BufPtr(pCliEnv);

    if (kRCC_CMD_BANG != *pInputCmd)
        return OK;

    pInputCmd++;   /* skip the '!' */

    /* "!!" - execute previous command */
    if (pInputCmd[0] == kRCC_CMD_BANG)
    {
        historyIndex = pHistory->iNumCmds;
    }
    else
    {
        if (OK != CONVERT_StrTo(pInputCmd, &historyIndex, kDTinteger))
        {
            /* if not a number than must be beginning of command string */
            realIndex = HIST_MatchHistory(pHistory, pInputCmd);
            if (0 > realIndex)
                return RCC_ERROR_THROW(ERROR_RCC_INVALID_HISTORY); 
        }
    }

    if (0 > realIndex)
        realIndex = HIST_Hist2Buff(pHistory, historyIndex);

    if (0 > realIndex)
        return RCC_ERROR_THROW(ERROR_RCC_INVALID_HISTORY);

    pHistoryCmd = HistBuffPtr(pHistory, realIndex);
    RCC_DB_SetCommand(pCliEnv, pHistoryCmd, STRLEN(pHistoryCmd));
    RCC_TASK_PrintPrompt(pCliEnv);
    RCC_EXT_WriteStrLine(pCliEnv, pHistoryCmd);
    RCC_HIST_AddHistLine(pCliEnv);
    return OK;
}
#endif /* kRCC_CMD_BANG */

#endif /* __RCC_ENABLED__ */

