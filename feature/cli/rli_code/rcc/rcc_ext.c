/*  
 *  rcc_ext.c
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

#ifdef __POSIX_OS__
# include <termios.h>
# ifdef __LINUX__
#  include <unistd.h>
# endif 
#endif /* __POSIX_OS__ */

#ifdef __VXWORKS_OS__
# include <ioLib.h>
#endif /* __VXWORKS_OS__ */

#ifdef RCC_LOG_HEADER
#include RCC_LOG_HEADER
#endif

#ifdef __WIN32_OS__
# include <Windows.h>
#endif /* __WIN32_OS__ */

#ifdef RCC_EDIT_STATUS_FN
void RCC_EDIT_STATUS_FN(cli_env *pCliEnv);
#else
#define RCC_EDIT_STATUS_FN(pCliEnv)
#endif

#define WHITE_SPACE(text)    (    (' '        == text) \
                               || (kCR        == text) \
                               || (kLF        == text) \
                               || (kTAB       == text) \
                               || (kCHAR_NULL == text) )


#undef kEOL

#ifdef __EOL_USES_LF__
# define kEOL "\n"
#endif

#ifdef __EOL_USES_CR__
# define kEOL "\r"
#endif

#ifndef kEOL
# define __EOL_USES_CRLF__
# define kEOL "\r\n"
#endif

#define kEOL_SIZE (sizeof(kEOL) - 1)

#define kTAG_FLAG_DIGIT     0x0001
#define kTAG_FLAG_ALPHA     0x0002
/*-----------------------------------------------------------------------*/

extern void 
RCC_EXT_CommandLineInit(cli_env *pCliEnv)
{
    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_LAST_INPUT))
    {
        RCC_DisableFeature(pCliEnv, kRCC_FLAG_LAST_INPUT);
        return;
    }

    MEDIT_SetLength(pCliEnv, 0);
    MEDIT_SetCursor(pCliEnv, 0);
    MEMSET(MEDIT_GetBufPtr(pCliEnv), 0, MEDIT_GetBufSize(pCliEnv));
}

/*-----------------------------------------------------------------------*/

extern EditType
RCC_EXT_GetWidth(cli_env *pCliEnv)
{
    return MSCRN_GetWidth(pCliEnv);
}

/*-----------------------------------------------------------------------*/

extern EditType
RCC_EXT_GetHeight(cli_env *pCliEnv)
{
    return MSCRN_GetHeight(pCliEnv);
}

/*-----------------------------------------------------------------------*/

extern void
RCC_EXT_SetWidth(cli_env *pCliEnv, EditType width)
{
    LineOut  *pOutput = MMISC_OutputPtr(pCliEnv);

    MSCRN_SetWidth(pCliEnv, width);
    pOutput->width = width;
}

/*-----------------------------------------------------------------------*/

extern void
RCC_EXT_SetHeight(cli_env *pCliEnv, EditType height)
{
    LineOut  *pOutput = MMISC_OutputPtr(pCliEnv);

    MSCRN_SetHeight(pCliEnv, height);
    pOutput->height = height;
}

/*-----------------------------------------------------------------------*/

RL_STATIC EditType 
EXT_GetCursorX(CmdEditInfo *edit)
{
    return edit->termX;
}

/*-----------------------------------------------------------------------*/

