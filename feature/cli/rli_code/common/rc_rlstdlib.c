/*  
 *  rc_rlstdlib.c
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
//$History: rc_rlstdlib.c $ 
 * 
 * *****************  Version 22  *****************
 * User: Pstuart      Date: 11/17/00   Time: 4:50p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * now using rc.h
 * 
 * *****************  Version 21  *****************
 * User: Pstuart      Date: 6/22/00    Time: 3:27p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Removed unused debug var
 * 
 * *****************  Version 20  *****************
 * User: Pstuart      Date: 6/07/00    Time: 5:43p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Added word option to RC_Replace
 * 
 * *****************  Version 19  *****************
 * User: Pstuart      Date: 6/02/00    Time: 10:19a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Clean up rc_replace
 * 
 * *****************  Version 18  *****************
 * User: Pstuart      Date: 5/23/00    Time: 4:22p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * RC_Replace better error codes
 * 
 * *****************  Version 17  *****************
 * User: Pstuart      Date: 5/23/00    Time: 10:19a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Added RC_Replace
 * 
 * *****************  Version 16  *****************
 * User: Pstuart      Date: 5/18/00    Time: 4:10p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Added ISALHPA()
 * 
 * *****************  Version 15  *****************
 * User: Pstuart      Date: 5/11/00    Time: 11:18a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Fix gcc warnings on STR(N)CAT
 * 
 * *****************  Version 13  *****************
 * User: Epeterson    Date: 4/25/00    Time: 1:19p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Include history and enable auto archiving feature from VSS
 * 
 * *****************  Version 12  *****************
 * User: Epeterson    Date: 4/24/00    Time: 6:21p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Enable Auto-Archive Keyword Expansion from Source Safe

$/Rapid Logic/Code Line/rli_code/common/rc_rlstdlib.c

12   Epeterson   4/24/00   6:21p   Checked in $/Rapid Logic/Code Line/rli_code/common     
11   Schew       3/22/00   7:21p   Checked in $/Rapid Logic/Code Line/rli_code/common     
     Pstuart     3/22/00  10:53a   Labeled 'PreCodeRename'                                
     David       3/07/00   8:06p   Labeled 'RC 3.01 Final Release'                        
     Builder     3/02/00   5:18p   Labeled 'RC 301 Build20000302'                         
     Builder     2/29/00   6:48p   Labeled 'Build301 20000229'                            
     Builder     2/28/00   6:35p   Labeled 'RC301 Build20000228'                          
     Builder     2/24/00   4:01p   Labeled 'RC301 Build20000224'                          
     David      12/20/99  10:55a   Labeled 'RC 3.0 Final Release'                         
     Builder    12/15/99   6:36p   Labeled 'RC30 Build19991215'                           
     Builder    12/14/99   7:08p   Labeled 'RC30 Build19991214'                           
10   Pstuart    12/14/99   6:02p   Checked in $/Rapid Logic/Code Line/rli_code/common     
9    Dreyna     12/14/99   5:42p   Checked in $/Rapid Logic/Code Line/rli_code/common     
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
8    Paul        9/17/99   2:50p   Checked in $/Rapid Logic/Code Line/rli_code/common     
7    Paul        9/17/99   2:20p   Checked in $/Rapid Logic/Code Line/rli_code/common     
6    Paul        9/17/99  11:25a   Checked in $/Rapid Logic/Code Line/rli_code/common     
     Builder     8/10/99   2:41p   Labeled 'RC 3.0 alpha 4 limited release'               
     David       8/04/99   1:24p   Labeled 'RC 2.4 release'                               
     Builder     7/29/99   5:56p   Labeled 'RCA/RCW/Mibway 2.4 Final'                     
     David       7/20/99   2:04p   Labeled 'RC 2.4 beta 6 training '                      
5    Kedron      5/27/99   8:31a   Checked in $/Rapid Logic/Code Line/rli_code/common     
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
4    Kedron     10/24/98   8:58p   Checked in $/Rapid Logic/Code Line/rli_code/common     
3    David      10/14/98   5:26p   Checked in $/Rapid Logic/Code Line/rli_code/common     
2    Mredison    9/16/98   5:12p   Checked in $/Rapid Logic/Code Line/rli_code/common     
     Leech       8/07/98   4:28p   Labeled '2.01'                                         
1    Leech       7/27/98   6:24p   Created rlstdlib.c                                     



*/
#include "rc.h"

