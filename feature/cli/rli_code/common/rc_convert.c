/*  
 *  rc_convert.c
 *
 *  This is a part of the OpenControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
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

$History: rc_convert.c $ 
 * 
 * *****************  Version 65  *****************
 * User: Pstuart      Date: 7/24/01    Time: 1:02p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * ip address conversion needs ntoh/hton conversion
 * 
 * *****************  Version 64  *****************
 * User: Pstuart      Date: 6/28/01    Time: 2:08p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * check for valid end boundary of mac addr 
 * 
 * *****************  Version 63  *****************
 * User: Pstuart      Date: 6/27/01    Time: 4:01p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * string can be empty but not null
 * 
 * *****************  Version 62  *****************
 * User: Pstuart      Date: 6/22/01    Time: 10:32a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * add null check
 * 
 * *****************  Version 61  *****************
 * User: Pstuart      Date: 6/19/01    Time: 5:32p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * fix access using max level
 * 
 * *****************  Version 60  *****************
 * User: Pstuart      Date: 5/30/01    Time: 3:29p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * fix number range check
 * 
 * *****************  Version 59  *****************
 * User: Pstuart      Date: 5/30/01    Time: 10:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * 
 * *****************  Version 58  *****************
 * User: Pstuart      Date: 5/30/01    Time: 10:46a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * 
 * *****************  Version 57  *****************
 * User: Pstuart      Date: 5/29/01    Time: 10:24a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * fix ambiguity problem for access enumerators
 * 
 * *****************  Version 56  *****************
 * User: Pstuart      Date: 5/24/01    Time: 3:17p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * fix string to enum
 * 
 * *****************  Version 55  *****************
 * User: Pstuart      Date: 5/14/01    Time: 5:21p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * return error if more than one partial match for enum name
 * 
 * *****************  Version 54  *****************
 * User: Pstuart      Date: 4/04/01    Time: 5:03p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * remove leading zeros from comparison in  CONVERT_StrNumCmp
 * 
 * *****************  Version 53  *****************
 * User: Pstuart      Date: 3/06/01    Time: 12:39p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * fix gcc warning
 * 
 * *****************  Version 52  *****************
 * User: Pstuart      Date: 1/11/01    Time: 2:58p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * fix gcc warning
 * 
 * *****************  Version 51  *****************
 * User: Pstuart      Date: 1/08/01    Time: 1:42p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Add check for buffer overflow
 * 
 * *****************  Version 50  *****************
 * User: Pstuart      Date: 1/05/01    Time: 9:58a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * remove gcc warning for unused var
 * 
 * *****************  Version 49  *****************
 * User: Pstuart      Date: 1/05/01    Time: 9:53a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * add optional __ALLOW_ENUM_ORDINALS__ to allow integers to be used for
 * enums 
 * 
 * *****************  Version 48  *****************
 * User: Pstuart      Date: 12/20/00   Time: 3:37p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * 
 * *****************  Version 47  *****************
 * User: Pstuart      Date: 12/20/00   Time: 3:37p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Fix null check
 * 
 * *****************  Version 46  *****************
 * User: Pstuart      Date: 11/17/00   Time: 6:58p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * expanded CONVERT_Validate params, removed custom_type ifdefs --
 * shouldn't be needed anymore
 * 
 * *****************  Version 45  *****************
 * User: Pstuart      Date: 10/13/00   Time: 10:24a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Add null check
 * 
 * *****************  Version 44  *****************
 * User: Pstuart      Date: 10/02/00   Time: 11:10a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * CONVERT_StrToList now returns not found if member not found
 * 
 * *****************  Version 43  *****************
 * User: Pstuart      Date: 9/12/00    Time: 3:18p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * standardized var names in CONVERT_Validate and added null check,
 * enabled CONVERT_StrToEnum to accept ints as enums
 * 
 * *****************  Version 42  *****************
 * User: Kedron       Date: 9/06/00    Time: 5:03p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * removed NTOH4 from CONVERT_StrToIP() and HTON4 from CONVERT_IPToStr()
 * 
 * *****************  Version 39  *****************
 * User: Pstuart      Date: 7/31/00    Time: 3:30p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Fix compiler warning
 * 
 * *****************  Version 38  *****************
 * User: James        Date: 7/05/00    Time: 11:07a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Fixed size bug.
 * 
 * *****************  Version 37  *****************
 * User: Pstuart      Date: 6/22/00    Time: 3:26p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Cleaned up hex conversion, number check
 * 
 * *****************  Version 36  *****************
 * User: Pstuart      Date: 6/19/00    Time: 3:59p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Added hex conversion
 * 
 * *****************  Version 35  *****************
 * User: Pstuart      Date: 6/13/00    Time: 11:05a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Added numeric string check for CONVERT_Validate
 * 
 * *****************  Version 34  *****************
 * User: Dreyna       Date: 6/02/00    Time: 5:18p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Changed displayed format of of Mac Addresses (xx.xx... to xx:xx:...)
 * 
 * *****************  Version 33  *****************
 * User: Pstuart      Date: 6/01/00    Time: 2:41p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * StrToIP wasn't checking for extra data, fixed Hex2Dec, StrToMacAddr
 * 
 * *****************  Version 32  *****************
 * User: Pstuart      Date: 5/31/00    Time: 4:53p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Fix DataType name funcs
 * 
 * *****************  Version 31  *****************
 * User: Pstuart      Date: 5/31/00    Time: 2:02p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Fixed CONVERT_StrToMacAddr
 * 
 * *****************  Version 30  *****************
 * User: Pstuart      Date: 5/31/00    Time: 10:33a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Added convert for MacAddr, range error for validation
 * 
 * *****************  Version 29  *****************
 * User: Dreyna       Date: 5/25/00    Time: 4:57p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Fixed default string validator for string legth keyed by "N="  instead
 * of "L="
 * 
 * *****************  Version 28  *****************
 * User: Pstuart      Date: 5/23/00    Time: 3:16p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Fixed buffer overrun in CONVERT_EnumStr
 * 
 * *****************  Version 27  *****************
 * User: Pstuart      Date: 5/18/00    Time: 4:10p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Fixed extended string validation
 * 
 * *****************  Version 26  *****************
 * User: Pstuart      Date: 5/11/00    Time: 11:17a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Fix gcc error/warnings
 * 
 * *****************  Version 25  *****************
 * User: Pstuart      Date: 5/10/00    Time: 9:59a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Changed c++ comments to c-style
 * 
 * *****************  Version 24  *****************
 * User: Dreyna       Date: 5/09/00    Time: 6:06p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Allow dump of all list stings values for RCC_TASK.
 * 
 * *****************  Version 23  *****************
 * User: Pstuart      Date: 5/09/00    Time: 5:16p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * changed DTName refs to use int indices
 * 
 * *****************  Version 22  *****************
 * User: Dreyna       Date: 5/09/00    Time: 5:02p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Update access <-> string conversion
 * 
 * *****************  Version 21  *****************
 * User: Dreyna       Date: 5/05/00    Time: 3:00p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Added conversions to and from kDTaccess.
 * 
 * *****************  Version 20  *****************
 * User: Pstuart      Date: 5/05/00    Time: 2:17p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Added CONVERT_GetDTProtoType
 * 
 * *****************  Version 19  *****************
 * User: Pstuart      Date: 5/04/00    Time: 11:00a
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Added prototype info for DataTypes, cleaned up CONVERT_GetDTName
 * 
 * *****************  Version 18  *****************
 * User: Dreyna       Date: 5/03/00    Time: 6:16p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Added call for any Custom Validator.
 * 
 * *****************  Version 17  *****************
 * User: Dreyna       Date: 5/01/00    Time: 3:18p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Made Default value part of ParamInfo Record. Allowed substitution of
 * base type validation string if ParamInfo Valid String is NULL (space
 * saving option).
 * 
 * *****************  Version 16  *****************
 * User: Dreyna       Date: 4/28/00    Time: 5:05p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Incremental update of custom type functions.
 * 
 * *****************  Version 14  *****************
 * User: Epeterson    Date: 4/25/00    Time: 1:19p
 * Updated in $/Rapid Logic/Code Line/rli_code/common
 * Include history and enable auto archiving feature from VSS

$/Rapid Logic/Code Line/rli_code/common/rc_convert.c

13   Pstuart     4/25/00  11:27a   Checked in $/Rapid Logic/Code Line/rli_code/common     
12   Epeterson   4/24/00   6:21p   Checked in $/Rapid Logic/Code Line/rli_code/common     
11   Dreyna      4/19/00   5:32p   Checked in $/Rapid Logic/Code Line/rli_code/common     
10   Dreyna      4/17/00   5:56p   Checked in $/Rapid Logic/Code Line/rli_code/common     
9    Pstuart     4/11/00   3:37p   Checked in $/Rapid Logic/Code Line/rli_code/common     
8    Pstuart     4/11/00   2:50p   Checked in $/Rapid Logic/Code Line/rli_code/common     
7    Schew       3/22/00   7:21p   Checked in $/Rapid Logic/Code Line/rli_code/common     
     Pstuart     3/22/00  10:53a   Labeled 'PreCodeRename'                                
6    Dreyna      3/16/00   5:12p   Checked in $/Rapid Logic/Code Line/rli_code/common     
     David       3/07/00   8:06p   Labeled 'RC 3.01 Final Release'                        
5    Pstuart     3/03/00  10:18a   Checked in $/Rapid Logic/Code Line/rli_code/common     
     Builder     3/02/00   5:18p   Labeled 'RC 301 Build20000302'                         
     Builder     2/29/00   6:48p   Labeled 'Build301 20000229'                            
     Builder     2/28/00   6:35p   Labeled 'RC301 Build20000228'                          
     Builder     2/24/00   4:01p   Labeled 'RC301 Build20000224'                          
4    Pstuart     2/17/00   1:58p   Checked in $/Rapid Logic/Code Line/rli_code/common     
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
3    Kedron      5/27/99   8:31a   Checked in $/Rapid Logic/Code Line/rli_code/common     
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
2    Kedron      8/11/98   3:56p   Checked in $/Rapid Logic/Code Line/rli_code/common     
     Leech       8/07/98   4:28p   Labeled '2.01'                                         
1    Leech       7/27/98   6:24p   Created convert.c                                      




*/