RL_STATIC EditType 
EXT_GetCursorY(CmdEditInfo *edit)
{
    return edit->termY;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
EXT_SetCursorX(CmdEditInfo *edit, EditType pos)
{
    edit->termX = pos;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
EXT_SetCursorY(CmdEditInfo *edit, EditType pos)
{
    edit->termY = pos;
}

/*-----------------------------------------------------------------------*/

extern void
RCC_EXT_GetScreenCoord(cli_env *pCliEnv, sbyte4 *xPos, sbyte4 *yPos) 
{
    CmdEditInfo *pEdit   = MEDIT_EditInfoPtr(pCliEnv);

    *xPos = EXT_GetCursorX(pEdit);
    *yPos = EXT_GetCursorY(pEdit);
}

/*-----------------------------------------------------------------------*/

/*
 *  Get coordinates of current cursor position
 *  --  including prompt and within existing line (not screen!)
 */

RL_STATIC void
RCC_EXT_CurrCoord(cli_env *pCliEnv, EditType *xPos, EditType *yPos) 
{
    sbyte    *pBuf        = MEDIT_GetBufPtr(pCliEnv);
    EditType  cursorPos   = MEDIT_GetCursor(pCliEnv);
    EditType  width       = MSCRN_GetWidth(pCliEnv);
    EditType  X           = MEDIT_GetPromptLen(pCliEnv);
    EditType  Y           = 0;
    
    if (0 >= width)
        width = kRCC_DEFAULT_WIDTH;

    while (0 < cursorPos--)
    {
        switch (*(pBuf++))
        {
        case kCR:
            X = 0;
            break;
        case kLF:
            Y++;
            break;
        default:         
            if (++X >= width)
            {
                Y++;
                X = 0;
            }
            break;
        }
    }
    *xPos = X;
    *yPos = Y;
}

/*-----------------------------------------------------------------------*/

/*
 *  Movement functions
 */

/* move cursor to beginning of current line */
extern void RCC_EXT_LineStart(cli_env *pCliEnv)
{
    RCC_EXT_SetCursor(pCliEnv, 0);
}

/*-----------------------------------------------------------------------*/

/* move cursor to end of current line */
extern void RCC_EXT_LineEnd(cli_env *pCliEnv)
{
    RCC_EXT_SetCursor(pCliEnv, MEDIT_GetLength(pCliEnv));
}

/*-----------------------------------------------------------------------*/

extern void RCC_EXT_EraseLine(cli_env *pCliEnv)
{
    sbyte    *pBuf       = MEDIT_GetBufPtr(pCliEnv);
    EditType  lineLength = MEDIT_GetLength(pCliEnv);
    EditType  index;

#ifdef kKEY_CONTINUE
    for (index = 0; index < lineLength; index++)
    {
        if ((kCR == pBuf[index]) || (kLF == pBuf[index]))
            continue;

        pBuf[index] = ' ';
    }
#else
    for (index = 0; index < lineLength; index++)
        pBuf[index] = ' ';
#endif

    pBuf[index] = '\0';

    RCC_EXT_LineStart(pCliEnv);
    RCC_EXT_Write(pCliEnv, pBuf, index);

    index = -index;
    RCC_EXT_MoveCursor(pCliEnv, index);
    RCC_EXT_CommandLineInit(pCliEnv);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
RCC_EXT_MoveTTYCursor(cli_env *pCliEnv, EditType xPos, EditType yPos)
{
    sbyte   buffer[32];

    /* skip if vt escapes disabled */
#ifdef __DISABLE_VT_ESCAPES__
    return;
#endif

    if (0 != xPos)
    {
        if (xPos < 0)
            sprintf(buffer, kRCC_VTTERM_LT, -xPos);
        else if (xPos > 0)
            sprintf(buffer, kRCC_VTTERM_RT, xPos);

        RCC_EXT_PutStr(pCliEnv, buffer);
    }

    if (0 != yPos)
    {
        if (yPos < 0)
            sprintf(buffer, kRCC_VTTERM_UP, -yPos);
        else if (yPos > 0)
            sprintf(buffer, kRCC_VTTERM_DN, yPos);

        RCC_EXT_PutStr(pCliEnv, buffer);
    }
}

/*-----------------------------------------------------------------------*/

#ifdef __WIN32_OS__
/* move cursor to absolute position */
RL_STATIC void 
RCC_EXT_MoveDOSCursor(cli_env *pCliEnv, EditType xPos, EditType yPos)
{
    CONSOLE_SCREEN_BUFFER_INFO screenBuffer;
    HANDLE                     console;
    
    
    if (xPos >= MSCRN_GetWidth(pCliEnv))
        xPos--;

    console = GetStdHandle(STD_OUTPUT_HANDLE);
        
    if (INVALID_HANDLE_VALUE == console)
        return;

    if (0 == GetConsoleScreenBufferInfo(console, &screenBuffer))
            return;

    screenBuffer.dwCursorPosition.X += xPos;
    screenBuffer.dwCursorPosition.Y += yPos;

    SetConsoleCursorPosition(console, screenBuffer.dwCursorPosition);
}
#endif /* __WIN32_OS__ */

/*-----------------------------------------------------------------------*/

/* move cursor to absolute position within line */
extern void RCC_EXT_SetCursor(cli_env *pCliEnv, EditType position)
{
    EditType  lineLength = MEDIT_GetLength(pCliEnv);
    EditType  new_X      = 0;
    EditType  new_Y      = 0;
    EditType  old_X      = 0;
    EditType  old_Y      = 0;
    CmdEditInfo *pEdit   = MEDIT_EditInfoPtr(pCliEnv);

    /* no point moving around text not on screen */
    if (RCC_NotEnabled(pCliEnv, kRCC_FLAG_ECHO))
        return;

    RCC_EXT_CurrCoord(pCliEnv, &old_X, &old_Y); 

    if (position < 0)
        position = 0;

    if (position > lineLength)
        position = lineLength;

    MEDIT_SetCursor(pCliEnv, position);

    RCC_EXT_CurrCoord(pCliEnv, &new_X, &new_Y);

    /* save position */
    EXT_SetCursorX(pEdit, new_X);
    EXT_SetCursorY(pEdit, new_Y);
    
    new_X -= old_X;
    new_Y -= old_Y;

    /* don't bother moving if it's nowhere */
    if ((0 == new_X) && (0 == new_Y))
        return;

    if (kRCC_CONN_CONSOLE == MCONN_GetConnType(pCliEnv))
    {
#ifdef __WIN32_OS__
        RCC_EXT_MoveDOSCursor(pCliEnv, new_X, new_Y);
#else        
        RCC_EXT_MoveTTYCursor(pCliEnv, new_X, new_Y);
#endif
    }
    else
    {
        RCC_EXT_MoveTTYCursor(pCliEnv, new_X, new_Y);
    }
}

/*-----------------------------------------------------------------------*/

/* move cursor a relative amount within line */
extern void RCC_EXT_MoveCursor(cli_env *pCliEnv, EditType offset)
{
    EditType  cursorPos = MEDIT_GetCursor(pCliEnv);
    sbyte    *pBuf      = MEDIT_GetBufPtr(pCliEnv);

    if (0 == offset)
        return;

    /* no point moving around text not on screen */
    if (RCC_NotEnabled(pCliEnv, kRCC_FLAG_ECHO))
        return;

    cursorPos += offset;
    pBuf      += cursorPos;

    /* if moving to a EOL char we have to scoot
       past to the other side */
    while ((kCR == *pBuf) || (kLF == *pBuf)) 
    {
        if (0 > offset)
        {
            cursorPos--;
            pBuf--;
        }
        else
        {
            cursorPos++;
            pBuf++;
        }
    }

    RCC_EXT_SetCursor(pCliEnv, cursorPos);
}

/*-----------------------------------------------------------------------*/

extern void RCC_EXT_OutputReset(cli_env *pEnv)
{
    LineOut  *pOutput = MMISC_OutputPtr(pEnv);

    if (NULL == pOutput)
        return;

    pOutput->flags     &= kPRINT_NO_ALLOC;
    pOutput->height     = MSCRN_GetHeight(pEnv);
    pOutput->width      = MSCRN_GetWidth(pEnv);
    pOutput->lineCount  = 0;
    pOutput->length     = 0;
    pOutput->offset     = 0;
    pOutput->stop       = 0;

    MEMSET(pOutput->pBuffer, 0, pOutput->maxSize);
}

/*-----------------------------------------------------------------------*/

/* pass NULL for buffer to create a new one */

extern RLSTATUS RCC_EXT_OutputCreate(cli_env *pEnv, sbyte *pBuffer, sbyte4 size)
{
    LineOut  *pOutput = MMISC_OutputPtr(pEnv);
    BitMask   flags  = 0;

    if (NULL != pBuffer)
        flags = kPRINT_NO_ALLOC;
    else
    {
        if (NULL == (pBuffer = RC_MALLOC(size)))
            return RCC_ERROR_THROW(ERROR_MEMMGR_NO_MEMORY);
    }

    pOutput->flags      = flags;
    pOutput->maxSize    = size;
    pOutput->pBuffer    = pBuffer;

    return OK;
}

/*-----------------------------------------------------------------------*/

extern void RCC_EXT_FreeOutput(cli_env *pEnv)
{
    LineOut  *pOutput = MMISC_OutputPtr(pEnv);

    if (FLAG_SET(pOutput, kPRINT_NO_ALLOC))
        return;

    if (NULL == pOutput->pBuffer)
        return;

    RC_FREE(pOutput->pBuffer);
}

/*-----------------------------------------------------------------------*/

/* return TRUE if user canceled */

RL_STATIC Boolean
EXT_More(cli_env *pCliEnv)
{
    cliChar      charIn    = 0;
    sbyte        moreBuf[] = kRCC_MORE_TEXT;
    sbyte4       moreSize  = sizeof(moreBuf);
    WriteHandle *Writer    = MCONN_GetWriteHandle(pCliEnv);
    ReadHandle  *Reader    = MCONN_GetReadHandle(pCliEnv);
    LineOut     *pOutput   = MMISC_OutputPtr(pCliEnv);
    ubyte4       time      = OS_SPECIFIC_GET_SECS();

    if (RCC_NotEnabled(pCliEnv, kRCC_FLAG_MORE))
        return FALSE;

    Writer(pCliEnv, moreBuf, moreSize);

    /* the pause that refreshes... */
    while (0 == charIn)
    {
        if (OK != Reader(pCliEnv, &charIn))
        {
            RCC_EnableFeature(pCliEnv, kRCC_FLAG_NOPRINT);
            return FALSE;
        }

        /* toss leftover \n from prior command */
        if ((OS_SPECIFIC_GET_SECS() == time)
             && (kLF == charIn))
        {
            charIn = 0;
        }

    }

    /* clear more message */
    Writer(pCliEnv, "\r", 1);
    MEMSET(moreBuf, ' ', moreSize);
    Writer(pCliEnv, moreBuf, moreSize);
    Writer(pCliEnv, "\r", 1);

#ifdef kRCC_MORE_CANCEL
    if (kRCC_MORE_CANCEL == TOUPPER(charIn))
    {
        RCC_EXT_OutputReset(pCliEnv);
        RCC_EnableFeature(pCliEnv, kRCC_FLAG_NOPRINT);
        HELP_INDENT_RESET_M(pCliEnv);
        return TRUE;
    }
#endif

#ifdef kRCC_MORE_LINE
    if (kRCC_MORE_LINE != TOUPPER(charIn))
#endif
       /* a fresh page to play with */
       pOutput->lineCount = 0;

    return FALSE;
}

/*-----------------------------------------------------------------------*/

extern void
RCC_EXT_EnablePrint(cli_env *pCliEnv, Boolean enable)
{
    LineOut  *pOutput   = MMISC_OutputPtr(pCliEnv);

    if (enable)
    {
        RCC_DisableFeature(pCliEnv, kRCC_FLAG_NOPRINT);
        pOutput->lineCount  = 0;
    }
    else
        RCC_EnableFeature(pCliEnv, kRCC_FLAG_NOPRINT);
}

/*-----------------------------------------------------------------------*/

/* returns true if last line of screen */
RL_STATIC Boolean
EXT_NextLine(LineOut *pOutput)
{
    pOutput->lineCount++;
     
    return (pOutput->lineCount > (pOutput->height - 1));
}

/*-----------------------------------------------------------------------*/

/*
  when last line of screen is reached, suspend output until next line is reached.
  if next line is reached, display more prompt and wait for action
      if Q or cancel, kill buffer and exit
      if CR, display just next line
      if any other key, reset count and display rest until 
         another full page and repeat
  else
      dump remainder of output on exit
*/

/*-----------------------------------------------------------------------*/

RL_STATIC RLSTATUS
EXT_Output(cli_env *pCliEnv, sbyte *pInBuf, sbyte4 length)
{
    sbyte4        loop     = 0;
    sbyte4        outSize  = 0;
    sbyte4        escCount = 0;
    LineOut      *pOutput  = MMISC_OutputPtr(pCliEnv);
    WriteHandle  *Writer   = MCONN_GetWriteHandle(pCliEnv);
    Boolean       noNull   = (0 < length);
    Boolean       hardWrap = RCC_IsEnabled(pCliEnv, kRCC_FLAG_HARDWRAP);
    Boolean       lastLine = FALSE;
    Boolean       newLine  = FALSE;
    Boolean       escaped  = FALSE;
    CmdEditInfo  *pEdit    = MEDIT_EditInfoPtr(pCliEnv);
    sbyte        *pStart   = pInBuf;
    sbyte        *pBuf     = pInBuf;
    EditType      xPos     = EXT_GetCursorX(pEdit);
    EditType      yPos     = EXT_GetCursorY(pEdit);
    EditType      cursor   = MEDIT_GetCursor(pCliEnv);
    EditType      maxCol   = MSCRN_GetWidth(pCliEnv);

    /* raw output -- dump it directly */
    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_RAW))
    {
        if (0 >= length)
            outSize = STRLEN(pInBuf);
        else
            outSize = length;

        return Writer(pCliEnv, pInBuf, outSize);
    }

    /* indent text, if indented and starting text before margin */
    if (HELP_SET(pCliEnv, HELP_FLAG_INDENT) && 
         (pOutput->indent > xPos))
    {
        for (loop = xPos; loop < pOutput->indent ; loop++, xPos++)
            Writer(pCliEnv, " ", 1);
    }

    while (kCHAR_NULL != *pBuf)
    {
        outSize++;

        /* print is cancelled -- send back "blank" error */
        if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_NOPRINT))
            return ERROR_RCC_NO_ERROR_MSG;

        switch (*pBuf) 
        {
        case kCR:
            xPos = 0;
            break;
        case kLF:
            yPos++;
            lastLine |= EXT_NextLine(pOutput);
            newLine = TRUE;
            break;
        case kESC:
            if (1 < outSize)
            {
                /* print what we have so far */
                Writer(pCliEnv, pStart, outSize);
                pStart  += outSize;
                outSize  = 0;
            }
            escaped = TRUE;
            escCount = 1;
            break;
        default:
            if (! escaped)
                xPos++;

            break;
        }

        /* column wrap */
        if (maxCol == xPos)
        {
            if (hardWrap)
            {
                /* print what we have so far */
                Writer(pCliEnv, pStart, outSize);
                pStart  += outSize;
                outSize  = 0;

                Writer(pCliEnv, kEOL, kEOL_SIZE);
            }
            xPos = 0;
            yPos++;
            lastLine |= EXT_NextLine(pOutput);

            if (lastLine)
            {
                Writer(pCliEnv, pStart, outSize);
                pStart  += outSize;
                outSize  = 0;
                EXT_More(pCliEnv);
                lastLine = FALSE;
            }

            newLine = TRUE;
        }

        if (newLine)
        {
            Writer(pCliEnv, pStart, outSize);
            pStart  += outSize;
            outSize  = 0;

#ifdef __ENABLE_AUTO_CR__
            Writer(pCliEnv, "\r", 1);
            xPos = 0;
#endif            
            /* 
             * is there a left margin to indent?
             * 
             * it would be nice to break on space char, however,
             * the space to break on may be in a prior string printed
             * which makes this impossible w/o caching prints
             */
            if (HELP_SET(pCliEnv, HELP_FLAG_INDENT) && 
                !lastLine                           && 
                (0 < pOutput->indent))
            {
                for (loop = pOutput->indent ; loop > 0; loop--)
                     Writer(pCliEnv, " ", 1);

                xPos += pOutput->indent;
            }
            newLine = FALSE;
        }
        pBuf++;
        cursor++;

        if (lastLine)
        {
            Writer(pCliEnv, pStart, outSize);
            pStart  += outSize;
            outSize  = 0;
            EXT_More(pCliEnv);
            lastLine = FALSE;
        }

        if (noNull && (0 >= --length))
            break;
    }

    if (0 < outSize)
    {
        Writer(pCliEnv, pStart, outSize);
        outSize = 0;
    }

    MEDIT_SetCursor(pCliEnv, cursor);
    EXT_SetCursorX(pEdit, xPos);
    EXT_SetCursorY(pEdit, yPos);

    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_EXT_Write(cli_env *pCliEnv, sbyte *pInBuf, sbyte4 length)
{	
    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_NOPRINT))
        return ERROR_RCC_NO_ERROR_MSG;

    /* this should be an assert eventually */
    if (NULL == pInBuf)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    if (0 == length)
        return OK;