/*-----------------------------------------------------------------------*/

#ifndef __USE_LOCAL_ANSI_LIB__

extern void *
MEMSET ( void *pDest, sbyte value, Length size )
{   
    Counter cNumBytes;
    sbyte   *pD;

    if (NULL == pDest)
        return NULL;

    pD          = ( sbyte * ) pDest;
    cNumBytes   = 0;

    while ( cNumBytes < size )
    {   
        *pD++ = value;
        cNumBytes++;
    }

    return pDest;

}



/*-----------------------------------------------------------------------*/

extern void *
MEMCPY ( void *pDest, const void *pSrc, Length size )
{   
    Counter cNumBytes;
    sbyte   *pD, *pS;

    if ((NULL == pSrc) || (NULL == pDest))
        return NULL;

    pD          = ( sbyte * ) pDest;
    pS          = ( sbyte * ) pSrc;
    cNumBytes   = 0;

    while ( cNumBytes < size )
    {
        *pD++ = *pS++;
        cNumBytes++;
    }

    return pDest;

}



/*-----------------------------------------------------------------------*/

extern sbyte4
MEMCMP ( const void *pBuf1, const void *pBuf2, Length Len )
{   
    register ubyte b1, b2;
    register sbyte *pData1, *pData2;

    pData1 = (sbyte*)pBuf1;
    pData2 = (sbyte*)pBuf2;

    do
    {
        b1 = (ubyte) *pData1++;
        b2 = (ubyte) *pData2++;

        if (0 >= (--Len))
            break;
    }
    while(b1 == b2);
    
    return(b1 - b2);
}


 
/*-----------------------------------------------------------------------*/

extern sbyte4 STRLEN (sbyte *pStr)
{
    sbyte4  cLen = 0;

    if (NULL == pStr)
        return 0;

    for (; *pStr != '\0'; pStr++, cLen++)
        ;

    return cLen;

}



/*-----------------------------------------------------------------------*/

extern sbyte *
STRCPY (sbyte *pDest, const sbyte *pSrc)
{
    register sbyte C;
    sbyte *pD = pDest;

    if ((NULL == pSrc) || (NULL == pDest))
        return NULL;
    
    do
    {
        C = *pSrc++;
        *pDest++ = C;
    }
    while (C != '\0');

    return pD;

}



/*-----------------------------------------------------------------------*/

extern sbyte *
STRNCPY (sbyte *pDest, const sbyte *pSrc, sbyte4 cLen)
{
    register sbyte C;
    sbyte *pD = pDest;

    if ((NULL == pSrc) || (NULL == pDest))
        return NULL;

    while (cLen > 0)
    {
        C = *pSrc++;
        *pDest++ = C;

        cLen--;

        if (C == '\0')
            break;
    }

    while (cLen > 0)
    {
        *pDest++ = '\0';
        cLen--;
    }

    return pD;

}


/*-----------------------------------------------------------------------*/
extern sbyte *
STRCAT(sbyte *pDest, sbyte const *pSrc)
{
    register sbyte *pOriginalString = pDest;
    register const sbyte *pAddedString = pSrc;
    char *pNewString = pDest;

    for (; *pOriginalString; ++pOriginalString);
    while ('\0' != (*pOriginalString++ = *pAddedString++));
        
    return pNewString;
}


/*-----------------------------------------------------------------------*/
extern sbyte *
STRNCAT(sbyte *pDest, const sbyte *pSrc, sbyte4 bufLen)
{
    register       sbyte *pOriginalString = pDest;
    register const sbyte *pAddedString    = pSrc;
    sbyte                *pNewString      = pDest;

    for (; *pOriginalString; ++pOriginalString);
    while ('\0' != (*pOriginalString++ = *pAddedString++))
    {
        if (0 >= --bufLen)
        {
            *pOriginalString = '\0';
            break;
        }
    }        
    return pNewString;
}

/*-----------------------------------------------------------------------*/

