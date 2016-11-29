/*  
 *  rc_sizeof.c
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
#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_linklist.h"
#include "rc_sizeof.h"
#include "rc_os_spec.h"
#include "rc_database.h"

#ifndef __DISABLE_STRUCTURES__



/*-----------------------------------------------------------------------*/

extern Counter SIZEOF_Type(DataType DType)
{
    switch (DType)
    {
        case kDTinteger:
            return sizeof(int);

        case kDTuinteger:
            return sizeof(unsigned int);

        case kDTchar:
            return sizeof(char);

        case kDTuchar:
            return sizeof(unsigned char);

        case kDTshort:
            return sizeof(short);

        case kDTushort:
            return sizeof(unsigned short);

        case kDTlong:
            return sizeof(long);

        case kDTulong:
            return sizeof(unsigned long);

        case kDTipaddress:
            return sizeof(ubyte4);

        case kDTstring:
            return kMagicMarkupBufferSize;

        case kDTenum:
        case kDTlist:
            return sizeof(unsigned long);

        case kDTaccess:
            return sizeof(Access);
    }

    return 0;
}

#endif /* __DISABLE_STRUCTURES__ */
