/*  
 *  rc_errors.h
 *
 *  This is a part of the OpenControl SDK source code library. 
 *
 *  Copyright (C) 1998 Rapid Logic, Inc.
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
/*


$History: rc_errors.h $
 * 
 * *****************  Version 26  *****************
 * User: Pstuart      Date: 7/27/01    Time: 11:09a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * add disconnect error
 * 
 * *****************  Version 25  *****************
 * User: Pstuart      Date: 7/05/00    Time: 2:56p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added ERROR_GENERAL_INVALID_PATH
 * 
 * *****************  Version 24  *****************
 * User: Ty           Date: 6/07/00    Time: 8:41a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added errors for ssl error conditions
 * 
 * *****************  Version 23  *****************
 * User: Pstuart      Date: 5/30/00    Time: 2:06p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added ERROR_GENERAL_OUT_OF_RANGE
 * 
 * *****************  Version 22  *****************
 * User: Pstuart      Date: 5/24/00    Time: 2:55p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added ERROR_GENERAL_INVALID_RAPIDMARK
 * 
 * *****************  Version 21  *****************
 * User: Pstuart      Date: 5/23/00    Time: 10:17a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added ERROR_GENERAL_BUFFER_OVERRUN
 * 
 * *****************  Version 20  *****************
 * User: Pstuart      Date: 5/18/00    Time: 1:14p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added ERROR_CONVERSION_TOO_LONG
 * 
 * *****************  Version 19  *****************
 * User: Pstuart      Date: 5/02/00    Time: 5:47p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added ERROR_GENERAL_FILE_NOT_FOUND
 * 
 * *****************  Version 18  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Include history and enable auto archiving feature from VSS

$/Rapid Logic/Code Line/rli_code/include/rc_errors.h

17   Pstuart     4/25/00  11:57a   Checked in $/Rapid Logic/Code Line/rli_code/include    
16   Schew       3/22/00   7:32p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Pstuart     3/22/00  10:53a   Labeled 'PreCodeRename'                                
     David       3/07/00   8:06p   Labeled 'RC 3.01 Final Release'                        
     Builder     3/02/00   5:18p   Labeled 'RC 301 Build20000302'                         
     Builder     2/29/00   6:48p   Labeled 'Build301 20000229'                            
     Builder     2/28/00   6:35p   Labeled 'RC301 Build20000228'                          
     Builder     2/24/00   4:01p   Labeled 'RC301 Build20000224'                          
15   Pstuart     2/19/00   8:52a   Checked in $/Rapid Logic/Code Line/rli_code/include    
     David      12/20/99  10:55a   Labeled 'RC 3.0 Final Release'                         
     Builder    12/15/99   6:36p   Labeled 'RC30 Build19991215'                           
     Builder    12/14/99   7:08p   Labeled 'RC30 Build19991214'                           
     Builder    12/10/99   5:55p   Labeled 'RC30 Build19991210'                           
     Builder    11/17/99   4:32p   Labeled 'RC30 Build19991117 Release'                   
     Builder    11/16/99   5:37p   Labeled 'RC30 Build19991116'                           
     Builder    11/15/99   5:59p   Labeled 'RC30 Build19991115'                           
     Builder    11/12/99   5:27p   Labeled 'RC30 Build19991112'                           
     Builder    11/11/99   7:00p   Labeled 'RC30 Build19991111'                           
     Builder    11/10/99   7:12p   Labeled 'RC30 Build19991110'                           
     Builder    11/08/99   6:09p   Labeled 'RC30 Build19991108'                           
     Builder    11/05/99   6:33p   Labeled 'RC30 Build19991105'                           
     Builder    11/04/99   5:04p   Labeled 'RC30 Build19991104'                           
     Builder    11/03/99   6:23p   Labeled 'RC3.0 Build 19991103'                         
     Builder    11/01/99   5:51p   Labeled 'RC 3.0 Build 19991101'                        
     Builder    10/29/99   5:30p   Labeled 'RC30 Build 19991029'                          
     David       9/28/99   5:42p   Labeled 'RC 3.0 beta 2 - 9-28-99'                      
     Builder     8/10/99   2:41p   Labeled 'RC 3.0 alpha 4 limited release'               
     David       8/04/99   1:24p   Labeled 'RC 2.4 release'                               
     Builder     7/29/99   5:56p   Labeled 'RCA/RCW/Mibway 2.4 Final'                     
     David       7/20/99   2:04p   Labeled 'RC 2.4 beta 6 training '                      
14   Kedron      6/04/99   1:51p   Checked in $/Rapid Logic/Code Line/rli_code/include    
13   Kedron      5/27/99   8:54a   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Leech       4/29/99  11:10a   Labeled 'WC/JC 2.31 Release - 4-29-99'                 
     Leech       4/27/99   2:10p   Labeled 'WC/JC 2.31 beta 7- 4-27-99'                   
     Leech       4/26/99   2:42p   Labeled 'WC/JC 2.31 beta6 4-26-99'                     
     David       4/14/99   3:53p   Labeled 'WC/JC 2.31 beta4 '                            
     David       4/07/99   6:06p   Labeled 'WC/JC2.31 beta2 - 4-7-99'                     
     David       4/06/99   3:38p   Labeled 'WC2.31 beta 1 - 4-6-99'                       
     Mredison    2/19/99   6:42p   Labeled 'WebControl 2.3 Final Gold'                    
     David       2/17/99  10:14a   Labeled 'JC2.3 Final Release - 2-17-99'                
     David       2/16/99   6:19p   Labeled 'WC 2.3 Final Release - 2-16-99'               
     David       2/15/99   5:56p   Labeled 'WC2.3 beta11 - 2-15-99'                       
     David       2/12/99   6:06p   Labeled 'JC 2.3 beta9 - 2-12-99'                       
     David       2/12/99   6:06p   Labeled 'WC 2.3 beta 9 - 2-12-99'                      
     David       2/11/99   4:51p   Labeled 'JC 2.3 beta8 - 2-11-99'                       
     David       2/11/99   4:51p   Labeled 'WC 2.3 beta 8 - 2-11-99'                      
12   Henry       2/10/99   2:07p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     David       2/09/99  11:30a   Labeled 'WC2.3 beta5 - 2-9-99'                         
     David       2/08/99   5:28p   Labeled 'JC2.3 beta5 - 2-8-99'                         
     David       2/05/99   6:55p   Labeled 'WC2.3 beta4 - 2-5-99'                         
     David       2/05/99   6:55p   Labeled 'JC2.3 beta4 - 2-5-99'                         
11   Henry       2/05/99  11:24a   Checked in $/Rapid Logic/Code Line/rli_code/include    
     David       2/03/99   7:23p   Labeled 'WC 2.3 beta3 - 2-3-99'                        
     David       2/03/99   7:23p   Labeled 'JC2.3 beta3 - 2-3-99'                         
10   Henry       2/03/99   6:38p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     David       2/01/99   7:00p   Labeled 'JC2.3 beta 2 - 2/1/99'                        
     David       1/29/99   6:13p   Labeled 'JC2.3beta1'                                   
9    Henry       1/19/99   5:22p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     David       1/15/99   3:16p   Labeled 'jc 2.2 rc4'                                   
8    Henry       1/14/99   2:00p   Checked in $/Rapid Logic/Code Line/rli_code/include    
7    Henry       1/13/99   4:04p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Henry       1/08/99   4:20p   Labeled 'jc 2.2 rc3'                                   
     Henry      12/18/98   5:12p   Labeled 'jc 2.2 rc2a'                                  
     Henry      12/18/98   4:34p   Labeled 'jc 2.2 rc2'                                   
     David      12/10/98   6:29p   Labeled 'JC 12/10'                                     
     Henry      12/07/98   5:27p   Labeled 'JC 12/7'                                      
     Henry      12/04/98   3:35p   Labeled 'Build 12/4'                                   
6    Henry      11/20/98   2:04p   Checked in $/Rapid Logic/Code Line/rli_code/include    
5    Kedron     10/30/98   6:20p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Leech       8/07/98   4:28p   Labeled '2.01'                                         
4    Kedron      8/05/98  10:01a   Checked in $/Rapid Logic/Code Line/rli_code/include    
3    Kedron      8/03/98  10:16a   Checked in $/Rapid Logic/Code Line/rli_code/include    
2    Kedron      8/02/98   7:23p   Checked in $/Rapid Logic/Code Line/rli_code/include    
1    Leech       7/27/98   6:25p   Created errors.h                                       


*/