extern sbyte4 
STRCMP (sbyte *pStr1, sbyte *pStr2)
{
    register ubyte b1, b2;

    do
    {
        b1 = (ubyte) *pStr1++;
        b2 = (ubyte) *pStr2++;
        
        if (b1 == '\0')
            break;
    }
    while(b1 == b2);
    
    return(b1 - b2);

}



/*-----------------------------------------------------------------------*/

extern sbyte4 
STRNCMP (sbyte *pStr1, sbyte *pStr2, sbyte4 Len)
{
    register ubyte b1, b2;

    do
    {
        b1 = (ubyte) *pStr1++;
        b2 = (ubyte) *pStr2++;
        
        if (b1 == '\0')
            break;

        if ((--Len) <= 0)
            break;
    }
    while(b1 == b2);
    
    return(b1 - b2);

}



/*-----------------------------------------------------------------------*/

extern sbyte *
STRCHR ( const sbyte *pStr, sbyte4 c )
{
    if (NULL == pStr)
        return NULL;

    while ( ( *pStr != c ) && ( *pStr != '\0' ) )
        pStr++;

    if ( *pStr != '\0' )
        return (sbyte *) pStr;
    else
        return NULL;

}

/*-----------------------------------------------------------------------*/

extern sbyte *
STRRCHR ( const sbyte *pStr, sbyte4 c )
{
    const   sbyte   *pLast;
    sbyte           *pStrLenStr;
    sbyte4          lenStr;

    if (NULL == pStr)
        return NULL;
    
    pStrLenStr = (sbyte*)pStr;
    lenStr = STRLEN(pStrLenStr);
    pLast  = pStr + lenStr;

    while ( pLast >= pStr )
    {
        if ( c == *pLast )
            break;
        else
            pLast--;
    }

    if ( pLast <  pStr )
      return NULL;
    else
      return (sbyte*)pLast;
}

/*-----------------------------------------------------------------------*/

extern sbyte4
STRSPN ( sbyte *pStr, sbyte *pSampleChars )
{
    sbyte       *pTest;
    Length  numChars;

    numChars = 0;

    if ((NULL == pStr) || (NULL == pSampleChars))
        return 0; 

    while ( *pStr != '\0' )
    {
        for ( pTest = pSampleChars; *pTest != '\0'; pTest++ )
            if ( *pStr == *pTest )
                break;

        if ( *pTest == '\0' )
            return numChars;

        numChars++;
        pStr++;
    }

    return numChars;

}

/*-----------------------------------------------------------------------*/

extern sbyte4
STRCSPN ( sbyte *pStr, sbyte *pReject)
{
    Length count = 0;

    if ((NULL == pStr) || (NULL == pReject))
        return 0; 

    while ( *pStr != '\0' )
    {
        if (NULL == STRCHR(pReject, *pStr++))
            ++count;
        else
            return count;
    }
  return count;
}

/*-----------------------------------------------------------------------*/

extern sbyte *
STRPBRK ( sbyte *pStr, sbyte *pSampleStr )
{
    sbyte *pTest;

    if ((NULL == pStr) || (NULL == pSampleStr))
        return NULL; 
    
    while ( *pStr != '\0' )
    {
        pTest = STRCHR( pSampleStr, *pStr );
        if ( pTest == NULL )
            pStr++;
        else
            return pStr;
    }

    return NULL;

}



/*-----------------------------------------------------------------------*/

extern sbyte4
ATOI ( const sbyte *pStr )
{
    Boolean isNegative = FALSE, isValidNum;
    sbyte   c;
    sbyte4  val;

    if (NULL == pStr)
        return 0;

    /* check for negativity */
    if ( pStr[0] == '-' )
    {
        isNegative = TRUE;
        pStr++;
    }
    else if ( pStr[0] == '+' )
    {
        pStr++;
    }
    
    isValidNum = TRUE;
    val = 0;
    while ( *pStr != '\0' )
    {
        c = *pStr;
        if ( ISDIGIT( c ) )
            c -= '0';
        else
        {
            isValidNum = FALSE;
            break;
        }
        
        val *= 10;
        val += ( sbyte4 ) c;
        pStr++;
    }
    
    if ( isValidNum )
        val = ( isNegative ) ? -val : val;
    else
        val = 0;
    
    return val;
    
} /* ATOI */