#ifdef RCC_LOG_OUTPUT_FN
    if ( RCC_NotEnabled(pCliEnv, kRCC_FLAG_INPUT) &&
         RCC_IsEnabled(pCliEnv, kRCC_FLAG_LOG_OUTPUT))
        RCC_LOG_OUTPUT_FN(pInBuf);
#endif

#ifdef RCC_LOG_IO_FN
    if ( RCC_IsEnabled(pCliEnv, kRCC_FLAG_INPUT) &&
         RCC_IsEnabled(pCliEnv, kRCC_FLAG_LOG_OUTPUT))
        RCC_LOG_IO_FN(NULL, pInBuf);
#endif

    /* skip it all if echo disabled */
    if ( RCC_IsEnabled(pCliEnv, kRCC_FLAG_INPUT) &&
         RCC_NotEnabled(pCliEnv, kRCC_FLAG_ECHO))
        return OK;
	
    return EXT_Output(pCliEnv, pInBuf, length);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_EXT_WriteStr(cli_env *pCliEnv, sbyte *pBuf)
{
    /* this should be an assert eventually */
    if (NULL == pBuf)
        return OK;

    /* if length is -1 then write will use NULL to determine length */
    return RCC_EXT_Write(pCliEnv, pBuf, -1);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_EXT_WriteStrLine(cli_env *pCliEnv, sbyte *pBuf)
{
    RLSTATUS status;

    if (OK == (status = RCC_EXT_WriteStr(pCliEnv, pBuf)))
          return RCC_EXT_Write(pCliEnv, kEOL, kEOL_SIZE);

    return status;
}

/*-----------------------------------------------------------------------*/

/* Similar to RCC_EXT_Write but doesn't update buffer  */
extern RLSTATUS RCC_EXT_Put(cli_env *pCliEnv, sbyte *pBuf, sbyte4 bufLen)
{
    WriteHandle  *Writer = MCONN_GetWriteHandle(pCliEnv);

    if (NULL == pBuf)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    return Writer(pCliEnv, pBuf, bufLen);
}

/*-----------------------------------------------------------------------*/

/* Similar to RCC_EXT_WriteStr but doesn't update buffer */
extern RLSTATUS RCC_EXT_PutStr(cli_env *pCliEnv, sbyte *pBuf)
{
    if (NULL == pBuf)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    return RCC_EXT_Put(pCliEnv, pBuf, STRLEN(pBuf));
}

/*-----------------------------------------------------------------------*/

/* Similar to RCC_EXT_WriteStrLine but doesn't update buffer */
extern RLSTATUS RCC_EXT_PutStrLine(cli_env *pCliEnv, sbyte *pBuf)
{
    RLSTATUS status;

    if (NULL == pBuf)
        return RCC_ERROR_THROW(ERROR_GENERAL_NULL_POINTER);

    if (OK == (status = RCC_EXT_PutStr(pCliEnv, pBuf)))
          return RCC_EXT_Put(pCliEnv, kEOL, kEOL_SIZE);

    return status;
}

/*-----------------------------------------------------------------------*/

extern void 
RCC_EXT_PrintString(cli_env *pCliEnv, sbyte *pString, sbyte4 length, sbyte fill)
{
    sbyte4  limit, size;

    if (0 == length)
    {
        RCC_EXT_WriteStr(pCliEnv, pString);
        return;
    }

    size  = (sbyte4) STRLEN(pString);
    limit = RC_MIN(length, size);
    RCC_EXT_Write(pCliEnv, pString, limit);

    limit = length - limit;
    while (0 < limit--)
        RCC_EXT_Write(pCliEnv, &fill, 1);

}

/*-----------------------------------------------------------------------*/

/*
 *  Line editing functions
 */

extern void
RCC_EXT_InsertText(cli_env *pCliEnv, sbyte *pText, sbyte4 length)
{
    EditType    offset;
    EditType    cursorPos   = MEDIT_GetCursor(pCliEnv);
    EditType    lineLength  = MEDIT_GetLength(pCliEnv);
    sbyte      *pBuf        = MEDIT_GetBufPtr(pCliEnv);
    sbyte4      index;

    if (MEDIT_GetBufSize(pCliEnv) <= (lineLength + length))
        return;

    /* are we just appending text? */
    if (cursorPos == lineLength)
    {
        pBuf       += lineLength;
        cursorPos  += length;

        STRNCPY(pBuf,          pText, length);
        RCC_EXT_Write(pCliEnv, pText, length);

        MEDIT_SetCursor(pCliEnv, cursorPos);
        MEDIT_SetLength(pCliEnv, cursorPos);

        return;
    }

    /* inserting in the middle... */
    for (index = lineLength; index >= cursorPos; index--)
    {
        pBuf[index + length] = pBuf[index];
    }

    pBuf += cursorPos;
    MEMCPY(pBuf, pText, length);
    RCC_EXT_WriteStr(pCliEnv, pBuf);

    lineLength += length;
    offset      = cursorPos - lineLength + length;

    MEDIT_SetCursor(pCliEnv, lineLength);
    MEDIT_SetLength(pCliEnv, lineLength);
    RCC_EXT_MoveCursor(pCliEnv, offset);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
EXT_Refresh(cli_env *pCliEnv, EditType offset, EditType length)
{
    sbyte     *pBuf       = MEDIT_GetBufPtr(pCliEnv);
    EditType   cursorPos;

    if (0 > length)
        return;

    RCC_EXT_MoveCursor(pCliEnv, offset);
    cursorPos = MEDIT_GetCursor(pCliEnv);
    pBuf += cursorPos;

    RCC_EXT_Write(pCliEnv, pBuf, length);
    length = -length;
    RCC_EXT_MoveCursor(pCliEnv, length);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
EXT_DeleteText(cli_env *pCliEnv, EditType start, EditType length)
{
    sbyte     *pBuf       = MEDIT_GetBufPtr(pCliEnv);
    EditType   lineLength = MEDIT_GetLength(pCliEnv);
    EditType   index;
    EditType   remainder;
    EditType   updated[kRCC_DEFAULT_HEIGHT];
    EditType   lineWidth    = 0;
    EditType   lineCount    = 0;
    Boolean    linesDeleted = FALSE;
    sbyte     *pTemp;

    if (0 >= length)
        return;

    /* 
     * Are we erasing a line end? If so, we've already skipped BEFORE 
     * the line break, due to cursor prepositioning
     */

    if (0 == STRNCMP(kEOL, &pBuf[start + 1], kEOL_SIZE))
    {
        if (1 < kEOL_SIZE)
            length++;

        start++;
    }

    /* delete mid string */
    if ( (start + length) < lineLength)
    {

#ifdef kKEY_CONTINUE
        /* erase pesky line continue character */
        for (index = start + length; index < lineLength; index++)
        {
            if (kKEY_CONTINUE == pBuf[index])
            {
                remainder = index - (start + length) + 1;
                RCC_EXT_MoveCursor(pCliEnv, remainder);
                RCC_EXT_Write(pCliEnv, " ", 1);
                RCC_EXT_SetCursor(pCliEnv, start);
            }
            if (kCR == pBuf[index])
                break;
        }
#endif

        /* if shifting lines up we need to know what to erase */
        for (index = start; index < (start + length); index++)
        {
            if (kCR == pBuf[index])
            {
                linesDeleted = TRUE;
                break;
            }
        }

        if (linesDeleted)
        {
            pTemp = &pBuf[start];
            for (index = start; index < lineLength; index++, pTemp++)
            {
                if (kCR == *pTemp)
                {
                    updated[lineCount] = lineWidth;
                    lineCount++;
                    lineWidth = 0;
                }
                if ((kCR != *pTemp) && (kLF != *pTemp))
                    lineWidth++;
            }
            if (0 < lineCount)
                updated[lineCount] = lineWidth;

            /* erase extra lines */
            for (index = 0; index <= lineCount; index++)
            {
                RCC_EXT_PrintString(pCliEnv, "", updated[index], ' ');
                if (index < lineCount)
                    RCC_EXT_WriteStrLine(pCliEnv, "");
            }
            RCC_EXT_SetCursor(pCliEnv, start);
        }

        /* shift remaining text left */
        for (index = start + length; index < lineLength; index++)
        {
            pBuf[index - length] = pBuf[index];
            pBuf[index] = ' ';
        }

        /* erase rest of line */
        for (index = lineLength - length; index < lineLength; index++)
            pBuf[index] = ' ';
    }
    else /* end of string */
    {
        for (index = start; index < lineLength; index++)
            pBuf[index] = ' ';
    }

    /* display revised text */
    remainder         = lineLength - start;
    start            -= MEDIT_GetCursor(pCliEnv);

    EXT_Refresh(pCliEnv, start, remainder);

    lineLength       -= length;
    pBuf[lineLength]  = kCHAR_NULL;

    MEDIT_SetLength(pCliEnv, lineLength);
}

/*-----------------------------------------------------------------------*/

/* delete character at current cursor position */
extern void RCC_EXT_DeleteChar(cli_env *pCliEnv)
{
    EditType lineLength = MEDIT_GetLength(pCliEnv);
    EditType cursorPos  = MEDIT_GetCursor(pCliEnv);

    if (lineLength <= 0)
        return;

    /* trying to delete past end of line? */
    if (cursorPos >= lineLength)
        return;

    EXT_DeleteText(pCliEnv, cursorPos, 1);
}

/*-----------------------------------------------------------------------*/

extern sbyte RCC_EXT_GetPriorChar(cli_env *pCliEnv)
{
    EditType  cursorPos  = MEDIT_GetCursor(pCliEnv);
    EditType  lineLength = MEDIT_GetLength(pCliEnv);
    sbyte    *pBuf       = MEDIT_GetBufPtr(pCliEnv);

    if (0 >= lineLength)
        return '\0';

    return pBuf[--cursorPos];
}

/*-----------------------------------------------------------------------*/

/* no screen updating - just for voiding last char entered */
void RCC_EXT_DiscardPrevChar(cli_env *pCliEnv)
{
    EditType  lineLength = MEDIT_GetLength(pCliEnv);
    sbyte    *pBuf       = MEDIT_GetBufPtr(pCliEnv);

    if (0 >=  lineLength)
        return;

    lineLength--;
    pBuf[lineLength] = '\0';
    MEDIT_SetLength(pCliEnv, lineLength);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
EXT_DeleteFromStart(cli_env *pCliEnv)
{
    EditType cursorPos  = MEDIT_GetCursor(pCliEnv);

    RCC_EXT_LineStart(pCliEnv);
    EXT_DeleteText(pCliEnv, 0, cursorPos);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
EXT_DeleteToEnd(cli_env *pCliEnv)
{
    EditType lineLength = MEDIT_GetLength(pCliEnv);
    EditType cursorPos  = MEDIT_GetCursor(pCliEnv);
    EditType length     = lineLength - cursorPos;

    EXT_DeleteText(pCliEnv, cursorPos, length);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void 
EXT_WordPrevious(cli_env *pCliEnv)
{
    Boolean   newWord    = FALSE;
    Boolean   blank      = FALSE;
    sbyte    *pBuf       = MEDIT_GetBufPtr(pCliEnv);
    EditType  cursorPos  = MEDIT_GetCursor(pCliEnv);
    EditType  index;
    
    index  = cursorPos;
    pBuf  += cursorPos;

    while (0 < --index)
    {
        pBuf--;

        blank = blank || WHITE_SPACE(*pBuf);

        if (newWord && WHITE_SPACE(*pBuf))
            break;
        else
            newWord = (blank && (! WHITE_SPACE(*pBuf)));

    }

    if (0 < index)
        index++;

    RCC_EXT_SetCursor(pCliEnv, index);
}

/*-----------------------------------------------------------------------*/

void RCC_EXT_WordNext(cli_env *pCliEnv)
{
    sbyte    *pBuf       = MEDIT_GetBufPtr(pCliEnv);
    EditType  cursorPos  = MEDIT_GetCursor(pCliEnv);
    EditType  lineLength = MEDIT_GetLength(pCliEnv);
    EditType  index      = 0;
    Boolean   blank      = FALSE;
    
    index  = cursorPos;
    pBuf  += cursorPos;
    while (lineLength > index++)
    {
        if (blank && (!WHITE_SPACE(*pBuf)))
            break;
        else
            blank = WHITE_SPACE(*pBuf);

        pBuf++;
    }
    if (blank)
        RCC_EXT_SetCursor(pCliEnv, --index);
}

/*-----------------------------------------------------------------------*/

void RCC_EXT_WordChangeCase(cli_env *pCliEnv, Boolean upperCase)
{
    sbyte    *pBuf       = MEDIT_GetBufPtr(pCliEnv);
    EditType  cursorPos  = MEDIT_GetCursor(pCliEnv);
    EditType  lineLength = MEDIT_GetLength(pCliEnv);
    EditType  count      = 0;

    pBuf += cursorPos;

    if (WHITE_SPACE(*pBuf))
        return;

    while ((cursorPos < lineLength) && !(WHITE_SPACE(*pBuf)))
    {
        if (upperCase)
            *pBuf = TOUPPER(*pBuf);
        else
            *pBuf = TOLOWER(*pBuf);

        count++;
        cursorPos++;
        pBuf++;
    }

    EXT_Refresh(pCliEnv, 0, count);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void EXT_WordDeleteStart(cli_env *pCliEnv)
{
    sbyte     *pBuf       = MEDIT_GetBufPtr(pCliEnv);
    EditType   cursorPos  = MEDIT_GetCursor(pCliEnv);
    EditType   length     = 0;
    
    if (0 >= cursorPos)
        return;

    pBuf += cursorPos - 1;

    while (0 < cursorPos)
    {
        cursorPos--;
        pBuf--;
        length++;

        if (WHITE_SPACE(*pBuf))
            break;
    }

    EXT_DeleteText(pCliEnv, cursorPos, length);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void EXT_WordDeleteEnd(cli_env *pCliEnv)
{
    sbyte     *pBuf       = MEDIT_GetBufPtr(pCliEnv);
    EditType   cursorPos  = MEDIT_GetCursor(pCliEnv);
    EditType   lineLength = MEDIT_GetLength(pCliEnv);
    EditType   length     = 0;
    EditType   index      = 0;

    pBuf += cursorPos;

    /* make sure in a word */
    if (WHITE_SPACE(*pBuf))
        return;

    index = cursorPos;
    while ((index < lineLength) && !WHITE_SPACE(*pBuf))
    {
        index++;
        length++;
        pBuf++;
    }

    EXT_DeleteText(pCliEnv, cursorPos, length);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void EXT_Transpose(cli_env *pCliEnv)
{
    sbyte      tempChar;
    sbyte     *pBuf       = MEDIT_GetBufPtr(pCliEnv);
    EditType   cursorPos  = MEDIT_GetCursor(pCliEnv);
    EditType   lineLength = MEDIT_GetLength(pCliEnv);
    
    if ((cursorPos <= 0) || (cursorPos == lineLength))
        return;

    tempChar            = pBuf[cursorPos];
    pBuf[cursorPos]     = pBuf[cursorPos - 1];
    pBuf[cursorPos - 1] = tempChar;

    EXT_Refresh(pCliEnv, -1, 2);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void EXT_Backspace(cli_env *pCliEnv)
{
    sbyte       *pBuf       = MEDIT_GetBufPtr(pCliEnv);
    EditType     cursorPos  = MEDIT_GetCursor(pCliEnv);
    EditType     lineLength = MEDIT_GetLength(pCliEnv);
    CmdEditInfo *pEdit      = MEDIT_EditInfoPtr(pCliEnv);
    EditType     xPos       = EXT_GetCursorX(pEdit);

    if (0 >= cursorPos)
        return;

    /* just remove from buffer w/o screen update */
    if ( (RCC_NotEnabled(pCliEnv, kRCC_FLAG_ECHO)) &&
         (cursorPos == lineLength))
    {
        MEDIT_SetCursor(pCliEnv, --cursorPos);
        MEDIT_SetLength(pCliEnv, --lineLength);
        pBuf[lineLength] = kCHAR_NULL;
        return;
    }

    /* 
     * deal with terminals without cursor nav support 
     * Since it can't be anywhere but the end of the line,
     * we can use telnet backspace
     */
    if ((kRCC_CONN_TELNET == MCONN_GetConnType(pCliEnv)) &&
        (cursorPos == lineLength))
    {
        /* telnet is dumb and won't go to previous line */
        if (0 < xPos--)
        {
            EXT_SetCursorX(pEdit, xPos);
            RCC_EXT_PutStr(pCliEnv, kRCC_TELNET_BACKSPACE);
            MEDIT_SetCursor(pCliEnv, --cursorPos);
            MEDIT_SetLength(pCliEnv, --lineLength);
            pBuf[lineLength] = kCHAR_NULL;
            return;
        }

#ifdef __DISABLE_VT_ESCAPES__
        return;
#endif

    }

    RCC_EXT_MoveCursor(pCliEnv, -1);
    RCC_EXT_DeleteChar(pCliEnv);
}

/*-----------------------------------------------------------------------*/

RL_STATIC void
EXT_RefreshDisplay(cli_env *pCliEnv)
{
    RCC_CMD_Clear(pCliEnv);
    RCC_DB_ShowInput(pCliEnv);
}

/*-----------------------------------------------------------------------*/

/* is cursor within quoted text? */
RL_STATIC Boolean
EXT_QuotedText(cli_env *pCliEnv)
{
    sbyte       *pBuf       = MEDIT_GetBufPtr(pCliEnv);
    EditType     cursorPos  = MEDIT_GetCursor(pCliEnv);
    EditType     index;
    sbyte4       count      = 0;
    sbyte        stack[16];

    for (index = 0; index < cursorPos; index++, pBuf++)
    {
        if (('\"' == *pBuf) || ('\'' == *pBuf))
        {
            if ((0 == count) || (*pBuf == stack[count]))
            {
                if (count < sizeof(stack))
                    stack[count++] = *pBuf;
            }
            else
                count--;
        }   
    }
    return (count > 0);
}

/*-----------------------------------------------------------------------*/

/* return FALSE if end of input */

RLSTATUS
RCC_EXT_ParseInput(cli_env *pCliEnv, sbyte * pData, sbyte4 len, 
                   Boolean extended)
{
    KEY_STATE   keyState    = MCONN_GetKeyState(pCliEnv);
    cliChar     prior       = 0;
    RLSTATUS    status      = OK;
    cliChar     charIn      = 0;
    sbyte4      index;
    
    for (index = 0; index < len; index++)
    {
        charIn = *(pData++);

        /* pre-process escape codes */
        switch (keyState)
        {
        case KEY_STATE_ESC:
            switch ( TOLOWER(charIn) ) 
            { 
            case kRCC_ESC_CURSOR:
                keyState = KEY_STATE_CURSOR;
                continue;
#ifdef __VT52__
            /* vt52 cursor */        
            case 'a':
                charIn = kKEY_MOVE_UP;
                break;
            case 'b':
                charIn = kKEY_MOVE_DOWN;
                break;
            case 'c':
                charIn = kKEY_MOVE_RIGHT;
                break;
            case 'd':
                charIn = kKEY_MOVE_LEFT;
                break;
#else
            case kRCC_WORD_PREV:
                charIn = kKEY_WORD_PREV;
                break;
            case kRCC_WORD_NEXT:
                charIn = kKEY_WORD_NEXT;
                break;
            case kRCC_WORD_UPPERCASE_TO_END:
                charIn = kKEY_UPPERCASE;
                break;
            case kRCC_WORD_DELETE_TO_END:
                charIn = kKEY_DELETE_WORD_END;
                break;
#endif
            case kRCC_WORD_LOWERCASE_TO_END:
                charIn = kKEY_LOWERCASE;
                break;
            }
            keyState = KEY_STATE_DATA;
            break;

        case KEY_STATE_CURSOR:

#ifdef __DISABLE_VT_ESCAPES__
            charIn = 0;
#else
            switch(charIn)
            {
            case kRCC_CURSOR_UP:
                charIn = kKEY_MOVE_UP;
                break;
            case kRCC_CURSOR_DOWN:  
                charIn = kKEY_MOVE_DOWN;
                break;
            case kRCC_CURSOR_LEFT:  
                charIn = kKEY_MOVE_LEFT;
                break;
            case kRCC_CURSOR_RIGHT:
                charIn = kKEY_MOVE_RIGHT;
                break;
            default:
                break;
            }
#endif /* __DISABLE_VT_ESCAPES__ */
            keyState = KEY_STATE_DATA;
            break;
            
        default:
            break;

        }

        switch ( charIn ) {
        case kESC:
            keyState = KEY_STATE_ESC;
            break;
#ifndef __EOL_USES_LF__
        case kCR: /* command complete? */
#endif

#ifdef IS_INTERACTIVE_FN
            if IS_INTERACTIVE_FN(pCliEnv) 
                break;
#endif

#ifdef __EOL_USES_LF__
        case kLF:
#endif
                /* backslash 'escapes' CR, continuing line */
#ifdef kKEY_CONTINUE
            if (kKEY_CONTINUE == prior)
            {
                RCC_EXT_InsertText(pCliEnv, kEOL, kEOL_SIZE);
                continue;
            }
#endif


#ifdef IS_INTERACTIVE_FN
            if (! IS_INTERACTIVE_FN(pCliEnv))
#endif
                RCC_EXT_WriteStr(pCliEnv, kEOL);

            status = STATUS_RCC_NO_ERROR;
            break;
        case kBS:
            EXT_Backspace(pCliEnv);
            break;
        case kKEY_BREAK:
            RCC_EXT_WriteStrLine(pCliEnv, "");
            RCC_EXT_CommandLineInit(pCliEnv);
            if (extended)
                RCC_TASK_PrintPrompt(pCliEnv);
            break;
        case kTAB:
        {
            if (extended)
            {
                if (OK != RCC_DB_ExpandToken(pCliEnv, TRUE))
                    RCC_EXT_PutStr(pCliEnv, "\a"); /* beep to complain */
            }
            break;
        }
        case kKEY_DELETE:
        case kKEY_DELETE_CHAR:
            RCC_EXT_DeleteChar(pCliEnv);
            break;
        case kKEY_DELETE_FROM_START:
            EXT_DeleteFromStart(pCliEnv);
            break;
        case kKEY_DELETE_TO_END:
            EXT_DeleteToEnd(pCliEnv);
            break;
        case kKEY_LINE_START:
            RCC_EXT_LineStart(pCliEnv);
            break;
        case kKEY_LINE_END:
            RCC_EXT_LineEnd(pCliEnv);
            break;
        case kKEY_MOVE_UP:
            if (extended)
                RCC_HIST_Scroll(pCliEnv, -1);
            break;
        case kKEY_MOVE_DOWN:
            if (extended)
                RCC_HIST_Scroll(pCliEnv, 1);
            break;
        case kKEY_MOVE_LEFT:
            RCC_EXT_MoveCursor(pCliEnv, -1);
            break;
        case kKEY_MOVE_RIGHT:
            RCC_EXT_MoveCursor(pCliEnv, 1);
            break;
        case kKEY_HELP:
            if (extended && ! EXT_QuotedText(pCliEnv))
            {
                if (OK != RCC_HELP_RetrieveHelp(pCliEnv, NULL, TRUE))
                {
                    RCC_EXT_PutStr(pCliEnv, "\a"); /* beep to complain */
                }
                status = STATUS_RCC_NO_PROMPT;
            }
            else
                RCC_EXT_InsertText(pCliEnv, (sbyte *)&charIn, 1);
            break;
        case kKEY_WORD_PREV:
            EXT_WordPrevious(pCliEnv);
            break;
        case kKEY_WORD_NEXT:
            RCC_EXT_WordNext(pCliEnv);
            break;
        case kKEY_UPPERCASE:
            RCC_EXT_WordChangeCase(pCliEnv, TRUE);
            break;
        case kKEY_LOWERCASE:
            RCC_EXT_WordChangeCase(pCliEnv, FALSE);
            break;
        case kKEY_DELETE_WORD_END:
            EXT_WordDeleteEnd(pCliEnv);
            break;
        case kKEY_DELETE_WORD_START:
            EXT_WordDeleteStart(pCliEnv);
            break;
        case kKEY_TRANSPOSE:
            EXT_Transpose(pCliEnv);
            break;
        case kKEY_REFRESH_DISPLAY:
            EXT_RefreshDisplay(pCliEnv);
            break;

#ifdef kKEY_END_OF_ENTRY
        case kKEY_END_OF_ENTRY:
            RCC_EXT_WriteStrLine(pCliEnv, kEOL);
            status = STATUS_RCC_EXIT_TO_ROOT;
            break;
#endif /* kKEY_END_OF_ENTRY */

        default:
            /* exclude control chars */
            if (' ' <= charIn)
                RCC_EXT_InsertText(pCliEnv, (sbyte *)&charIn, 1);

            break;
        }  /* switch */

        RCC_EDIT_STATUS_FN(pCliEnv);

#ifdef __RCC_DEBUG__
        /* Esc - ctrl-y for debug info */
        if ((kESC == prior) && (25 == charIn))
        {
            RCC_DB_SystemSettings(pCliEnv);
            RCC_DB_ShowInput(pCliEnv);
        }
#endif

        prior = charIn;

        if (OK != status)
            break;
    }

    MCONN_SetKeyState(pCliEnv, keyState);
    return status;
}

/* 
    Main loop for handling character input. This is fed by 
    the read handler (telnet daemon or console charin)
    which deals with the raw input (handshaking and actual typed text).

    Cursor movement is being mapped to non-standard 
    ascii characters so they can be handled
    as a group with the other "end of input" characters
*/

extern RLSTATUS 
RCC_EXT_ReadCmd(cli_env *pCliEnv, Boolean extended)
{
    cliChar      charIn      = 0;
    RLSTATUS     status      = OK;
    ReadHandle  *Reader      = MCONN_GetReadHandle(pCliEnv);

#if defined(RCC_LOG_INPUT_FN) || defined(RCC_LOG_IO_FN)
    sbyte       *pBuffer     = MEDIT_GetBufPtr(pCliEnv);
#endif
#ifdef RCC_LOG_IO_FN
    sbyte       *pPrompt     = MEDIT_Prompt(pCliEnv);
#endif
      
    RCC_EXT_CommandLineInit(pCliEnv);
    RCC_EXT_OutputReset(pCliEnv);
    RCC_EnableFeature(pCliEnv, kRCC_FLAG_INPUT);

    while (OK == status)
    {
        if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_KILL))
            break;

        if (OK != (status = Reader(pCliEnv, &charIn)))
            break;

        status = RCC_EXT_ParseInput(pCliEnv, (sbyte *)&charIn, 1, extended);
    }
        
#ifdef RCC_LOG_INPUT_FN
    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_LOG_INPUT))
        RCC_LOG_INPUT_FN(pBuffer);
#endif

#ifdef RCC_LOG_IO_FN
    if (RCC_IsEnabled(pCliEnv, kRCC_FLAG_LOG_INPUT))
        RCC_LOG_IO_FN(pPrompt, pBuffer);
#endif

    RCC_DisableFeature(pCliEnv, kRCC_FLAG_INPUT);

    return status;
}

/*-----------------------------------------------------------------------*/

extern void 
RCC_EXT_LocalEcho(cli_env *pCliEnv, Boolean enable)
{
    if (enable)
        RCC_EnableFeature(pCliEnv,  kRCC_FLAG_ECHO);
    else
        RCC_DisableFeature(pCliEnv, kRCC_FLAG_ECHO);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_EXT_Gets(cli_env *pCliEnv, sbyte *pBuffer, sbyte4 buffSize,
             sbyte4 *byteCount, Boolean echo)
{
    RLSTATUS  status     = OK;
    sbyte    *pOldBufPtr = MEDIT_GetBufPtr(pCliEnv);
    sbyte4    oldBufSize = MEDIT_GetBufSize(pCliEnv);
    Boolean   oldEcho    = RCC_IsEnabled(pCliEnv, kRCC_FLAG_ECHO);

    MEDIT_SetBufPtr(pCliEnv,  pBuffer);
    MEDIT_SetBufSize(pCliEnv, buffSize);
    RCC_EXT_CommandLineInit(pCliEnv);
    RCC_EXT_LocalEcho(pCliEnv, echo);

    status = RCC_EXT_ReadCmd(pCliEnv, FALSE);
    *byteCount = MEDIT_GetLength(pCliEnv);

    MEDIT_SetBufPtr(pCliEnv,  pOldBufPtr);
    MEDIT_SetBufSize(pCliEnv, oldBufSize);
    MEDIT_SetLength(pCliEnv,  0);
    MEDIT_SetCursor(pCliEnv,  0);
    RCC_EXT_LocalEcho(pCliEnv, oldEcho);

    return status;
}


#endif /* __RCC_ENABLED__ */
