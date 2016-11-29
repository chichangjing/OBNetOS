/*  
 *  vd_defs.h
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

#ifndef __VD_DEFS_HEADER__
#define __VD_DEFS_HEADER__

#define kVD_MaxStringSize           32
#define kVD_MaxErrorStringSize      128
#define kVD_SelectedString          "SELECTED"
#define kVD_CheckBoxOn              "ON"
#define kVD_CheckBoxChecked         "CHECKED"

/* Identifiers for the Pages */
#define kVD_START_html          "/vd_start.htm"
#define kVD_BASIC_html          "/vd_basic.htm"
#define kVD_TEXT_html           "/vd_text.htm"
#define kVD_TXTA_html           "/vd_txta.htm"
#define kVD_SEL_html            "/vd_sel.htm"
#define kVD_CB_html             "/vd_cb.htm"
#define kVD_RB_html             "/vd_rb.htm"
#define kVD_CACHE_html          "/vd_cache.htm"
#define kVD_REPT_html           "/vd_rept.htm"
#define kVD_UPLD_html           "/vd_upld.htm"
#define kVD_UPLD2_html          "/vd_upld2.htm"
#define kVD_EXTRA_html          "/vd_extra.htm"
#define kVD_SMTP_html           "/vd_smtp.htm"
#define kVD_AUTH_html           "/vd_auth.htm"
#define kVD_ENUM_html           "/vd_enum.htm"

#define kVD_SNMP_DATA_SCALAR_html        "/vd_snmps.htm"
#define kVD_SNMP_DATA_TABLE_SETS_html    "/vdsnmpts.htm"

#define kVD_GOOD_POST_html          "/vd_post.htm"
#define kVD_GOOD_CACHE_POST_html    "/gd_cache.htm"
#define kVD_GOOD_REPT_POST_html     "/gd_rept.htm"

#define kVD_ERROR_txt           "/vd_error.txt"
#define kVD_WC_INTERNAL_MSG_txt "/int_msg.txt"

#ifndef __VD_GLUE_HEADER__
typedef ubyte4              IpAddress;
#endif

#endif /* __VD_DEFS_HEADER__ */
