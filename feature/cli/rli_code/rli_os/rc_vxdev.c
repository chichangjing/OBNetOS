/*
 *  rc_vxDev.c    (OpenControl virtual device driver --- interface for SNMP Research)
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

$History: rc_vxdev.c $
 * 
 * *****************  Version 7  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 6  *****************
 * User: Schew        Date: 5/02/00    Time: 3:07p
 * Change rc_oc_vxdev.c to rc_vxdev.c
 * 
 * *****************  Version 5  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/
#include "rc_options.h"

#ifdef __MASTER_SLAVE_SNMP_STACK__
#ifdef __VXWORKS_OS__

#include <vxWorks.h>
#include <selectLib.h>
#include <iosLib.h>
#include <stdio.h>
#include <taskLib.h>

#include "rcw_webctrl.h"
#include "rc_socks.h"
#include "rcm_ocsr.h"
#include "rcm_vxdev.h"

/* Virtual Device Driver Data */
typedef struct
{
    DEV_HDR                 devHdr;
    SEL_WAKEUP_LIST         selWakeupList;

} WC_DEV;

static WC_DEV               mVirtualWebControlDevice;
static int                  mNumOfConnectionsPending = 0;



/*-----------------------------------------------------------------------*/

extern STATUS WCvxDev_Create(char *name)
{
    return (int)&mVirtualWebControlDevice;
}



/*-----------------------------------------------------------------------*/

extern STATUS WCvxDev_Ioctl(WC_DEV *pVxWcDevice, int request, int *arg)
{
    /* Ioctl code provides an interface for select() */

    /* Need to mutex protect the Ioctl to prevent race condition bugs occurring,
       when multiple blades accessing SR's stack. JAB */

    WCSR_MutexWait();

    switch (request)
    {
        case FIOSELECT:
            if (ERROR == selNodeAdd(&mVirtualWebControlDevice.selWakeupList,
                                    (SEL_WAKEUP_NODE *)arg))
            {
                printf("WCvxDev_Ioctl:  selNodeAdd() returned an error.\n");
            }

            if (mNumOfConnectionsPending)
            {
                selWakeup((SEL_WAKEUP_NODE *)arg);
                mNumOfConnectionsPending--;

                if (0 > mNumOfConnectionsPending)
                    printf("WCvxDev_Ioctl: mNumOfConnectionsPending = %d!!!!!!!!\n", mNumOfConnectionsPending);

                break;
            }

            break;

        case FIOUNSELECT:
            selNodeDelete(&mVirtualWebControlDevice.selWakeupList, (SEL_WAKEUP_NODE *)arg);

            break;
    }

    WCSR_MutexRelease();

    return 0;

}  /* WCvxDev_Ioctl */



/*-----------------------------------------------------------------------*/

extern STATUS WCvxDev_Driver(void)
{
    int DriveNum;

    /* Install the WebControl virtual device driver. */
    if (0 > (DriveNum = iosDrvInstall(WCvxDev_Create, (FUNCPTR)NULL, (FUNCPTR)NULL,
                                      (FUNCPTR)NULL,  (FUNCPTR)NULL, (FUNCPTR)NULL,
                                      WCvxDev_Ioctl)) )
    {
        printf("WCvxDev_Driver: iosDrvInstall() returned an error.\n");
    }

    if (0 > iosDevAdd((DEV_HDR *)&mVirtualWebControlDevice, wcDev, DriveNum))
        printf("WCvxDev_Driver:  iosDevAdd() returned an error!\n");

    selWakeupListInit(&mVirtualWebControlDevice.selWakeupList);

    return DriveNum;

}  /* WCvxDev_Driver */



/*-----------------------------------------------------------------------*/

extern void WCvxDev_Tickle(void)
{
    WCSR_MutexWait();

    /* Tickle SR to let them know there is a WebControl event pending */

    if (1 > selWakeupListLen(&mVirtualWebControlDevice.selWakeupList))
    {
        /* Nothing to tickle right now, queue up the request for later... */
        mNumOfConnectionsPending++;
        WCSR_MutexRelease();

        return;
    }

    /* Tickle the select waiting on our Virtual Device */
    selWakeupAll(&mVirtualWebControlDevice.selWakeupList, SELREAD);

    WCSR_MutexRelease();
}

#endif /* __VXWORKS_OS__ */
#endif /* __MASTER_SLAVE_SNMP_STACK__ */