#ifndef __ERRORS_HEADER__
#define __ERRORS_HEADER__

/* General purpose errors. */
#define ERROR_GENERAL                           -100
#define ERROR_GENERAL_NO_DATA                   ( ERROR_GENERAL - 1  )
#define ERROR_GENERAL_NOT_FOUND                 ( ERROR_GENERAL - 2  )
#define ERROR_GENERAL_ACCESS_DENIED             ( ERROR_GENERAL - 3  )
#define ERROR_GENERAL_NOT_EQUAL                 ( ERROR_GENERAL - 4  )
#define ERROR_GENERAL_ILLEGAL_VALUE             ( ERROR_GENERAL - 5  )
#define ERROR_GENERAL_CREATE_TASK            	( ERROR_GENERAL - 6  )
#define ERROR_GENERAL_NULL_POINTER            	( ERROR_GENERAL - 7  )
#define ERROR_GENERAL_DATA_AMBIG                ( ERROR_GENERAL - 8  )
#define ERROR_GENERAL_FILE_NOT_FOUND            ( ERROR_GENERAL - 9  )
#define ERROR_GENERAL_BUFFER_OVERRUN            ( ERROR_GENERAL - 10 )
#define ERROR_GENERAL_INVALID_RAPIDMARK         ( ERROR_GENERAL - 11 )
#define ERROR_GENERAL_OUT_OF_RANGE              ( ERROR_GENERAL - 12 )
#define ERROR_GENERAL_INVALID_PATH              ( ERROR_GENERAL - 13 )

