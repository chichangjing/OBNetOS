/*  
 *  rcc_ext.h
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

#ifndef __RCC_EXT_H__
#define __RCC_EXT_H__

/* System state flags */
#define kRCC_FLAG_NEWMODE    0x00000001 /* entered/exited current mode       */
#define kRCC_FLAG_ECHO       0x00000002 /* enables output                    */
#define kRCC_FLAG_MODE       0x00000004 /* disallow intermediate modes       */
#define kRCC_FLAG_HISTORY    0x00000008 /* capture script to history buffer  */
#define kRCC_FLAG_CONVERT    0x00000010 /* "\r\n" from string to actual      */
#define kRCC_FLAG_RAW        0x00000020 /* output w/o updating screen info   */
#define kRCC_FLAG_NOPRINT    0x00000080 /* output aborted at "more" prompt   */
#define kRCC_FLAG_SNMPAUTO   0x00000100 /* auto-commit snmp changes          */
#define kRCC_FLAG_RETRIES    0x00000200 /* reprint input w/ cursor at error  */
#define kRCC_FLAG_HARDWRAP   0x00000400 /* use hard wrap in output           */
#define kRCC_FLAG_ALIASED    0x00000800 /* input has alias command - reparse */
#define kRCC_FLAG_MYPROMPT   0x00001000 /* custom prompt is in effect        */
#define kRCC_FLAG_INPUT      0x00002000 /* in input mode (for logging)       */
#define kRCC_FLAG_DASHES     0x00004000 /* uses dashes in error line         */
#define kRCC_FLAG_KILL       0x00008000 /* exit thread                       */
#define kRCC_FLAG_MORE       0x00010000 /* paginate output                   */
#define kRCC_FLAG_LOG_INPUT  0x00080000 /* enable logging input              */
#define kRCC_FLAG_LOG_OUTPUT 0x00100000 /* enable logging output             */
#define kRCC_FLAG_LOG_IO     0x00200000 /* enable logging output             */
#define kRCC_FLAG_LAST_INPUT 0x00400000 /* recycle input buffer              */
#define kRCC_FLAG_EXEC       0x00800000 /* script is running                 */
#define kRCC_FLAG_STATUS     0x01000000 /* show edit/line status             */
#define kRCC_FLAG_RM_PROMPT  0x02000000 /* prompt contains rapidmark         */
#define kRCC_FLAG_ORDER_NONE 0x04000000 /* no parameter ordering             */
#define kRCC_FLAG_ORDER_SOME 0x08000000 /* unnamed parameter ordering        */
#define kRCC_FLAG_ORDER_FULL 0x10000000 /* full parameter ordering           */
#define kRCC_FLAG_WARNING    0x20000000 /* timeout message has fired off     */
#define kRCC_FLAG_ERROR_SET  0x40000000 /* only need one error notification  */
/* debugging flags */
#define kRCC_DEBUG_MEMORY    0x00000001 /* show system memory used           */
#define kRCC_DEBUG_SHOWEXEC  0x00000002 /* dump exec name to console         */
#define kRCC_DEBUG_STACK     0x00000004 /* dump parent nodes to console      */

#define kRCC_FLAG_ORDER_MASK (kRCC_FLAG_ORDER_NONE | \
                              kRCC_FLAG_ORDER_SOME | \
                              kRCC_FLAG_ORDER_FULL)

/* 
   vt100 cursor movement escape codes
   - abstract out to support other term types!
*/
#define kRCC_VTTERM_UP      "\x1B[%dA"
#define kRCC_VTTERM_DN      "\x1B[%dB"
#define kRCC_VTTERM_RT      "\x1B[%dC"
#define kRCC_VTTERM_LT      "\x1B[%dD"

/* telnet screen commands */
#define kRCC_TELNET_BACKSPACE   "\b \b"
#define kRCC_TELNET_CLEAR       "\33[2J"


/* 
   cursor/keyboard escape codes 
   -- is there an rfc for these? is this just vt100? 
*/
#define kRCC_ESC_CURSOR   0x5b
#define kRCC_CURSOR_UP    0x41
#define kRCC_CURSOR_DOWN  0x42
#define kRCC_CURSOR_RIGHT 0x43
#define kRCC_CURSOR_LEFT  0x44

/* IOS-like escape codes */
#define kRCC_WORD_PREV               'b'
#define kRCC_WORD_NEXT               'f'
#define kRCC_WORD_UPPERCASE_TO_END   'c'
#define kRCC_WORD_DELETE_TO_END      'd'
#define kRCC_WORD_LOWERCASE_TO_END   'l'

#define CONTROL_KEY(x)          ((char) (x < 'a')   ? (x + 'a' - 1) : (x - 'a' + 1))
#define ESCAPE_KEY(key)         ((char) (key > 127) ? 128 - key : 128 + key)

#define kKEY_HELP               ((char) '?')
#define kKEY_DELETE             ((char)(0x7F))

