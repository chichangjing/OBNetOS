/*
 *  rc_base64.h
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

$History: rc_base64.h $
 * 
 * *****************  Version 5  *****************
 * User: Pstuart      Date: 12/11/00   Time: 11:30a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added c++ wrappers for prototypes
 * 
 * *****************  Version 4  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Include history and enable auto archiving feature from VSS



$/Rapid Logic/Code Line/rli_code/include/rc_base64.h

3    Schew       3/22/00   7:32p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Pstuart     3/22/00  10:53a   Labeled 'PreCodeRename'                                
     David       3/07/00   8:06p   Labeled 'RC 3.01 Final Release'                        
     Builder     3/02/00   5:18p   Labeled 'RC 301 Build20000302'                         
     Builder     2/29/00   6:48p   Labeled 'Build301 20000229'                            
     Builder     2/28/00   6:35p   Labeled 'RC301 Build20000228'                          
     Builder     2/24/00   4:01p   Labeled 'RC301 Build20000224'                          
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
2    Kedron      5/27/99   8:54a   Checked in $/Rapid Logic/Code Line/rli_code/include    
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
     David       2/09/99  11:30a   Labeled 'WC2.3 beta5 - 2-9-99'                         
     David       2/08/99   5:28p   Labeled 'JC2.3 beta5 - 2-8-99'                         
     David       2/05/99   6:55p   Labeled 'WC2.3 beta4 - 2-5-99'                         
     David       2/05/99   6:55p   Labeled 'JC2.3 beta4 - 2-5-99'                         
     David       2/03/99   7:23p   Labeled 'WC 2.3 beta3 - 2-3-99'                        
     David       2/03/99   7:23p   Labeled 'JC2.3 beta3 - 2-3-99'                         
     David       2/01/99   7:00p   Labeled 'JC2.3 beta 2 - 2/1/99'                        
     David       1/29/99   6:13p   Labeled 'JC2.3beta1'                                   
     David       1/15/99   3:16p   Labeled 'jc 2.2 rc4'                                   
     Henry       1/08/99   4:20p   Labeled 'jc 2.2 rc3'                                   
     Henry      12/18/98   5:12p   Labeled 'jc 2.2 rc2a'                                  
     Henry      12/18/98   4:34p   Labeled 'jc 2.2 rc2'                                   
     David      12/10/98   6:29p   Labeled 'JC 12/10'                                     
     Henry      12/07/98   5:27p   Labeled 'JC 12/7'                                      
     Henry      12/04/98   3:35p   Labeled 'Build 12/4'                                   
     Leech       8/07/98   4:28p   Labeled '2.01'                                         
1    Leech       7/27/98   6:24p   Created base64.h                                       





*/



#ifndef __BASE64_HEADER__
#define __BASE64_HEADER__

/* prototypes */

#ifdef __cplusplus
extern "C" {
#endif

Length BASE64_EncodeBytesRequired   (Length NumBytes);
Length BASE64_DecodeBytesRequired   (Length NumBytes);

RLSTATUS BASE64_Decode    (ubyte *pSource, ubyte *pDest, Length NumBytes, Length *pOutputLen);
RLSTATUS BASE64_Encode    (ubyte *pSource, ubyte *pDest, Length NumBytes, Length *pOutputLen);

#ifdef __cplusplus
}
#endif


#endif