#include "rc.h"
#include <string.h>


#define kRLI_VALID_NUMERATOR "N="
#define kRLI_VALID_NUMERATOR_LEN  (sizeof(kRLI_VALID_NUMERATOR) - 1)
#define kSTRING_BUF_SIZE      10

#ifdef __RLI_ACCESS_LEVEL_MASK__
#define kMAX_ACCESS_LEVEL   __RLI_ACCESS_LEVEL_MASK__
#else
#define kMAX_ACCESS_LEVEL   0
#endif

static DTNames DataTypeNames[] =
{
    {kDTinvalid,    kDTNinvalid,    NULL},
    {kDTmacro,      kDTNmacro,      NULL},
    {kDTstring,     kDTNstring,     NULL},
    {kDTipaddress,  kDTNipaddress,  "A.B.C.D"},
    {kDTinteger,    kDTNinteger,    NULL},
    {kDTchar,       kDTNchar,       NULL},
    {kDTshort,      kDTNshort,      NULL},
    {kDTlong,       kDTNlong,       NULL},
    {kDTuinteger,   kDTNuinteger,   NULL},
    {kDTuchar,      kDTNuchar,      NULL},
    {kDTushort,     kDTNushort,     NULL},
    {kDTulong,      kDTNulong,      NULL},
    {kDTmacaddr,    kDTNmacaddr,    "aa:bb:cc:dd:ee:ff"},
    {kDTenum,       kDTNenum,       NULL},
    {kDTlist,       kDTNlist,       NULL},
    {kDTaccess,     kDTNaccess,     NULL},
    {kDTabsolute,   kDTNabsolute,   NULL},
    {kDTnull,       kDTNnull,       NULL},
    {kDTvoid,       kDTNvoid,       NULL}
};


typedef enum StringType
{
    kSTRING_Invalid,
    kSTRING_Alpha,
    kSTRING_AlphaNumeric,
    kSTRING_Numeric,
    kSTRING_Any
} StringType;


typedef struct StringInfo
{
    StringType  stringType;
    sbyte      *stringCode;
} StringInfo;


static StringInfo validStrings[] =
{
    {kSTRING_Alpha,         "AP"},
    {kSTRING_AlphaNumeric,  "AN"},
    {kSTRING_Numeric,       "NU"},
    {kSTRING_Any,           "AL"}
};

/*-----------------------------------------------------------------------*/

extern sbyte * CONVERT_GetDTName(DataType type)
{
    sbyte4   index;
    DTNames *dType = DataTypeNames;

    for (index = 0; index < ARRAY_SIZE(DataTypeNames); index ++, dType++)
    {
        if (type == dType->type)
            return dType->label;
    }
    return NULL;
}

/*-----------------------------------------------------------------------*/

extern sbyte * CONVERT_GetDTProtoType(DataType type)
{
    sbyte4   index;
    DTNames *dType = DataTypeNames;

    for (index = 0; index < ARRAY_SIZE(DataTypeNames); index ++, dType++)
    {
        if (type != dType->type)
            continue;

        if (NULL != dType->prototype)
            return dType->prototype;
        else
            return dType->label;
    }
    return NULL;

}

/*-----------------------------------------------------------------------*/

/* Get number of valid enums */
extern int CONVERT_GetEnumCount(DTTypeInfo *pTypeInfo)
{
    sbyte  *tptr;
    sbyte   str[kSTRING_BUF_SIZE + 1];
	sbyte4  i;
    sbyte4  j;

	if (NULL == (tptr = strstr(pTypeInfo->pValidateStr, kRLI_VALID_NUMERATOR))) 
        return -1;

    tptr += kRLI_VALID_NUMERATOR_LEN;
	for (i=0, j=0; ISDIGIT(tptr[i]) && (i < kSTRING_BUF_SIZE); i++) 
        str[j++] = tptr[i];

    str[j] = '\0'; 
    return ATOI(str);
}

/*-----------------------------------------------------------------------*/

extern void 
RC_CONVERT_Suffix(DTTypeInfo *pTypeInfo, sbyte *what, sbyte **start, sbyte4 *length)
{
    sbyte  *pBuf;

    *length = 0;

    if ((NULL == pTypeInfo) || (NULL == pTypeInfo->pValidateStr))
        return;

	if (NULL == (*start = strstr(pTypeInfo->pValidateStr, what)))
        return;

    *start += STRLEN(what);

    for (pBuf = *start; ('\0' != *pBuf) && (' ' != *pBuf); pBuf++)
        (*length)++;
}

/*-----------------------------------------------------------------------*/

#define SignTest(MinValue, MaxValue, Signed, Value, NextDigit, DataType)                        \
{                                                                                               \
    if (FALSE == Signed)                                                                        \
    {                                                                                           \
        if ((unsigned DataType)Value > ((unsigned DataType)MaxValue / 10))                      \
            return ERROR_CONVERSION_OVERFLOW;                                                   \
                                                                                                \
        if ((unsigned DataType)Value == ((unsigned DataType)MaxValue / 10))                     \
            if ((unsigned DataType)DIGIT_TO_CHAR(NextDigit) > ((unsigned DataType)MaxValue % 10)) \
                return ERROR_CONVERSION_OVERFLOW;                                               \
    }                                                                                           \
    else                                                                                        \
    {                                                                                           \
        if ((unsigned DataType)Value > ((unsigned DataType)MinValue / 10))                      \
            return ERROR_CONVERSION_UNDERFLOW;                                                  \
                                                                                                \
        if ((unsigned DataType)Value == ((unsigned DataType)MinValue / 10))                     \
            if ((unsigned DataType)DIGIT_TO_CHAR(NextDigit) > ((unsigned DataType)MinValue % 10)) \
                return ERROR_CONVERSION_UNDERFLOW;                                              \
    }                                                                                           \
}

