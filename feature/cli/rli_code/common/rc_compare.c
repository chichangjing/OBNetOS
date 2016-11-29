/*  
 *  rc_compare.c
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





#include "rc_options.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_compare.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_database.h"


/*-----------------------------------------------------------------------*/

extern Boolean COMPARE_Uchars(void *pUchar, void *pUchar1)
{
    if ( (NULL == pUchar) || (NULL == pUchar1) )
        return FALSE;
    else
        return (*((unsigned char *)pUchar) == *((unsigned char *)pUchar1)) ? TRUE : FALSE;
}


/*-----------------------------------------------------------------------*/
/* sample comparison code */

extern Boolean COMPARE_IP(void *pIPaddr, void *pIPaddr1)
{
    if ( (NULL == pIPaddr) || (NULL == pIPaddr1) )
        return FALSE;
    else
        return (*((ubyte4 *)pIPaddr) == *((ubyte4 *)pIPaddr1)) ? TRUE : FALSE;
}


/*-----------------------------------------------------------------------*/

extern Boolean COMPARE_Ints(void *pInt, void *pInt1)
{
    if ( (NULL == pInt) || (NULL == pInt1) )
        return FALSE;
    else
        return (*((int *)pInt) == *((int *)pInt1)) ? TRUE : FALSE;
}


/*-----------------------------------------------------------------------*/

extern Boolean COMPARE_UnsignedInts(void *pUnsigned, void *pUnsigned1)
{
    if ( (NULL == pUnsigned) || (NULL == pUnsigned1) )
        return FALSE;
    else
        return (*((unsigned int *)pUnsigned) == *((unsigned int *)pUnsigned1)) ? TRUE : FALSE;
}


/*-----------------------------------------------------------------------*/

extern Boolean COMPARE_Shorts(void *pShort, void *pShort1)
{
    if ( (NULL == pShort) || (NULL == pShort1) )
        return FALSE;
    else
        return (*((short *)pShort) == *((short *)pShort1)) ? TRUE : FALSE;
}


/*-----------------------------------------------------------------------*/

extern Boolean COMPARE_Ushorts(void *pUshort, void *pUshort1)
{
    if ( (NULL == pUshort) || (NULL == pUshort1) )
        return FALSE;
    else
        return (*((unsigned short *)pUshort) == *((unsigned short *)pUshort1)) ? TRUE : FALSE;
}   


/*-----------------------------------------------------------------------*/

extern Boolean COMPARE_Longs(void *pLong, void *pLong1)
{
    if ( (NULL == pLong) || (NULL == pLong1) )
        return FALSE;
    else
        return (*((long *)pLong) == *((long *)pLong1)) ? TRUE : FALSE;
}


/*-----------------------------------------------------------------------*/

extern Boolean COMPARE_Ulongs(void *pUlong, void *pUlong1)
{
    if ( (NULL == pUlong) || (NULL == pUlong1) )
        return FALSE;
    else
        return (*((unsigned long *)pUlong) == *((unsigned long *)pUlong1)) ? TRUE : FALSE;
}

/*-----------------------------------------------------------------------*/

extern Boolean COMPARE_Strings(void *pStr, void *pStr1)
{
    if ( (NULL == pStr) || (NULL == pStr1) )
        return FALSE;
    else
        return (STRCMP(pStr, pStr1) == 0) ? TRUE : FALSE;
}



/*-----------------------------------------------------------------------*/

extern Boolean COMPARE_Values(void *pDataObject, void *pDataObject1, DataType DType)
{
    switch (DType)
    {
        case kDTinteger:
            return COMPARE_Ints(pDataObject, pDataObject1);

        case kDTuinteger:
            return COMPARE_UnsignedInts(pDataObject, pDataObject1);

        case kDTshort:
            return COMPARE_Shorts(pDataObject, pDataObject1);

        case kDTushort:
            return COMPARE_Ushorts(pDataObject, pDataObject1);

        case kDTlong:
            return COMPARE_Longs(pDataObject, pDataObject1);

        case kDTulong:
            return COMPARE_Ulongs(pDataObject, pDataObject1);

        case kDTipaddress:
            return COMPARE_IP(pDataObject, pDataObject1);
        
        case kDTstring:
            return COMPARE_Strings(pDataObject, pDataObject1);

        default:
            break;
    }

    return FALSE;
}

