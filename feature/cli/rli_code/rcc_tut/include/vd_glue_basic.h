/*  
 *  vd_glue_basic.h
 *
 *  This is a part of the RapidControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */

#ifndef __VD_GLUE_BASIC_HEADER__
#define __VD_GLUE_BASIC_HEADER__

extern int         gGlobalInteger;
extern long        gGlobalHexInteger;
extern IpAddress   gIpAddress;
extern IpAddress   gNetMask;

extern char        gName[kVD_GLUE_MaxStringSize];


/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_BASIC_ReadPrivateVariable(environment *pEnv, void *pOutputBuf,
                                  void *pObject, char *pArgs);


/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_BASIC_ReadHexadecimalVariable(environment *pEnv, void *pOutputBuf,
                                      void *pObject, char *pArgs);


/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_BASIC_ReadIpAddress(environment *pEnv,  void *pOutputBuf,
                            void *pObject, char *pArgs);


/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_BASIC_WriteIpAddress(environment *pEnv, void *pDest, void *pInputBuf, ...);


/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_BASIC_ValidateIpAddress(environment *pEnv, void *pInputBuf);


/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_BASIC_ValidateNetMask(environment *pEnv, void *pInputBuf);


/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_BASIC_ReadSecureID(environment *pEnv, void *pOutputBuf, 
                           void *pObject, char *pArgs);


/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_BASIC_WriteSecureID(environment *pEnv, void *pDest, void *pInputBuf, ...);


/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_BASIC_ValidateSecureID(environment *pEnv, void *pInputBuf);


/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_GLUE_BASIC_ValidateName(environment *pEnv, void *pInputBuf);


/*-----------------------------------------------------------------------*/
extern void 
VD_GLUE_BASIC_ReadNextHop(environment *pEnv, void *pOutputBuf,
                                 void *pObject, char *pArgs);

/*-----------------------------------------------------------------------*/
extern void 
VD_MOTDRead(environment *pEnv, void *pOutputBuf, void *pObject, sbyte *pArgs);

/*-----------------------------------------------------------------------*/
extern RLSTATUS 
VD_MOTDWrite(environment *pEnv, void *pDest, void *pInputBuf, sbyte *pArgs, ...);

/*-----------------------------------------------------------------------*/

extern void VD_CLIPromptExampleRead(environment *pEnv, void *pOutputBuf, 
											void *pObject, sbyte *pArgs);

/*-----------------------------------------------------------------------*/

extern RLSTATUS VD_CLIPromptExampleWrite(environment *pEnv, void *pDest, 
											void *pInputBuf, sbyte *pArgs, ...);

/*-----------------------------------------------------------------------*/
extern void VD_GLUE_BASIC_Init( void *pArg );

#if 1	/* Added by hejg */
extern void VD_ReadPortNo(environment *pEnv, void *pOutputBuf, void *pObject, sbyte *pArgs);
extern RLSTATUS VD_WritePortNo(environment *pEnv, void *pDest, void *pInputBuf, ...);
extern void VD_ReadDomainID(environment *pEnv, void *pOutputBuf, void *pObject, sbyte *pArgs);
extern RLSTATUS VD_WriteDomainID(environment *pEnv, void *pDest, void *pInputBuf, ...);
#endif	/* Added end */


#endif /* __VD_GLUE_BASIC_HEADER__ */