/* Errors returned by the Post Handler */
#define ERROR_POST_GENERAL                      -200
#define ERROR_POST_NO_MORE_MAGICMARKUPS         ( ERROR_POST_GENERAL - 1 )

/* Errors returned by the Get Handler */
#define ERROR_GET_GENERAL                       -300
#define ERROR_TX_ENG_BAD_MAGICMARKUP            ( ERROR_GET_GENERAL - 1 )

/* Errors returned by the datatype conversion routines */
#define ERROR_CONVERSION_GENERAL                -400
#define ERROR_CONVERSION_INCORRECT_TYPE         ( ERROR_CONVERSION_GENERAL - 1 )
#define ERROR_CONVERSION_OVERFLOW               ( ERROR_CONVERSION_GENERAL - 2 )
#define ERROR_CONVERSION_UNDERFLOW              ( ERROR_CONVERSION_GENERAL - 3 )
#define ERROR_CONVERSION_TOO_LONG               ( ERROR_CONVERSION_GENERAL - 4 )

/* Errors returned by the memory management system.*/
#define ERROR_MEMMGR_GENERAL                    -500
#define ERROR_MEMMGR_BAD_MEMSIZE                ( ERROR_MEMMGR_GENERAL - 1 )
#define ERROR_MEMMGR_INITIALIZATION             ( ERROR_MEMMGR_GENERAL - 2 )
#define ERROR_MEMMGR_NO_MEMORY                  ( ERROR_MEMMGR_GENERAL - 3 )
#define ERROR_MEMMGR_BAD_POINTER                ( ERROR_MEMMGR_GENERAL - 4 )
#define ERROR_MEMMGR_BAD_FREE                   ( ERROR_MEMMGR_GENERAL - 5 )
#define ERROR_MEMMGR_MEMORY_CORRUPTION          ( ERROR_MEMMGR_GENERAL - 6 )
#define ERROR_MEMMGR_INVALID_LENGTH             ( ERROR_MEMMGR_GENERAL - 7 )