#endif /* __USE_LOCAL_ANSI_LIB__ */

/* Allow STRCHR for ENVOYSAM.LIB */
#if  defined(__USE_LOCAL_ANSI_LIB__) && defined(__MIBWAY_TUTORIAL__)
#undef STRCHR 
extern sbyte *
STRCHR ( const sbyte *pStr, sbyte4 c )
{
    if (NULL == pStr)
        return NULL;

    while ( ( *pStr != c ) && ( *pStr != '\0' ) )
        pStr++;

    if ( *pStr != '\0' )
        return (sbyte *) pStr;
    else
        return NULL;

}
#define STRCHR      strchr
#endif



/*-----------------------------------------------------------------------*/

static sbyte ToUpper(sbyte C)
{
    if (('a' <= C) && ('z' >= C))
        return ((C - ((sbyte)'a')) + ((sbyte)'A'));

    return C;
}



/*-----------------------------------------------------------------------*/

extern sbyte4 STRICMP (sbyte *pStr1, sbyte *pStr2)
{
    register ubyte b1, b2;

    do
    {
        b1 = (ubyte) *pStr1++;
        b2 = (ubyte) *pStr2++;
        
        if (b1 == '\0')
            break;
    }
    while(ToUpper(b1) == ToUpper(b2));

    return(ToUpper(b1) - ToUpper(b2));
}



/*-----------------------------------------------------------------------*/

extern sbyte4 STRNICMP (sbyte *pStr1, sbyte *pStr2, sbyte4 Len)
{
    register ubyte b1, b2;

    do
    {
        b1 = (ubyte) *pStr1++;
        b2 = (ubyte) *pStr2++;
        
        if (b1 == '\0')
            break;

        if ((--Len) <= 0)
            break;
    }
    while(ToUpper(b1) == ToUpper(b2));

    return(ToUpper(b1) - ToUpper(b2));
}



/*-----------------------------------------------------------------------*/

extern sbyte *
STRICHR ( const sbyte *pStr, sbyte4 c )
{
    if (NULL == pStr)
        return NULL;

    while ((ToUpper(*pStr) != ToUpper((sbyte)c)) && ( *pStr != '\0' ) )
        pStr++;

    if ( *pStr != '\0' )
        return (sbyte *) pStr;
    else
        return NULL;
}



/*-----------------------------------------------------------------------*/

extern sbyte *
STRTOK_REENTRANT ( sbyte *pStr, sbyte *pSampleStr, sbyte **ppPrevStr )
{
    sbyte   *pToken;
    Length  tokenLen;

    if (( ppPrevStr == NULL ) || ( pSampleStr == NULL ))
        return NULL;

    if ( pStr == NULL )
    {
        if ( *ppPrevStr == NULL )
            return NULL;
        else
            pStr = *ppPrevStr;
    }   

    tokenLen =  STRSPN( pStr, pSampleStr );
    pStr     += tokenLen;
    if ( *pStr == '\0')
    {
        *ppPrevStr = NULL;
        return NULL;
    }

    pToken = pStr;
    pStr   = STRPBRK( pToken, pSampleStr );
    if ( pStr != NULL )
    {
        *pStr = '\0';
        *ppPrevStr = pStr + 1;
    }
    else
        *ppPrevStr = NULL;

    return pToken;
    
} /* STRTOK_REENTRANT */



/*-----------------------------------------------------------------------*/

extern Boolean
ISDIGIT ( sbyte4 c )
{
    return (('0' <= c) && (c <= '9')) ? TRUE : FALSE;

}

/*-----------------------------------------------------------------------*/

extern Boolean
ISALPHA(sbyte4 x)
{  
    return( (('a' <= x) && (x <= 'z')) || 
            (('A' <= x) && (x <= 'Z'))    );

}

/*-----------------------------------------------------------------------*/

