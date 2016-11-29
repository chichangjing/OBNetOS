/*  
 *  rc_binsrch.c
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
#include "rc_binsrch.h"



#if  (defined(__DATABASE_USE_ARRAY__) || defined(__RLI_MIB_TRANSLATION__))



/*-----------------------------------------------------------------------*/

extern void *
BinSrch_FindObj ( void          *pArrayStart, 
                  sbyte4        numEntries,
                  ubyte4        sizeOfEntry,
                  void          *pObject, 
                  CompareType   (*funcCompare)(void *pObject1,void *pObject2) )
{
    sbyte4      lowerBound, upperBound, middle;
    CompareType compResult;
    void        *pEntry;
    
    if (( NULL == pArrayStart   ) || ( NULL == funcCompare  ) ||    
        ( 0 == sizeOfEntry      ) || ( 0 >= numEntries      )   )
    {
        return NULL;
    }

    lowerBound = 0;
    upperBound = numEntries - 1;

    while ( TRUE )
    {
        if ( lowerBound > upperBound )
            return NULL; /* Wasn't found */
        
        /* The middle of the range is found through integer division */
        middle = ( lowerBound + upperBound ) >> 1;
        
        pEntry      = (void*) (sizeOfEntry * middle + (ubyte4)pArrayStart);
        compResult  = funcCompare( pEntry, pObject );

        switch( compResult )
        {
            case LESS_THAN:
                    lowerBound = middle + 1;
                    break;

            case EQUAL:
                    return pEntry;

            case GREATER_THAN:
                    upperBound = middle - 1;
                    break;
        }
    }

} /* BinSrch_FindObj */

#endif  /*__DATABASE_USE_ARRAY__*/