/* Errors returned by the decompression system.*/
#define ERROR_DECOMP_GENERAL                    -600
#define ERROR_DECOMP_BAD_PKZIP_FILE             ( ERROR_DECOMP_GENERAL - 1 )
#define ERROR_DECOMP_BAD_FIRST_ENTRY            ( ERROR_DECOMP_GENERAL - 2 )
#define ERROR_DECOMP_GZIP_FILE_NOT_DEFLATED     ( ERROR_DECOMP_GENERAL - 3 )
#define ERROR_DECOMP_MULTIPART_GZIP_FILES       ( ERROR_DECOMP_GENERAL - 4 )
#define ERROR_DECOMP_INVALID_FILE_FORMAT        ( ERROR_DECOMP_GENERAL - 5 )
#define ERROR_DECOMP_FORMAT_VIOLATION           ( ERROR_DECOMP_GENERAL - 6 )
#define ERROR_DECOMP_LENGTH_MISMATCH            ( ERROR_DECOMP_GENERAL - 7 )
#define ERROR_DECOMP_CRC_MISMATCH               ( ERROR_DECOMP_GENERAL - 8 )
#define ERROR_DECOMP_DATA_LENGTH                ( ERROR_DECOMP_GENERAL - 9 )

/* Errors returned by the e-mail system.*/
#define ERROR_SMTP_GENERAL                      -700
#define ERROR_SMTP_NOT_INIT                     ( ERROR_SMTP_GENERAL - 1 )
#define ERROR_SMTP_ABORT                        ( ERROR_SMTP_GENERAL - 2 )

/* Errors returned by the JC system. */
#define ERROR_JC_GENERAL                        -800
#define ERROR_JC_DATALEN_NOT_FOUND              ( ERROR_JC_GENERAL - 1 )
#define ERROR_JC_NAMING_SERVICE_DATABASE_EMPTY  ( ERROR_JC_GENERAL - 2 )
#define ERROR_JC_CRC_CHECK_FAILED               ( ERROR_JC_GENERAL - 3 )
#define ERROR_JC_PACKET_HEADER                  ( ERROR_JC_GENERAL - 4 )
#define ERROR_JC_INVALID_MICROTAG               ( ERROR_JC_GENERAL - 5 )
#define ERROR_JC_NO_MICROTAG                    ( ERROR_JC_GENERAL - 6 )
#define ERROR_JC_BAD_PACKET_RECEIVED            ( ERROR_JC_GENERAL - 7 )

/* error codes newly defined for OCP */
#define ERROR_OCP_GENERAL                        -810
#define ERROR_OCP_BADHEADER                     (ERROR_OCP_GENERAL -1)
#define ERROR_OCP_PROTOCOL_VERSION              (ERROR_OCP_GENERAL -3)
#define ERROR_OCP_UNKOWN_MSGTYPE                (ERROR_OCP_GENERAL -4)
#define ERROR_OCP_MSGTYPE_NOTIMPLEMENTED        (ERROR_OCP_GENERAL -5)
#define ERROR_OCP_BAD_CHUNK                    	(ERROR_OCP_GENERAL -6)
#define ERROR_OCP_BAD_CRC                    	(ERROR_OCP_GENERAL -7)
#define ERROR_OCP_AUTH_FAIL                     (ERROR_OCP_GENERAL -8)
#define ERROR_OCP_CONNECTION_FAIL         	    (ERROR_OCP_GENERAL -9)
#define ERROR_OCP_TXPACKET_LENGTH               (ERROR_OCP_GENERAL -10)
#define	ERROR_OCP_MSGERROR_MSGFLUSHED			(ERROR_OCP_GENERAL -11)
#define ERROR_OCP_CREATE_TASK            		( ERROR_OCP_GENERAL - 12 )
#define ERROR_OCP_RXMSG_LENGTH                	(ERROR_OCP_GENERAL - 13)
#define ERROR_OCP_CONN_TIMEOUT                	(ERROR_OCP_GENERAL - 14)
#define ERROR_OCP_SNMP_NOT_ENABLED             	(ERROR_OCP_GENERAL - 15)