/*-----------------------------------------------------------------------*/

#define UnsignTest(MaxValue, Value, NextDigit, DataType)                                    \
{                                                                                           \
    if ((unsigned DataType)Value > ((unsigned DataType)MaxValue / 10))                      \
        return ERROR_CONVERSION_OVERFLOW;                                                   \
                                                                                            \
    if ((unsigned DataType)Value == ((unsigned DataType)MaxValue / 10))                     \
        if (DIGIT_TO_CHAR(NextDigit) > ((unsigned DataType)MaxValue % 10))                    \
            return ERROR_CONVERSION_OVERFLOW;                                               \
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_StrToByte(sbyte *pString, void *pChar, Boolean convert)
{
    signed char     Char = 0;
    sbyte           Digit;
    Boolean         Signed = FALSE;

    if (NULL_STRING(pString))
        return ERROR_GENERAL_NO_DATA;

    if ('-' == *pString)
    {
        Signed = TRUE;
        pString++;
    }
    else if ('+' == *pString)
        pString++;

    if ('\0' == *pString)
        return ERROR_GENERAL_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        SignTest(kMIN_CHAR, kMAX_CHAR, Signed, Char, Digit, char);

        Char *= 10;
        Char = (char)(Char + DIGIT_TO_CHAR(Digit));

        pString++;
    }

    if (convert)
        *((char *) pChar) = (char) ((Signed == TRUE) ? ((char)((~Char)+1)) : ((char)Char));

    return OK;

}   /* CONVERT_StrToByte */

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_StrToUbyte(sbyte *pString, void *pUchar, Boolean convert)
{
    unsigned char       Uchar = 0;
    sbyte               Digit;

    if (NULL_STRING(pString))
        return ERROR_GENERAL_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        UnsignTest(kMAX_UCHAR, Uchar, Digit, char);

        Uchar *= 10;
        Uchar = (unsigned char)(Uchar + DIGIT_TO_CHAR(Digit));

        pString++;
    }

    if (convert)
    {
        if (NULL == pUchar)
            return ERROR_GENERAL_NULL_POINTER;

        *((unsigned char *) pUchar) = Uchar;        
    }

    return OK;
}

/*-----------------------------------------------------------------------*/

/* sample conversion code */

static RLSTATUS CONVERT_StrToIP(sbyte *pString, void *pIPaddr, Boolean convert)
{
    unsigned char  Uchar = 0;
    sbyte          Digit;
    Counter        IPdigit;
    ubyte4         IPaddr = 0;
    Boolean        hasOctet;

    if (NULL_STRING(pString))
        return ERROR_GENERAL_NO_DATA;

    for (IPdigit = 0; IPdigit < 4; IPdigit++)
    {
        hasOctet = FALSE;
        while (('\0' != (Digit = *pString)) && ('.' != *pString))
        {
            if (FALSE == ISDIGIT(Digit))
                return ERROR_CONVERSION_INCORRECT_TYPE;

            /* check for overflow.  size containment problems... */
            UnsignTest(kMAX_UCHAR, Uchar, Digit, char);

            Uchar *= 10;
            Uchar += (unsigned char) DIGIT_TO_CHAR(Digit);

            pString++;
            hasOctet = TRUE;
        }
        if (FALSE == hasOctet)
            return ERROR_CONVERSION_INCORRECT_TYPE;

        IPaddr <<= 8;
        IPaddr  += Uchar;
        Uchar    = 0;

        if ('\0' == Digit)
        {
            if (3 != IPdigit)
                return ERROR_CONVERSION_INCORRECT_TYPE;
            else break;
        }

        /* extra garbage characters? */
        if ((3 == IPdigit) && ('\0' != Digit))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        pString++;
    }

/* endian conversion may be necessary here */

    if (convert)
    {
        if (NULL == pIPaddr)
            return ERROR_GENERAL_NULL_POINTER;

        *((ubyte4 *)pIPaddr) = HTON4(IPaddr);
    }

    return OK;

}   /* CONVERT_StrToIP */

/*-----------------------------------------------------------------------*/
/* sample conversion code */

static sbyte CONVERT_Hex2Dec(sbyte HexDigit)
{
    if (('0' <= HexDigit) && (HexDigit <= '9'))
        return (HexDigit - '0');

    if (('a' <= HexDigit) && (HexDigit <= 'f'))
        return (HexDigit - 'a' + 10);

    if (('A' <= HexDigit) && (HexDigit <= 'F'))
        return (HexDigit - 'A' + 10);

    /* illegal digit */
    return -1;
}

/*-----------------------------------------------------------------------*/

static sbyte CONVERT_Dec2Hex(sbyte DecDigit) 
{
    sbyte4 digit = (int) DecDigit;

    if ((0 <= digit) && (digit <= 9))
        return (digit + '0');

    if ((10 <= digit) && (digit <= 15))
        return (digit - 10 + 'a');

    /* illegal digit */
    return ' ';
}

/*-----------------------------------------------------------------------*/
/*
extern void CONVERT_HexString(sbyte4 *hex, sbyte *pBuf)
{
    sbyte   t     = 0;
    Boolean zeros = TRUE;

    *(pBuf)++ = '0';
    *(pBuf)++ = 'x';

    while (*hex)
    {
        t = (0xf0000000 & *hex) >> 28;
        if (! zeros || (0 < t))
        {
            *pBuf = CONVERT_Dec2Hex(t);
            pBuf++;
            zeros = FALSE;
        }
        *hex = *hex << 4;
    }
    if (zeros)
        *(pBuf++) = '0';

    *pBuf = kCHAR_NULL;
}
*/

/*-----------------------------------------------------------------------*/

/* Is string a valid numeric format? */