#define kKEY_LINE_START         CONTROL_KEY('a')
#define kKEY_MOVE_LEFT          CONTROL_KEY('b')
#define kKEY_BREAK              CONTROL_KEY('c')
#define kKEY_DELETE_CHAR        CONTROL_KEY('d')
#define kKEY_LINE_END           CONTROL_KEY('e')
#define kKEY_MOVE_RIGHT         CONTROL_KEY('f')
#define kKEY_DELETE_TO_END      CONTROL_KEY('k')
#define kKEY_REFRESH_DISPLAY    CONTROL_KEY('l')
#define kKEY_MOVE_DOWN          CONTROL_KEY('n')
#define kKEY_MOVE_UP            CONTROL_KEY('p')
#define kKEY_TRANSPOSE          CONTROL_KEY('t')
#define kKEY_DELETE_FROM_START  CONTROL_KEY('u')
#define kKEY_DELETE_WORD_START  CONTROL_KEY('w')
#define kKEY_END_OF_ENTRY       CONTROL_KEY('z')

#define kKEY_WORD_PREV          ESCAPE_KEY(kRCC_WORD_PREV)
#define kKEY_WORD_NEXT          ESCAPE_KEY(kRCC_WORD_NEXT)
#define kKEY_UPPERCASE          ESCAPE_KEY(kRCC_WORD_UPPERCASE_TO_END)
#define kKEY_DELETE_WORD_END    ESCAPE_KEY(kRCC_WORD_DELETE_TO_END)
#define kKEY_LOWERCASE          ESCAPE_KEY(kRCC_WORD_LOWERCASE_TO_END)

#define INDENT_SET_HERE(pEnv)   HELP_SET(pEnv, HELP_FLAG_INDENT)  ? \
                                MEDIT_SetIndent(pEnv, MEDIT_GetXPos(pEnv)) : FALSE


/* if defined, allows multi-line editing */
/**
#define kKEY_CONTINUE           '\\'
**/

/* not supported yet - for multiple commands on one line */
/**
  #define kKEY_SEPARATOR         ';'
**/

#ifdef __cplusplus
extern "C" {
#endif

extern void     RCC_EXT_CommandLineInit(cli_env *pCliEnv);
extern EditType RCC_EXT_GetWidth(cli_env *pCliEnv);
extern EditType RCC_EXT_GetHeight(cli_env *pCliEnv);
extern void     RCC_EXT_SetWidth(cli_env *pCliEnv, EditType width);
extern void     RCC_EXT_SetHeight(cli_env *pCliEnv, EditType height);
extern void     RCC_EXT_GetScreenCoord(cli_env *pCliEnv, sbyte4 *xPos, sbyte4 *yPos);
extern void     RCC_EXT_DeleteChar(cli_env *pCliEnv);
extern void     RCC_EXT_EraseLine(cli_env *pCliEnv);
extern void     RCC_EXT_EnablePrint(cli_env *pCliEnv, Boolean enable);
extern void     RCC_EXT_FlushOutput(cli_env *pCliEnv);
extern void     RCC_EXT_FreeOutput(cli_env *pEnv);
extern sbyte    RCC_EXT_GetPriorChar(cli_env *pCliEnv);
extern RLSTATUS RCC_EXT_Gets(cli_env *pCliEnv, sbyte *pBuffer, sbyte4 buffSize,
                sbyte4 *byteCount, Boolean echo);
extern RLSTATUS RCC_EXT_OutputCreate(cli_env *pEnv, sbyte *pBuffer, sbyte4 size);
extern void     RCC_EXT_OutputReset(cli_env *pEnv);
extern void     RCC_EXT_InsertText(cli_env *pCliEnv, sbyte *text, sbyte4 length);
extern void     RCC_EXT_LineEnd(cli_env *pCliEnv);
extern void     RCC_EXT_LineStart(cli_env *pCliEnv);
extern void     RCC_EXT_LocalEcho(cli_env *pCliEnv, Boolean enable);
extern void     RCC_EXT_MoveCursor(cli_env *pCliEnv, EditType offset);
extern RLSTATUS RCC_EXT_ParseInput(cli_env *pCliEnv, sbyte * pData, 
                                   sbyte4 len, Boolean extended);
extern void     RCC_EXT_PrintString(cli_env *pCliEnv, sbyte *pString, sbyte4 length, sbyte fill);
extern RLSTATUS RCC_EXT_Put(cli_env *pCliEnv, sbyte *pBuf, sbyte4 bufLen);
extern RLSTATUS RCC_EXT_PutStr(cli_env *pCliEnv, sbyte *pBuf);
extern RLSTATUS RCC_EXT_PutStrLine(cli_env *pCliEnv, sbyte *pBuf);
extern RLSTATUS RCC_EXT_ReadCmd(cli_env *pCliEnv, Boolean extended);
extern void     RCC_EXT_SetCursor(cli_env *pCliEnv, EditType position);
extern void     RCC_EXT_StringPrint(cli_env *pEnv, sbyte *string, sbyte4 length, sbyte fill);
extern RLSTATUS RCC_EXT_Write(cli_env *pCliEnv, sbyte *pBuf, sbyte4 bufLen);
extern RLSTATUS RCC_EXT_WriteStr(cli_env *pCliEnv, sbyte *pBuf);
extern RLSTATUS RCC_EXT_WriteStrLine(cli_env *pCliEnv, sbyte *pBuf);

#ifdef __cplusplus
}
#endif

#endif /* __RCC_EXT_H__ */