/* these codes cause error response (e.g. Mm chunk) */
/* but might not fail message (depends on location of response code) */
#define ERROR_OCP_MMGENERAL                      -840
#define ERROR_OCP_BAD_STRING                    (ERROR_OCP_MMGENERAL -1)
#define ERROR_OCP_BAD_MMNAME                    (ERROR_OCP_MMGENERAL -2)
#define ERROR_OCP_TXCHUNK_LENGTH                (ERROR_OCP_MMGENERAL -3)
#define ERROR_OCP_NOSUCHINSTANCE                (ERROR_OCP_MMGENERAL -4)

/* Non-Error warnings from OCP	*/
#define	WARNING_OCPGENERAL						-850
#define WARNING_OCP_NO_CONNECTIONS            	( WARNING_OCPGENERAL - 1 )

/* Errors returned by the pre-existing system.*/
#define SYS_ERROR_GENERAL                       -1000
#define SYS_ERROR_NO_MEMORY                     ( SYS_ERROR_GENERAL - 1 )
#define SYS_ERROR_UNEXPECTED_END                ( SYS_ERROR_GENERAL - 2 )

#define SYS_ERROR_MUTEX_GENERAL                 -1100
#define SYS_ERROR_MUTEX_CREATE                  ( SYS_ERROR_MUTEX_GENERAL - 1 )
#define SYS_ERROR_MUTEX_WAIT                    ( SYS_ERROR_MUTEX_GENERAL - 2 )
#define SYS_ERROR_MUTEX_RELEASE                 ( SYS_ERROR_MUTEX_GENERAL - 3 )

#define SYS_ERROR_SOCKET_GENERAL                -1200
#define SYS_ERROR_SOCKET_CREATE                 ( SYS_ERROR_SOCKET_GENERAL - 1 )
#define SYS_ERROR_SOCKET_BIND                   ( SYS_ERROR_SOCKET_GENERAL - 2 )
#define SYS_ERROR_SOCKET_THREAD                 ( SYS_ERROR_SOCKET_GENERAL - 3 )
#define SYS_ERROR_SOCKET_LISTEN                 ( SYS_ERROR_SOCKET_GENERAL - 4 )
#define SYS_ERROR_SOCKET_ACCEPT                 ( SYS_ERROR_SOCKET_GENERAL - 5 )
#define SYS_ERROR_SOCKET_CREATE_TASK            ( SYS_ERROR_SOCKET_GENERAL - 6 )
#define SYS_ERROR_SOCKET_DELETE                 ( SYS_ERROR_SOCKET_GENERAL - 7 )
#define SYS_ERROR_SOCKET_SHARE                  ( SYS_ERROR_SOCKET_GENERAL - 8 )
#define SYS_ERROR_SOCKET_START                  ( SYS_ERROR_SOCKET_GENERAL - 9 )
#define SYS_ERROR_SOCKET_CONNECT                ( SYS_ERROR_SOCKET_GENERAL - 10 )
#define SYS_ERROR_SOCKET_TIMEOUT                ( SYS_ERROR_SOCKET_GENERAL - 11 )
#define SYS_ERROR_SOCKET_DISCONNECTED           ( SYS_ERROR_SOCKET_GENERAL - 12 )

#ifdef __RLI_SSL_ENABLED__

/* errors returned by SSL */

#define ERROR_SSL_GENERAL                       -1300
#define ERROR_SSL_NO_SLOTS                      ( ERROR_SSL_GENERAL - 1 )
#define ERROR_SSL_CTX                           ( ERROR_SSL_GENERAL - 2 )
#define ERROR_SSL_SERVER_CERT                   ( ERROR_SSL_GENERAL - 3 )
#define ERROR_SSL_SERVER_CA_PATH                ( ERROR_SSL_GENERAL - 4 )

#endif



#endif
