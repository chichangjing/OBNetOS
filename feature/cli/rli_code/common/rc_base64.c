/*  
 *  rc_base64.c
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
#include "rc_rlstdlib.h"
#include "rc_base64.h" 


#define kBASE64_BITMASK         0x3f
#define kBASE64_NumBitsPerByte  8
#define kBASE64_PAD             '='
#define BASE64_BadChar          0xff



/*-----------------------------------------------------------------------*/

#ifdef __BASE64_ENCODE_ENABLED__
extern Length BASE64_EncodeBytesRequired(Length NumBytes)
{
    return (Length)(9 + ((NumBytes * 8) / 6));
}
#endif



/*-----------------------------------------------------------------------*/

extern Length BASE64_DecodeBytesRequired(Length NumBytes)
{
    return (Length)(9 + ((NumBytes * 6) / 8));
}



/*-----------------------------------------------------------------------*/

#ifdef __BASE64_ENCODE_ENABLED__
static ubyte BASE64_EncodeConvert(ubyte Char)
{
    Char &= 0x3f;

    if (Char < 26)
        return (ubyte)(Char + 'A');

    if (Char < 52)
        return (ubyte)((Char-26) + 'a');

    if (Char < 62)
        return (ubyte)((Char-52) + '0');
    
    if (62 == Char)
        return (ubyte)'+';
    
    return (ubyte)'/';
}
#endif



/*-----------------------------------------------------------------------*/

static ubyte BASE64_DecodeConvert(ubyte Char)
{
    if ((Char >= 'A') && (Char <= 'Z'))
        return (ubyte)(Char - 'A');

    if ((Char >= 'a') && (Char <= 'z'))
        return (ubyte)(Char - 'a' + 26);

    if ((Char >= '0') && (Char <= '9'))
        return (ubyte)(Char - '0' + 52);
    
    if ('+' == Char)
        return (ubyte)62;

    if ('/' == Char)
        return (ubyte)63;

    return BASE64_BadChar;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS BASE64_Decode(ubyte *pSource, ubyte *pDest, Length SrcLen, Length *pOutputLen)
{
    ubyte2  Convert = 0;    /* scratch space */
    ubyte2  RightEdge = 0;  /* bit "pointer" to the right edge of the scratch space */
    ubyte   Char;
    Length  OutputLen = 0;

    if ((0 == SrcLen) || (NULL == pSource) || (NULL == pDest) || (NULL == pOutputLen))
        return ERROR_GENERAL_NO_DATA;

    while ((0 < SrcLen) && (kBASE64_PAD != *pSource))
    {
        if (8 <= RightEdge)             /* top byte ready to be written */
        {
            *pDest = (ubyte)((Convert >> 8) & 0xff);

            pDest++;
            OutputLen++;

            Convert <<= 8;              /* shift out the written byte */
            RightEdge -= 8;
        }

        if (BASE64_BadChar != (Char = BASE64_DecodeConvert(*pSource)))
        {
            Convert |= (ubyte2)(((ubyte2) Char) << (10 - RightEdge));
            RightEdge += 6;             /* adjust for the 6 bits we just appended */
        }

        pSource++;
        SrcLen--;
    }

    /* push out the remaining bits */

    if (0 != RightEdge)
    {
        *pDest = (ubyte)((Convert >> 8) & 0xff);

        pDest++;
        OutputLen++;
    }

    *pDest = '\0';      /* null terminate the base64 string */
    *pOutputLen = OutputLen;

    return OK;

} /* BASE64_Decode */



/*-----------------------------------------------------------------------*/

#ifdef __BASE64_ENCODE_ENABLED__
extern RLSTATUS BASE64_Encode(ubyte *pSource, ubyte *pDest, Length NumBytes, Length *pOutputLen)
{
    Counter ShiftLeft;
    Counter NumBytesWritten = 0;
    Counter NumBytesPad;
    ubyte2  Convert = 0;    /* scratch space */
    ubyte2  BitMask;
    ubyte   B64EncodedChar;
    
    if ((0 == NumBytes) || (NULL == pSource) || (NULL == pDest) || (NULL == pOutputLen))
        return ERROR_GENERAL_NO_DATA;

    Convert = *pSource << kBASE64_NumBitsPerByte;
    BitMask = 0xff << kBASE64_NumBitsPerByte;   /* bitmask is used to keep track of
                                                   when convert can be written to safely */
    pSource++;
    NumBytes--;

    while (NumBytes > 0)
    {
        B64EncodedChar =                    /* copy out top six bits of the ubyte2 */
            BASE64_EncodeConvert((ubyte)(((Convert >> 10) & kBASE64_BITMASK)));

        /* shift out 6 bits, 2 at a time */
        for (ShiftLeft = 0; ShiftLeft < 3; ShiftLeft++)
        {
            if (0 == (BitMask & 0xff))      /* lower byte empty? */
            {
                Convert |= *pSource;        /* lower byte empty, fill it up */
                BitMask |= 0xff;            /* mark lower byte filled */

                pSource++;
                NumBytes--;
            }

            Convert <<= 2;
            BitMask <<= 2;
        }

        Convert &= 0xffff;
        BitMask &= 0xffff;

        *pDest = B64EncodedChar;
        NumBytesWritten++;
        pDest++;
    }

    /* make sure convert is empty, push out the remaining bits */

    while (0 != BitMask)
    {
        B64EncodedChar =                    /* copy out top six bits of the ubyte2 */
            BASE64_EncodeConvert((ubyte)(((Convert >> 10) & kBASE64_BITMASK)));

        Convert <<= 6;
        BitMask <<= 6;

        Convert &= 0xffff;
        BitMask &= 0xffff;

        *pDest = B64EncodedChar;
        NumBytesWritten++;
        pDest++;
    }

    NumBytesPad = (NumBytesWritten & 3);

    while (NumBytesPad)
    {
        *pDest = kBASE64_PAD;

        NumBytesPad--;
        pDest++;
    }

    *pDest = '\0';                          /* null terminate the base64 encoded string */
    *pOutputLen = NumBytesWritten;

    return OK;

} /* BASE64_Encode */
#endif

