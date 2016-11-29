/*  
 *  rcc_help.h
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


#ifndef __RCC_HELP_H__
#define __RCC_HELP_H__

typedef struct EditKeys {
    ubyte  key;
    sbyte *helpText;
} EditKeys;

#define HELP_FLAG_SHOW_NO       0x00000001
#define HELP_FLAG_HIDE_BLANK    0x00000002
#define HELP_FLAG_NEXT          0x00000004
#define HELP_FLAG_EXECUTABLE    0x00000008
#define HELP_FLAG_ALLOC_TITLE   0x00000010
#define HELP_FLAG_ALLOC_PREFIX  0x00000020
#define HELP_FLAG_ALLOC_DELIM   0x00000040
#define HELP_FLAG_ALLOC_QUOTE   0x00000080
#define HELP_FLAG_ALLOC_UNQUOTE 0x00000100
#define HELP_FLAG_ALLOC_NODE    0x00000200
#define HELP_FLAG_ERROR         0x00000400
#define HELP_FLAG_THIS          0x00000800
#define HELP_FLAG_SAME_LINE     0x00001000
#define HELP_FLAG_PARAMS        0x00002000
#define HELP_FLAG_SKIP_NODE     0x00004000
#define HELP_FLAG_INDENT        0x00008000
#define HELP_FLAG_FIXED_WIDTH   0x00010000
#define HELP_FLAG_WIDTH         0x00020000
#define HELP_FLAG_LEADER        0x00040000

/* help defaults */

#ifndef kRCC_HELP_WIDTH
#define kRCC_HELP_WIDTH 30
#endif

#ifndef kRCC_HELP_LEADER
#define kRCC_HELP_LEADER ' '
#endif

#ifndef kRCC_HELP_TITLE
#define kRCC_HELP_TITLE NULL
#endif

#ifndef kRCC_HELP_PREFIX
#define kRCC_HELP_PREFIX NULL
#endif

#ifndef kRCC_HELP_NODE_DELIMITER
#define kRCC_HELP_NODE_DELIMITER NULL
#endif

#ifndef kRCC_HELP_DELIMITER
#define kRCC_HELP_DELIMITER NULL
#endif

#ifndef kRCC_HELP_SYNTAX
#define kRCC_HELP_SYNTAX NULL
#endif

#ifndef kRCC_HELP_SYNTAX_TAIL
#define kRCC_HELP_SYNTAX_TAIL NULL
#endif

#ifndef __RCC_NO_HELP_INDENT__
#define HELP_INDENT_SET_M(pCliEnv)      INDENT_SET_HERE(pCliEnv)
#define HELP_INDENT_RESET_M(pCliEnv)    MEDIT_SetIndent(pCliEnv, 0)
#else
#define HELP_INDENT_SET_M(pCliEnv)
#define HELP_INDENT_RESET_M(pCliEnv)
#endif


#ifdef __cplusplus
extern "C" {
#endif

extern void     RCC_HELP_CustomHelpString(cli_env *pCliEnv, cmdNode *pNode, 
                                          paramDefn *pParam);
extern RLSTATUS RCC_HELP_EditHelp(cli_env *pCliEnv);
extern RLSTATUS RCC_HELP_ErrorHelp(cli_env *pCliEnv, cmdNode *pNode);
extern RLSTATUS RCC_HELP_Globals(cli_env *pCliEnv);
extern void     RCC_HELP_PrintLabel(cli_env *pCliEnv, cmdNode *pNode, 
                                    sbyte *text, sbyte *pWrapper);
extern void     RCC_HELP_NodeHandler(cli_env *pCliEnv, cmdNode *pNode);
extern RLSTATUS RCC_HELP_RetrieveHelp(cli_env *pCliEnv, cmdNode *pNode, 
                                      Boolean immediate);
extern RLSTATUS RCC_HELP_SetFeature(cli_env * pCliEnv, ubyte4 feature, 
                                    sbyte * pText);

#ifdef __cplusplus
}
#endif

#endif /* __RCC_HELP_H__ */