static Boolean CONVERT_IsNumber(sbyte *pBuf)
{
    if (NULL_STRING(pBuf))
        return FALSE;

    if (('-' == *pBuf) || ('+' == *pBuf))
        pBuf++;

    if (! ISDIGIT(*pBuf))
        return FALSE;

    for ( ; '\0' != *pBuf ; pBuf++)
    {
        if (! ISDIGIT(*pBuf))
            return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------*/

#define MACADDR_OCTETS  6
static RLSTATUS CONVERT_StrToMacAddr(sbyte *pString, void *pMacAddr, Boolean convert)
{
    sbyte2         hiDigit;
    sbyte2         loDigit;
    sbyte2         tempDigit;
    Counter        index;
    sbyte          MacAddr[MACADDR_OCTETS];

    if (NULL_STRING(pString))
        return ERROR_GENERAL_NO_DATA;

    for (index = 0; index < MACADDR_OCTETS; index++)
    {
        hiDigit = CONVERT_Hex2Dec(*(pString++));

        if (('\0' != *pString) && (':' != *pString))
            loDigit = CONVERT_Hex2Dec(*(pString++));
        else
        {
            loDigit = hiDigit;
            hiDigit = 0;
        }
        
        if ((0 > hiDigit) || (0 > loDigit))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        tempDigit = (hiDigit << 4) + loDigit;
        if ((0 > tempDigit) || (tempDigit > 255))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        if ((index < (MACADDR_OCTETS - 1)) && (':' != *pString))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        if (index < MACADDR_OCTETS - 1)
        {
            if (':' != *pString)
                return ERROR_GENERAL_ILLEGAL_VALUE;
        }
        else
        {
            if (0 != *pString)
                return ERROR_GENERAL_ILLEGAL_VALUE;
        }

        pString++;

        MacAddr[index] = (sbyte) tempDigit;
    }

    /* endian conversion may be necessary here */

    if (convert)
    {
        if (NULL == pMacAddr)
            return ERROR_GENERAL_NULL_POINTER;

        MEMCPY(pMacAddr, MacAddr, MACADDR_OCTETS);
    }

    return OK;

}   /* CONVERT_StrToMacAddr */
 
/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_StrToInt(sbyte *pString, void *pInt, Boolean convert)
{
    unsigned int    Int = 0;
    sbyte   Digit;
    Boolean Signed = FALSE;

    if (NULL_STRING(pString))
        return ERROR_GENERAL_NO_DATA;

    if ('-' == *pString)
    {
        Signed = TRUE;
        pString++;
    }
    else if ('+' == *pString)
        pString++;

    if ('\0' == *pString)
        return ERROR_GENERAL_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        SignTest(kMIN_INT, kMAX_INT, Signed, Int, Digit, int);

        Int *= 10;
        Int += DIGIT_TO_CHAR(Digit);

        pString++;
    }

    if (convert)
    {
        if (NULL == pInt)
            return ERROR_GENERAL_NULL_POINTER;

        *((int *) pInt) = (Signed == TRUE) ? (int)((~Int)+1) : (int)Int;    
    }

    return OK;

}   /* CONVERT_StrToInt */

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_StrToUnsigned(sbyte *pString, void *pUnsigned, Boolean convert)
{
    unsigned int    Unsigned = 0;
    sbyte           Digit;

    if (NULL_STRING(pString))
        return ERROR_GENERAL_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        UnsignTest(kMAX_UNSIGNED, Unsigned, Digit, int);

        Unsigned *= 10;
        Unsigned += DIGIT_TO_CHAR(Digit);

        pString++;
    }

    if (convert)
    {
        if (NULL == pUnsigned)
            return ERROR_GENERAL_NULL_POINTER;

        *((unsigned int *) pUnsigned) = Unsigned;   
    }

    return OK;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_StrToShort(sbyte *pString, void *pShort, Boolean convert)
{
    unsigned short  Short = 0;
    sbyte           Digit;
    Boolean         Signed = FALSE;

    if (NULL_STRING(pString))
        return ERROR_GENERAL_NO_DATA;

    if ('-' == *pString)
    {
        Signed = TRUE;
        pString++;
    }
    else if ('+' == *pString)
        pString++;

    if ('\0' == *pString)
        return ERROR_GENERAL_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        SignTest(kMIN_SHORT, kMAX_SHORT, Signed, Short, Digit, short);

        Short *= 10;
        Short += (short) DIGIT_TO_CHAR(Digit);

        pString++;
    }

    if (convert)
    {
        if (NULL == pShort)
            return ERROR_GENERAL_NULL_POINTER;

        *((short *) pShort) = (short) ((Signed == TRUE) ? (~Short + 1) : Short);
    }

    return OK;

}   /* CONVERT_StrToShort */

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_StrToUshort(sbyte *pString, void *pUshort, Boolean convert)
{
    unsigned short      Ushort = 0;
    sbyte               Digit;

    if (NULL_STRING(pString))
        return ERROR_GENERAL_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        UnsignTest(kMAX_USHORT, Ushort, Digit, short);

        Ushort *= 10;
        Ushort = (unsigned short)(Ushort + DIGIT_TO_CHAR(Digit));

        pString++;
    }

    if (convert)
    {
        if (NULL == pUshort) 
            return ERROR_GENERAL_NULL_POINTER;

        *((unsigned short *) pUshort) = Ushort;     
    }

    return OK;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_StrToLong(sbyte *pString, void *pLong, Boolean convert)
{
    unsigned long   Long = 0;
    sbyte           Digit;
    Boolean         Signed = FALSE;

    if (NULL_STRING(pString))
        return ERROR_GENERAL_NO_DATA;

    if ('-' == *pString)
    {
        Signed = TRUE;
        pString++;
    }
    else if ('+' == *pString)
        pString++;

    if ('\0' == *pString)
        return ERROR_GENERAL_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        SignTest(kMIN_LONG, kMAX_LONG, Signed, Long, Digit, long);

        Long *= 10;
        Long += DIGIT_TO_CHAR(Digit);

        pString++;
    }

    if (convert)
    {
        if (NULL == pLong)
            return ERROR_GENERAL_NULL_POINTER;

        *((long *) pLong) = (Signed == TRUE) ? (long)((~Long)+1) : (long)Long;  
    }

    return OK;

}   /* CONVERT_StrToLong */

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_StrToUlong(sbyte *pString, void *pUlong, Boolean convert)
{
    unsigned long       Ulong = 0;
    sbyte               Digit;

    if (NULL_STRING(pString))
        return ERROR_GENERAL_NO_DATA;

    while ('\0' != (Digit = *pString))
    {
        if (FALSE == ISDIGIT(Digit))
            return ERROR_CONVERSION_INCORRECT_TYPE;

        /* check for overflow.  size containment problems... */
        UnsignTest(kMAX_ULONG, Ulong, Digit, long);

        Ulong *= 10;
        Ulong += DIGIT_TO_CHAR(Digit);

        pString++;
    }

    if (convert)
    {
        if (NULL == pUlong)
            return ERROR_GENERAL_NULL_POINTER;

        *((unsigned long *) pUlong) = Ulong;        
    }

    return OK;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_StrToEnum(sbyte *pString, EnumType *pEnum, DTTypeInfo *pTypeInfo)
{   
	sbyte4       i;
    sbyte4       count;
    sbyte4       dLen;
    DTEnumInfo  *pEnumTable;
    EnumType     matchEnum  = 0;
    sbyte4       matchCount = 0;
#ifdef __ALLOW_ENUM_ORDINALS__
    EnumType     value;
#endif

	*pEnum = 0;

	if ((NULL == pTypeInfo) || (NULL_STRING(pString)))
        return ERROR_GENERAL_NOT_FOUND;

	/* Get number of valid enums */
    count = CONVERT_GetEnumCount(pTypeInfo);

    /* find WHOLE string is in enum list */
    pEnumTable = pTypeInfo->pEnumTable;
	for (i = 0; i < count; i++, pEnumTable++)
	{
		if (0 == STRCMP(pString, pEnumTable->EnumString)) 
		{
			*pEnum = pEnumTable->EnumValue;
			return(OK);
		}
	}

    /* find PARTIAL string is in enum list */
    dLen       = STRLEN(pString);
    pEnumTable = pTypeInfo->pEnumTable;
	for (i = 0; i < count; i++, pEnumTable++)
	{
		if (0 == STRNCMP(pString, pEnumTable->EnumString, dLen)) 
		{
			 matchEnum = pEnumTable->EnumValue;
             matchCount++;
		}
	}

    if (1 == matchCount)
    {
        *pEnum = matchEnum;
        return OK;
    }

    if (1 < matchCount)
        return ERROR_GENERAL_DATA_AMBIG;

    /* is the enum actually the numeric value itself? */

#ifdef __ALLOW_ENUM_ORDINALS__
    value = (EnumType) ATOI(pString);
    if (0 == value)
    	return ERROR_GENERAL_NOT_FOUND;

    pEnumTable = pTypeInfo->pEnumTable;
    for (i = 0; i < count; i++, pEnumTable++)
	{
	    if (pEnumTable->EnumValue != value) 
            continue;

        *pEnum = value;
        return OK;
    }
#endif /* __ALLOW_ENUM_ORDINALS__ */

	return ERROR_GENERAL_NOT_FOUND;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_StrToList(sbyte *pString, EnumType *pEnum, DTTypeInfo *pTypeInfo)
{
    sbyte	    *tptr, *uptr, ch;
	int          i, j, count;
    DTEnumInfo  *pEnumTable;

	*pEnum = 0;

	if ((NULL == pTypeInfo) || (NULL == pString))
        return ERROR_GENERAL_NOT_FOUND;
	
	/* get separater */
	if (NULL== (tptr = strstr(pTypeInfo->pValidateStr,"D=")))
        return ERROR_GENERAL_NOT_FOUND;

    ch   = tptr[2];

	/* Get number of valid enums */
    count = CONVERT_GetEnumCount(pTypeInfo);

	tptr = pString;
	while (STRLEN(tptr))
	{	
		j = STRLEN(tptr);
		if (NULL != (uptr = STRCHR(tptr,ch)))
		{	
            j = ((long)uptr) - ((long) tptr);
		}
		
		/* find it in the list */
        pEnumTable = pTypeInfo->pEnumTable;
		for (i = 0; i < count; i++, pEnumTable++)
		{
			if (0 == STRNCMP(tptr, pEnumTable->EnumString, j)) 
			{
				*pEnum |= pEnumTable->EnumValue;
				break;
			}
		}
		if (i == count)
            return(ERROR_GENERAL_NOT_FOUND);

		/* advance tptr */
		tptr = &tptr[j]; 
        if (NULL != uptr) 
            tptr++;
	}

	return OK;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_StrToAccess(sbyte *pString, Access *pAccess, 
                                    DTTypeInfo *pTypeInfo)
{
    RLSTATUS    status;
    sbyte	   *pTemp;
    sbyte      *pNext;
    sbyte       delimiter;
	int         i;
    int         enumLen;
    int         enumValue;
    int         count;
    DTEnumInfo *pEnumTable;
	char		numstr[20];
	sbyte4		tmpAccess;
    Access      tmpAccess1;
    Boolean     match;

	*pAccess = 0;

	if ((NULL == pTypeInfo) || (NULL == pString))
        return ERROR_GENERAL_NOT_FOUND;
	
#ifndef	__RLI_ACCESS_LEVEL_MASK__
	return(CONVERT_StrTo(pString,pAccess,kDTinteger));
#endif

	/* get separater */
	if (NULL == (pTemp = strstr(pTypeInfo->pValidateStr,"D=")))
        return ERROR_GENERAL_NOT_FOUND;

    delimiter = pTemp[2];
    count     = CONVERT_GetEnumCount(pTypeInfo);
	pTemp     = pString;

	while (0 != *pTemp)
	{	
		enumLen = STRLEN(pTemp);
		if (NULL != (pNext = STRCHR(pTemp, delimiter)))
            enumLen = (int) (pNext - pTemp);
		
		if (ISDIGIT(*pTemp))
		{
			strncpy(numstr, pTemp, enumLen);
			numstr[enumLen] = '\0';
			if (OK != (status = CONVERT_StrTo(numstr, &tmpAccess, kDTinteger)))
                return status;

            if ((0 > tmpAccess) || (tmpAccess > kMAX_ACCESS_LEVEL))
                return ERROR_GENERAL_OUT_OF_RANGE;

            tmpAccess1 = (Access)tmpAccess;
			*pAccess |= tmpAccess1;
		} 
        else 
        {
			/* find it in the list */
			pEnumTable = pTypeInfo->pEnumTable;
            match      = FALSE;
            enumValue  = 0;
			for (i = 0; i < count; i++, pEnumTable++)
			{
				if (0 != STRNCMP(pTemp, pEnumTable->EnumString, enumLen))
                    continue;

 			    enumValue = pEnumTable->EnumValue;

                if (enumLen == (int) STRLEN(pEnumTable->EnumString))
                {
                    match = TRUE;
                    break;
                }

                if (match)
                    return ERROR_GENERAL_DATA_AMBIG;

                match = TRUE;
			}

			/* Do not allow undefined option bits */
            if (! match)
        		return ERROR_GENERAL_ILLEGAL_VALUE;

            *pAccess |= enumValue;
		}

		/* get next entry */
		pTemp += enumLen; 
        if (NULL != pNext) 
            pTemp++;
	}

	return OK;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_Str(sbyte *pString, void *pDataObject, DataType DType, Boolean convert)
{
    switch (DType)
    {
        case kDTchar:
            return CONVERT_StrToByte(pString, pDataObject, convert);

        case kDTuchar:
            return CONVERT_StrToUbyte(pString, pDataObject, convert);

        case kDTshort:
            return CONVERT_StrToShort(pString, pDataObject, convert);

        case kDTushort:
            return CONVERT_StrToUshort(pString, pDataObject, convert);

        case kDTinteger:
            return CONVERT_StrToInt(pString, pDataObject, convert);

        case kDTuinteger:
            return CONVERT_StrToUnsigned(pString, pDataObject, convert);

        case kDTlong:
            return CONVERT_StrToLong(pString, pDataObject, convert);

        case kDTulong:
            return CONVERT_StrToUlong(pString, pDataObject, convert);

        case kDTipaddress:
            return CONVERT_StrToIP(pString, pDataObject, convert);

        case kDTmacaddr:
            return CONVERT_StrToMacAddr(pString, pDataObject, convert);

        case kDTstring:
            if (convert)
                STRCPY((sbyte*)pDataObject, pString);
            return OK;

        case kDTabsolute:
            if (STRLEN(pString) > 0)
            {
                if (convert)
                    STRCPY((sbyte*)pDataObject, pString);

                return OK;
            }
            else
                return ERROR_GENERAL_NO_DATA;

        default:
            break;
    }

    return ERROR_GENERAL_NOT_FOUND;

}   /* CONVERT_StrTo */

/*-----------------------------------------------------------------------*/

extern RLSTATUS CONVERT_StrTo(sbyte *pString, void *pDataObject, DataType DType)
{
    return CONVERT_Str(pString, pDataObject, DType, TRUE);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS CONVERT_StrTypeTo(sbyte *pString, void *pDataObject, struct DTTypeInfo *pTypeInfo)
{	RLSTATUS ret = ERROR_GENERAL_ILLEGAL_VALUE;

    
 	if ((NULL == pDataObject) || (NULL == pTypeInfo))
        return ERROR_GENERAL_NOT_FOUND;

    if (NULL == pString)
        return ERROR_GENERAL_NULL_POINTER;

	switch (pTypeInfo->baseType)
    {
		case kDTenum:		/* find if string is in enum list */
			ret = CONVERT_StrToEnum(pString,pDataObject,pTypeInfo);
			break;
		case kDTlist:		/* find if strings are in enum list */
			ret = CONVERT_StrToList(pString,pDataObject,pTypeInfo);
			break;
		case kDTaccess:		/* find if strings are in enum list */
			ret = CONVERT_StrToAccess(pString,pDataObject,pTypeInfo);
			break;
		default:			/* if other type, assume it is OK for now */
			ret = CONVERT_Str(pString, pDataObject,pTypeInfo->baseType, TRUE);
			break;
    }

    return(ret);
}

/*-----------------------------------------------------------------------*/

static void CONVERT_ReverseString(sbyte *pStartString)
{
    sbyte   Char;
    sbyte   *pEndString;

    if (NULL == pStartString)
        return;

    pEndString = pStartString + STRLEN(pStartString) - 1;

    while (pStartString < pEndString)
    {
        Char = *pStartString;
        *pStartString = *pEndString;
        *pEndString = Char;

        pStartString++;
        pEndString--;
    }
}

/*-----------------------------------------------------------------------*/

extern void CONVERT_HexString(sbyte4 *hex, sbyte *pBuf)
{
    sbyte   digit  = 0;
    sbyte  *pStart = pBuf;

    if (0 == *hex)
        *(pBuf)++ = '0';

    while (*hex)
    {
        digit = (0xf & *hex);
        *pBuf = CONVERT_Dec2Hex(digit);
        pBuf++;
        *hex >>= 4;
    }

    *(pBuf)++ = 'x';
    *(pBuf)++ = '0';

    *pBuf = kCHAR_NULL;
    CONVERT_ReverseString(pStart);
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_UlongToStr(unsigned long Ulong, sbyte *pString)
{
    sbyte *HeadString = pString;

    if (NULL == pString)
        return ERROR_GENERAL_NO_DATA;

    while (Ulong >= 10)
    {
        *pString++ = (sbyte)((Ulong % 10) + '0');
        Ulong /= 10;
    }

    *pString++ = (sbyte)(Ulong + '0');
    *pString = '\0';

    CONVERT_ReverseString(HeadString);

    return OK;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_LongToStr(long Long, sbyte *pString)
{
    int             Signed = (0 > Long) ? TRUE : FALSE;
    sbyte           *HeadString = pString;
    unsigned long   TempLong;

    if (NULL == pString)
        return ERROR_GENERAL_NO_DATA;

    if (TRUE == Signed)
        TempLong = (unsigned long)((~Long)+1);
    else
        TempLong = (unsigned long)Long;

    while (TempLong >= 10)
    {
        *pString++ = (sbyte)((TempLong % 10) + '0');
        TempLong /= 10;
    }

    *pString++ = (sbyte)(TempLong + '0');

    if (TRUE == Signed)
        *pString++ = '-';

    *pString = '\0';

    CONVERT_ReverseString(HeadString);

    return OK;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_IntToStr(int Int, sbyte *pString)
{
    return CONVERT_LongToStr((long)Int, pString);
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_UnsignedToStr(unsigned int Unsigned, sbyte *pString)
{
    return CONVERT_UlongToStr((unsigned long)Unsigned, pString);
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_CharToStr(char Char, sbyte *pString)
{
    return CONVERT_LongToStr((long)Char, pString);
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_UcharToStr(unsigned char Uchar, sbyte *pString)
{
    return CONVERT_UlongToStr((unsigned long)Uchar, pString);
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_ShortToStr(short Short, sbyte *pString)
{
    return CONVERT_LongToStr((long)Short, pString);
}

/*-----------------------------------------------------------------------*/

static RLSTATUS CONVERT_UshortToStr(unsigned short Ushort, sbyte *pString)
{
    return CONVERT_UlongToStr((unsigned long)Ushort, pString);
}


/* get all possible enums */
extern RLSTATUS 
CONVERT_EnumStr(DTTypeInfo *pTypeInfo, sbyte *pOutput, sbyte4 bufLen, sbyte *sep)
{	
	sbyte4      i;
    sbyte4      count;
    sbyte4      remainder   = bufLen;
    sbyte4      debug; /* delete after testing */
    sbyte       *divider;
    DTEnumInfo  *pEnumTable = pTypeInfo->pEnumTable;

	if ((NULL == pTypeInfo) || (NULL == pOutput))
        return ERROR_GENERAL_NOT_FOUND;
	
    divider = NULL != sep ? sep : ", ";

	pOutput[0] = '\0';

    count = CONVERT_GetEnumCount(pTypeInfo);
	for (i = 0; i < count; i++, pEnumTable++)
	{
    	STRNCAT(pOutput, pEnumTable->EnumString, remainder);
        remainder -= STRLEN(pEnumTable->EnumString);
	    if (i + 1 < count)
        {
            if (0 < remainder)
                STRNCAT(pOutput, divider, remainder);
            remainder -= STRLEN(divider);
        }
        debug = STRLEN(pOutput);    /* delete after testing */
        debug += remainder;         /* delete after testing */
        if (0 >= remainder)
            return OK; /* should be overflow error? */
	}
	return OK;
}

/*-----------------------------------------------------------------------*/
#if 0
static RLSTATUS CONVERT_EnumToStr(EnumType nEnum, sbyte *pString, DTTypeInfo *pTypeInfo)
{	
	int      i, count;

	if ((NULL == pTypeInfo) || (NULL == pString))
        return ERROR_GENERAL_NOT_FOUND;
	
	pString[0] = '\0';

	/* Get number of valid enums */
    count = CONVERT_GetEnumCount(pTypeInfo);

	/* find it in the list */
	for (i = 0; i < count; i++)
	{
		if (nEnum == pTypeInfo->pEnumTable[i].EnumValue)
		{
			strcat(pString, pTypeInfo->pEnumTable[i].EnumString);
			return(OK);
		}
	}

	return ERROR_GENERAL_NOT_FOUND;
}
#endif
/*-----------------------------------------------------------------------*/

extern RLSTATUS CONVERT_ListToStr(EnumType nEnum, sbyte *pString, DTTypeInfo *pTypeInfo)
{	sbyte	 *tptr, delim[2];
	int      i;
    int      hits; 
    int      count;
	EnumType mask;

	if ((NULL == pTypeInfo) || (NULL == pString))
        return ERROR_GENERAL_NOT_FOUND;
	
	pString[0] = '\0';

	/* get separater */
	tptr     = strstr(pTypeInfo->pValidateStr,"D="); 
    if (NULL==tptr) 
        return ERROR_GENERAL_NOT_FOUND;

	delim[0] = tptr[2];
	delim[1] = '\0';
    count    = CONVERT_GetEnumCount(pTypeInfo);
	mask     = 0x01;
	hits     = 0;

	while (nEnum)
	{	
		if (nEnum & mask)
		{
			/* find it in the list */
			for (i = 0; i < count; i++)
			{
				if (mask == pTypeInfo->pEnumTable[i].EnumValue)
				{
					if (hits) 
                        strcat(pString, delim);

                    strcat(pString,pTypeInfo->pEnumTable[i].EnumString);
					
					nEnum &= ~mask;
					hits++;
					break;
				}
			}
			if (i == count) 
			{
				nEnum &= ~mask;
/*				return ERROR_GENERAL_NOT_FOUND; */
			}
		}

		mask <<= 1;
	}

	return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS CONVERT_AccessToStr(Access nAccess, sbyte *pString, DTTypeInfo *pTypeInfo)
{	sbyte	 *tptr, delim[2],numstr[20];
	int      i;
    int      hits; 
    int      count;
	Access	 mask;
	unsigned long tmp_access;

	if ((NULL == pTypeInfo) || (NULL == pString))
        return ERROR_GENERAL_NOT_FOUND;
	
	pString[0] = '\0';

#ifndef	__RLI_ACCESS_LEVEL_MASK__
	return(CONVERT_ToStr(&nAccess,pString,kDTinteger));
#endif

	/* get separater */
	tptr     = strstr(pTypeInfo->pValidateStr,"D="); 
    if (NULL==tptr) 
        return ERROR_GENERAL_NOT_FOUND;

	delim[0] = tptr[2];
	delim[1] = '\0';
    count    = CONVERT_GetEnumCount(pTypeInfo);
	mask     = 0x01 << __RLI_ACCESS_LEVEL_SHIFT__;
	hits     = 0;

	/* Convert Option Bits */
	while (mask)
	{	
		if (nAccess & mask)
		{
			/* find it in the list */
			for (i = 0; i < count; i++)
			{
				if (mask == (Access) pTypeInfo->pEnumTable[i].EnumValue)
				{
					if (hits) 
                        strcat(pString, delim);

                    strcat(pString,pTypeInfo->pEnumTable[i].EnumString);
					
					nAccess &= ~mask;
					hits++;
					break;
				}
			}
			/* Do not allow undefined option bits */
			if (i == count) {
				nAccess &= ~mask;
/*				return ERROR_GENERAL_NOT_FOUND; */
			}
		}

		mask <<= 1;
	}

	/* Convert and append access level */
#if (__RLI_ACCESS_LEVEL_MASK__ > 0)
	tmp_access = (unsigned long) (nAccess & __RLI_ACCESS_LEVEL_MASK__);
	CONVERT_ToStr(&tmp_access,numstr,kDTulong);
	if (strlen(pString)) strcat(pString,delim);
	strcat(pString,numstr);
#endif

	return OK;
}

/*-----------------------------------------------------------------------*/

/* sample conversion code */

static RLSTATUS CONVERT_IPToStr(ubyte4 IPaddr, sbyte *pString)
{

    if (NULL == pString)
        return ERROR_GENERAL_NO_DATA;

    IPaddr = NTOH4(IPaddr);

    CONVERT_UlongToStr((unsigned long)((IPaddr >> 24) & 0xff), pString);
    pString += STRLEN(pString);
    *pString++ = '.';

    CONVERT_UlongToStr((unsigned long)((IPaddr >> 16) & 0xff), pString);
    pString += STRLEN(pString);
    *pString++ = '.';

    CONVERT_UlongToStr((unsigned long)((IPaddr >> 8) & 0xff), pString);
    pString += STRLEN(pString);
    *pString++ = '.';

    CONVERT_UlongToStr((unsigned long)(IPaddr & 0xff), pString);

    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS CONVERT_ToStr(void *pDataObject, sbyte *pString, DataType DType)
{
    if ((NULL == pDataObject) || (NULL == pString))
        return ERROR_GENERAL_NOT_FOUND;

    switch (DType)
    {
        case kDTinteger:
            return CONVERT_IntToStr(*((int *)pDataObject), pString);

        case kDTuinteger:
            return CONVERT_UnsignedToStr(*((unsigned int *)pDataObject), pString);

        case kDTchar:
            return CONVERT_CharToStr(*((char *)pDataObject), pString);

        case kDTuchar:
            return CONVERT_UcharToStr(*((unsigned char *)pDataObject), pString);

        case kDTshort:
            return CONVERT_ShortToStr(*((short *)pDataObject), pString);

        case kDTushort:
            return CONVERT_UshortToStr(*((unsigned short *)pDataObject), pString);

        case kDTlong:
            return CONVERT_LongToStr( *((long *)pDataObject), pString);

        case kDTulong:
            return CONVERT_UlongToStr(*((unsigned long *)pDataObject), pString);

        case kDTipaddress:
            return CONVERT_IPToStr(*((ubyte4 *)pDataObject), pString);

        case kDTstring: 
            STRCPY (pString, (sbyte *)pDataObject);
            return OK;

        default:
            break;
    }

    return ERROR_GENERAL_NOT_FOUND;

} /* CONVERT_ToStr */

/*-----------------------------------------------------------------------*/

extern RLSTATUS CONVERT_Valid(sbyte *pString, DataType DType)
{
    return CONVERT_Str(pString, NULL, DType, FALSE);
}


/* Number (in string form) compare (without atoi()...) */
/* return value: -1=A.LT.B, 0=A.EQ.B, 1=A.GT.B         */
static int CONVERT_StrNumCmp(sbyte *aptr, sbyte *bptr)
{	int IsAneg = FALSE;
	int IsBneg = FALSE;
	int iAlen,iBlen,i;

	/* handle polarity  differences*/
	if ('-' == *aptr) {IsAneg=TRUE; aptr++;}
	if ('-' == *bptr) {IsBneg=TRUE; bptr++;}
	if (!IsAneg &&  IsBneg) return( 1);
	if ( IsAneg && !IsBneg) return(-1);

    /* kill leading zeros */
    while ('0' == *aptr) aptr++;
    while ('0' == *bptr) bptr++;

	/* string length magnitude differences */
	for (iAlen=0; ISDIGIT(aptr[iAlen]);iAlen++);
	for (iBlen=0; ISDIGIT(bptr[iBlen]);iBlen++);
	if (iAlen < iBlen) return((IsAneg) ?  1:-1);
	if (iAlen > iBlen) return((IsAneg) ? -1: 1);

	/* string value magnitude differences */
	for (i=0;i<iAlen;i++)
	{
		if (*aptr   < *bptr  ) return((IsAneg) ?  1:-1);
		if (*aptr++ > *bptr++) return((IsAneg) ? -1: 1);
	}

	/* else, it is equal */
	return(0);	
}

/*-----------------------------------------------------------------------*/

StringType VALID_GetStringType(sbyte *pValidStr)
{
    sbyte      *tptr;
    sbyte4      index;
    StringInfo *code   = validStrings;

	if (NULL == (tptr = strstr(pValidStr, "T=")))
        return kSTRING_Invalid;
            
    tptr += 2;

    for (index = 0; index < ARRAY_SIZE(validStrings); index++, code++)
    {
        if (0 == STRNCMP(tptr, code->stringCode, STRLEN(code->stringCode)))
            return code->stringType;
    }

    return kSTRING_Invalid;
}

/*-----------------------------------------------------------------------*/

RLSTATUS VALID_Length(sbyte *pString, sbyte *pValidStr)
{
    sbyte4  i, j, count, size;
    sbyte  *tptr;
    sbyte   str[10];

	if (NULL == (tptr = strstr(pValidStr, "N=")))
        return OK;
            
    tptr += 2;
	for (i = 0, j = 0; ISDIGIT(tptr[i]) && (i < sizeof(str)-1); i++, j++) 
        str[j] = tptr[i];
			
    str[j] = '\0'; 
            
    count = ATOI(str);
    size  = (sbyte4) STRLEN(pString);

    return (size > count ? ERROR_CONVERSION_TOO_LONG : OK);
}

/*-----------------------------------------------------------------------*/

RLSTATUS VALID_String(sbyte *pString, sbyte *pValidStr)
{
    StringType  stringType;

    if (kSTRING_Invalid == (stringType = VALID_GetStringType(pValidStr)))
        return ERROR_GENERAL_ILLEGAL_VALUE;
            
    switch(stringType)
    {
    case kSTRING_Alpha:
        while (kCHAR_NULL != *pString)
        {
            if (!ISALPHA(*(pString++)))
                return ERROR_GENERAL_ILLEGAL_VALUE;
        }
        break;
    case kSTRING_AlphaNumeric:
        while (kCHAR_NULL != *pString)
        {
            if (!ISALPHANUMERIC(*(pString++)))
                return ERROR_GENERAL_ILLEGAL_VALUE;
        }
        break;
    case kSTRING_Numeric:
        while (kCHAR_NULL != *pString)
        {
            if (!ISDIGIT(*(pString++)))
                return ERROR_GENERAL_ILLEGAL_VALUE;
        }
        break;
    case kSTRING_Any:
        break;
    default:
        return ERROR_GENERAL_ILLEGAL_VALUE;
    }

    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
CONVERT_Validate(environment *pEnv, sbyte *pString, 
                 DTTypeInfo *pTypeInfo, Boolean errorMsg)
{   
    RLSTATUS         ret = OK;
	sbyte	        *pBuf;
	EnumType         nEnum;
	Access	         nAccess;
	ubyte4           nIPaddr;
    long             nMacAddr;
	sbyte	        *pValidStr;
	DTTypeInfo       tempInfo;
	DTTypeInfo      *pTempInfo;
    ValidateHandler *pCustomValidator;

	if ((NULL == pTypeInfo) || (NULL == pString))
        return ERROR_GENERAL_NOT_FOUND;

	/* If Validate String is NULL, insert default validate string for its base type */
	if (NULL == (pValidStr = pTypeInfo->pValidateStr))
	{	
        pTempInfo = CONVERT_TypeToInfo(pTypeInfo->baseType);
        if (NULL == pTempInfo)
            return ERROR_CONVERSION_INCORRECT_TYPE;

        tempInfo  = *pTempInfo;
		pValidStr = tempInfo.pValidateStr;
	}

	/* if there is a custom validator, call that instead */
	pCustomValidator = (ValidateHandler *) pTypeInfo->pCustomValidate;
	if (NULL != pCustomValidator)
	{	
		/* BLock circular loops in case that validator calls this one */
		tempInfo = *pTypeInfo;
		tempInfo.pCustomValidate = NULL;

		return pCustomValidator(pEnv, pString, &tempInfo, errorMsg);
	}
	
	switch (pTypeInfo->baseType)
    {
        case kDTinteger:	
        case kDTuinteger:	
        case kDTchar:		
        case kDTuchar:		
        case kDTshort:		
        case kDTushort:		
        case kDTlong:		
        case kDTulong:
            if (! CONVERT_IsNumber(pString))
                return ERROR_CONVERSION_INCORRECT_TYPE;
			/* Check lower bound */
			if (NULL == (pBuf = strstr(pValidStr,"L="))) 
                break;
			if (-1 == CONVERT_StrNumCmp(pString , &pBuf[2]))
            {
                ret = ERROR_GENERAL_OUT_OF_RANGE;
                break;
            }
			/* Check upper bound */
			if (NULL == (pBuf = strstr(pValidStr,"U=")))
                break;
			if (1 == CONVERT_StrNumCmp(pString , &pBuf[2]))
                ret = ERROR_GENERAL_OUT_OF_RANGE;
            break;
        case kDTstring:		/* evaluate string contents and length */
            if (OK != (ret = VALID_String(pString, pValidStr)))
                return ret;
			/* Check string length */
            if (OK != (ret = VALID_Length(pString, pValidStr)))
                return ret;
            break;
        case kDTipaddress:	/* evaulate 256.256.256.256 format */
			ret = CONVERT_StrToIP(pString, &nIPaddr, FALSE);
			break;
		case kDTmacaddr:	/* evaulate xx:xx:xx:xx:xx:xx format */
			ret = CONVERT_StrToMacAddr(pString, &nMacAddr, FALSE);
			break;
		case kDTenum:		/* find if string is in enum list */
			ret = CONVERT_StrToEnum(pString, &nEnum, pTypeInfo);
			/* CONVERT_EnumToStr(nEnum,pTemp,pTypeInfo); */
			break;
		case kDTlist:		/* find if strings are in enum list */
			ret = CONVERT_StrToList(pString, &nEnum, pTypeInfo);
			/* CONVERT_ListToStr(nEnum,pTemp,pTypeInfo); */
			break;
		case kDTaccess:		/* find if strings are in enum list */
			ret = CONVERT_StrToAccess(pString, &nAccess, pTypeInfo);
			/* CONVERT_AccessToStr(nAccess,pTemp,pTypeInfo); */
			break;
		default:			/* if other type, assume it is OK for now */
			break;
    }

    return(ret);
}


#ifdef NOT_NOW
#define InfoMinMax(l,u) "L=" #l " U=" #u ""
DTTypeInfo pDTinteger	= {NULL, kDTinteger  , InfoMinMax(kMIN_INT,kMAX_INT          ), 0, NULL, NULL};
DTTypeInfo pDTuinteger	= {NULL, kDTuinteger , InfoMinMax(kMIN_UNSIGNED,kMAX_UNSIGNED), 0, NULL, NULL};
DTTypeInfo pDTchar		= {NULL, kDTchar     , InfoMinMax(kMIN_CHAR,kMAX_CHAR        ), 0, NULL, NULL};
DTTypeInfo pDTuchar		= {NULL, kDTuchar    , InfoMinMax(kMIN_UCHAR,kMAX_UCHAR      ), 0, NULL, NULL};
DTTypeInfo pDTshort		= {NULL, kDTshort    , InfoMinMax(kMIN_SHORT,kMAX_SHORT      ), 0, NULL, NULL};
DTTypeInfo pDTushort	= {NULL, kDTushort   , InfoMinMax(kMIN_USHORT,kMAX_USHORT    ), 0, NULL, NULL};
DTTypeInfo pDTlong		= {NULL, kDTlong     , InfoMinMax(kMIN_LONG,kMAX_LONG        ), 0, NULL, NULL};
DTTypeInfo pDTulong		= {NULL, kDTulong    , InfoMinMax(kMIN_ULONG,kMAX_ULONG      ), 0, NULL, NULL};
DTTypeInfo pDTipaddress = {NULL, kDTipaddress, "256.256.256.256"                      , 0, NULL, NULL};
DTTypeInfo pDTmacaddr   = {NULL, kDTmacaddr  , "xx:xx:xx:xx:xx:xx"                    , 0, NULL, NULL};
DTTypeInfo pDTstring	= {NULL, kDTstring   , "T=AL N=256"                           , 0, NULL, NULL};
#else
DTTypeInfo pDTuchar		= {NULL, NULL, kDTuchar    , "L=0 U=255"                 , 0, NULL, NULL};
DTTypeInfo pDTushort	= {NULL, NULL, kDTushort   , "L=0 U=65535"               , 0, NULL, NULL};
DTTypeInfo pDTuint2		= {NULL, NULL, kDTuinteger , "L=0 U=65535"               , 0, NULL, NULL};
DTTypeInfo pDTuint4		= {NULL, NULL, kDTuinteger , "L=0 U=4294967295"          , 0, NULL, NULL};
DTTypeInfo pDTulong		= {NULL, NULL, kDTulong    , "L=0 U=4294967295"          , 0, NULL, NULL};
DTTypeInfo pDTchar		= {NULL, NULL, kDTchar     , "L=-128 U=127"              , 0, NULL, NULL};
DTTypeInfo pDTshort		= {NULL, NULL, kDTshort    , "L=-32768 U=32767"          , 0, NULL, NULL};
DTTypeInfo pDTint2		= {NULL, NULL, kDTinteger  , "L=-32768 U=32767"          , 0, NULL, NULL};
DTTypeInfo pDTint4 		= {NULL, NULL, kDTinteger  , "L=-2147483648 U=2147483647", 0, NULL, NULL};
DTTypeInfo pDTlong 		= {NULL, NULL, kDTlong     , "L=-2147483648 U=2147483647", 0, NULL, NULL};
DTTypeInfo pDTipaddress = {NULL, NULL, kDTipaddress, "256.256.256.256"           , 0, NULL, NULL};
DTTypeInfo pDTmacaddr   = {NULL, NULL, kDTmacaddr  , "xx:xx:xx:xx:xx:xx"         , 0, NULL, NULL};
DTTypeInfo pDTstring	= {NULL, NULL, kDTstring   , "T=AL N=256"                , 0, NULL, NULL};
#endif

extern DTTypeInfo *CONVERT_TypeToInfo(DataType DType)
{
	switch (DType)
    {
		case kDTinteger:	switch (sizeof(int)) {
								case 2 : return &pDTint2; break;
								case 4 : return &pDTint4; break;
							};
							break;
        case kDTuinteger:	switch (sizeof(unsigned int)) {
								case 2 : return &pDTuint2; break;
								case 4 : return &pDTuint4; break;
							};
							break;
        case kDTchar:		return &pDTchar;
        case kDTuchar:		return &pDTuchar;
        case kDTshort:		return &pDTshort;
        case kDTushort:		return &pDTushort;
        case kDTlong:		return &pDTlong;
        case kDTulong:		return &pDTulong;
        case kDTipaddress:	return &pDTipaddress;
        case kDTmacaddr:	return &pDTmacaddr;
        case kDTstring:		return &pDTstring;
    }

    return NULL;
}

/*-----------------------------------------------------------------------*/