extern RLSTATUS
RC_Replace(sbyte *pInput, sbyte *pOutput, sbyte4 outLen,
                sbyte *pFind, sbyte *pReplace, Boolean word)
{
    /* refine to use boyer-moore? */
    RLSTATUS status     = ERROR_GENERAL_NOT_FOUND;
    sbyte4   outCount   = 0;
    sbyte   *pBuf       = pInput;
    sbyte   *pLast      = pInput;
    sbyte   *pFindTmp;
    sbyte   *pStart;
    sbyte   *pOffset;
    Boolean  matched;
    Boolean  okay;

    while ('\0' != *pBuf)
    {
        /* word replace */
        okay = TRUE;
        if (word && (0 < outCount))
        {
            switch (*(pBuf - 1))
            {
            case ' ':
            case '"':
            case '\'':
                break;
            default:
                okay = FALSE;
                break;
            }
        }
        pFindTmp = pFind;
        pOffset  = pBuf;
        pStart   = pBuf;
        matched  = FALSE;
        while (okay && ('\0' != *pOffset) && (*(pFindTmp++) == *pOffset))
        {
            pOffset++;
            if ('\0' == *pFindTmp)
            {
                if (word)
                {
                    switch (*pOffset)
                    {
                    case '\0':
                    case ' ':
                    case '"':
                    case '\'':
                        break;
                    default:
                        continue;
                    }
                }

                matched = TRUE;
                status  = OK;
                pLast   = pOffset;
                break;       
            }
        }

        if (! matched)
        {
            if (outLen < ++outCount)
                return ERROR_GENERAL_BUFFER_OVERRUN;
            *(pOutput++) = *(pBuf++);
            continue;
        }

        pStart  = pReplace;
        while ('\0' != *pStart)
        {
            if (outLen < ++outCount)
                return ERROR_GENERAL_BUFFER_OVERRUN;

            *(pOutput++) = *(pStart++);
        }
        pBuf = pLast;

    }

    *pOutput = '\0';

    return status;
}

/*-----------------------------------------------------------------------*/

extern Boolean
ISALPHANUMERIC ( sbyte4 c )
{
    return (    (('a' <= c) && (c <= 'z')) ||   
                (('A' <= c) && (c <= 'Z')) || 
                (('0' <= c) && (c <= '9')) || 
                ('_' == c)                      ) ? TRUE : FALSE;

}

/*-----------------------------------------------------------------------*/

extern Boolean
ISWSPACE ( sbyte4 c )
{
    return (    ((0x09 <= c) && (c <= 0x0D)) ||
                (c == 0x20)                     ) ? TRUE : FALSE;

}


/*-----------------------------------------------------------------------*/

extern Boolean
ISSPACE ( sbyte4 c )
{
    return (c == 0x20) ? TRUE : FALSE;
}



/*-----------------------------------------------------------------------*/

extern void
UPCASE ( sbyte *pStr )
{
    if (NULL == pStr)
        return;

    while ('\0' != *pStr)
    {
        if ((*pStr >= 'a') && (*pStr <= 'z'))
        {
            *pStr -= 'a';
            *pStr += 'A';
        }

        pStr++;
    }
}



/*-----------------------------------------------------------------------*/

extern Boolean
ISLOWER(sbyte4 c)
{
    return (('a' <= c) && ('z' >= c)) ? TRUE : FALSE;
}



/*-----------------------------------------------------------------------*/

static void
DAYOFWEEK(ubyte4 DayIndex, sbyte *pBuffer)
{
    switch (DayIndex)
    {
        case 0: STRNCPY(pBuffer, "Sun ", 4);
            break;
        case 1: STRNCPY(pBuffer, "Mon ", 4);
            break;
        case 2: STRNCPY(pBuffer, "Tue ", 4);
            break;
        case 3: STRNCPY(pBuffer, "Wed ", 4);
            break;
        case 4: STRNCPY(pBuffer, "Thu ", 4);
            break;
        case 5: STRNCPY(pBuffer, "Fri ", 4);
            break;
        case 6: STRNCPY(pBuffer, "Sat ", 4);
            break;
    }

    return;
}



/*-----------------------------------------------------------------------*/

static void
MONTH(ubyte4 MonthIndex, sbyte *pBuffer)
{
    switch (MonthIndex)
    {
        case  0: STRNCPY(pBuffer, "Jan ", 4);
            break;
        case  1: STRNCPY(pBuffer, "Feb ", 4);
            break;
        case  2: STRNCPY(pBuffer, "Mar ", 4);
            break;
        case  3: STRNCPY(pBuffer, "Apr ", 4);
            break;
        case  4: STRNCPY(pBuffer, "May ", 4);
            break;
        case  5: STRNCPY(pBuffer, "Jun ", 4);
            break;
        case  6: STRNCPY(pBuffer, "Jul ", 4);
            break;
        case  7: STRNCPY(pBuffer, "Aug ", 4);
            break;
        case  8: STRNCPY(pBuffer, "Sep ", 4);
            break;
        case  9: STRNCPY(pBuffer, "Oct ", 4);
            break;
        case 10: STRNCPY(pBuffer, "Nov ", 4);
            break;
        case 11: STRNCPY(pBuffer, "Dec ", 4);
            break;
    }

    return;
}



/*-----------------------------------------------------------------------*/

static Boolean LeapYear(ubyte4 year)
{
    if (0 != (year % 4))
        return FALSE;

    if (0 != (year % 100))
        return TRUE;

    if (0 == (year % 400))
        return TRUE;

    return FALSE;
}



/*-----------------------------------------------------------------------*/

static ubyte4 GetNumDaysInYear(ubyte4 year)
{
    if (TRUE == LeapYear(year))
        return 366;

    return 365;
}



/*-----------------------------------------------------------------------*/

static ubyte4 GetNumDaysInMonth(ubyte4 month, ubyte4 year)
{
    switch (month)
    {
        case 1:  return (28 + ((TRUE == LeapYear(year)) ? 1 : 0));

        case 3:  return 30;     /* Apr */
        case 5:  return 30;     /* Jun */
        case 8:  return 30;     /* Sep */
        case 10: return 30;     /* Nov */
    }

    return 31; /* Jan, Mar, May, Jul, Aug, Oct, Dec */
}



/*-----------------------------------------------------------------------*/

extern void
ASCTIME(ubyte4 TimeInSec, sbyte *pTimeBuffer)
{
    /* output format: Sun Nov  6 08:49:37 1994 */
    ubyte4  seconds, minutes, hours, days, month, dayOfWeek = 4;
    ubyte4  NumDaysInMonth, NumDaysInYear;
    ubyte4  tempTimeInSec = TimeInSec;
    unsigned long year;

    /* calculate the easy units first */
    seconds = tempTimeInSec % 60;
    tempTimeInSec /= 60;

    minutes = tempTimeInSec % 60;
    tempTimeInSec /= 60;

    hours   = tempTimeInSec % 24;
    days    = tempTimeInSec / 24;

    /* determine year */
    for (year = 1970; (days > ((NumDaysInYear = GetNumDaysInYear(year))-1));)
    {
        days      -= NumDaysInYear;
        dayOfWeek += NumDaysInYear;
        dayOfWeek %= 7;

        year++;
    }

    /* determine month */
    for (month = 0; (days > ((NumDaysInMonth = GetNumDaysInMonth(month, year))-1));)
    {
        days      -= NumDaysInMonth;
        dayOfWeek += NumDaysInMonth;
        dayOfWeek %= 7;

        month++;
    }

    dayOfWeek += days;
    dayOfWeek %= 7;

    /* we shoud implement our own sprintf.  JB */
    DAYOFWEEK(dayOfWeek, pTimeBuffer);
    MONTH(month, &pTimeBuffer[4]);
    
    if (10 > (days + 1))
        pTimeBuffer[8] = ' ';
    else
        pTimeBuffer[8] = (sbyte)('0' + ((days + 1) / 10));

    pTimeBuffer[9]  = (sbyte)('0' + ((days + 1) % 10));
    pTimeBuffer[10] = ' ';

    pTimeBuffer[11] = (sbyte)('0' + (hours / 10));
    pTimeBuffer[12] = (sbyte)('0' + (hours % 10));
    pTimeBuffer[13] = ':';
    pTimeBuffer[14] = (sbyte)('0' + (minutes / 10));
    pTimeBuffer[15] = (sbyte)('0' + (minutes % 10));
    pTimeBuffer[16] = ':';
    pTimeBuffer[17] = (sbyte)('0' + (seconds / 10));
    pTimeBuffer[18] = (sbyte)('0' + (seconds % 10));
    pTimeBuffer[19] = ' ';

    CONVERT_ToStr(&year, &pTimeBuffer[20], kDTulong);
}

