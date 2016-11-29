
/*
 *  rc_enum_tab.h
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

/* This file contains a sample enumeration table for MIBII. Customer should create their
   own file for their project */

/* some day an auto-generated file...! */

#ifdef __IN_ENUM_C__
 
/* !-!-!-!-!-!-! following will only be included if in the file rc_enum.c */
 
static enumStruct enumSysServices[] = 
{
    { 0x00, "No Services"      },
    { 0x01, "Physical"    },  
    { 0x03, "Data Link, Physical" },
    { 0x07, "Internet, Data Link, Physical"},
    { 0x0F, "End-to-End, Internet, Data Link, Physical"},
    { 0x4F, "Applications, End-to-End, Internet, Data Link, Physical"}
};

static enumStruct enumTcpConnState[] = 
{
    { 1, "closed"},
    { 2, "listen"},
    { 3, "synSent"},
    { 4, "synReceived"},
    { 5, "established"},
    { 6, "finWait1"},
    { 7, "finWait2"},
    { 8, "closeWait"},
    { 9, "lastAck"},
    { 10, "closing"},
    { 11, "timeWait"},
    { 12, "deleteTCB"}
};

static enumStruct enumIfType[] = 
{
    { 1, "other" },
    { 2, "regular1822" },
    { 3, "hdh1822" },
    { 4, "ddn-x25" },
    { 5, "rfc877-x25" }, 
    { 6, "ethernet-csmacd" },
    { 7, "iso88023-csmacd" },
    { 8, "iso88024-tokenBus" },
    { 9, "iso88025-tokenRing" },
    { 10, "iso88026-man" },
    { 11, "starLan" },
    { 12, "proteon-10Mbit" },
    { 13, "proteon-80Mbit" },
    { 14, "hyperchannel" },
    { 15, "fddi" },
    { 16, "lapb" },
    { 17, "sdlc" },
    { 18, "ds1" },
    { 19, "e1" },
    { 20, "basicISDN" },
    { 21, "primaryISDN" },
    { 22, "propPointToPointSerial" },
    { 23, "ppp" },
    { 24, "softwareLoopback" },
    { 25, "eon" },
    { 26, "ethernet-3Mbit" },
    { 27, "nsip" },
    { 28, "slip" },
    { 29, "ultra" },
    { 30, "ds3" },
    { 31, "sip" },
    { 32, "frame-relay" }
};

static enumStruct enumIfStatus[] =
{
    { 1, "up" },
    { 2, "down" },
    { 3, "testing" }
};

 
static enumStruct enumPairs1[] = 
{
    {  1, "other"      },
    {  2, "invalid"    },  
    {  3, "learned" },
    {  4, "self" },
    {  5, "mgmt" }
};

static enumStruct enumPairs2[] = 
{
    {  1, "other"      },
    {  2, "valid"     },  
    {  3, "ignore"     },  
    {  4, "delete"     },  
    {  5, "create"     }
};

static enumStruct enumPairs3[] = 
{
    {  1, "up"      },
    {  2, "down"    },  
    {  3, "testing" }
};

static enumStruct enumPairs4[] = 
{
    {  1, "up"      },
    {  2, "down"    },  
    {  3, "testing" }
};

static enumStruct enumPairs5[] = 
{
    {  1, "up"      },
    {  2, "down"    },  
    {  3, "testing" }
};

 
static enumStruct enumRouteType[] =
{
	{ 1, "Other" },
	{ 2, "Invalid" },
	{ 3, "Direct" },
	{ 4, "Indirect" }
};

static enumStruct enumRouteProto[] =
{
	{ 1, "Other" },
	{ 2, "Local" },
	{ 3, "NetManagement" },
	{ 4, "Icmp" },
	{ 5, "Egp" },
	{ 6, "Ggp" }
};

static enumTableLookupStruct enumTableList[] =
{
    { "enumSysServices", enumSysServices, ENUM_LIST_LENGTH(enumSysServices) },
    { "enumTcpConnState", enumTcpConnState, ENUM_LIST_LENGTH(enumTcpConnState)},
    { "enumIfType", enumIfType, ENUM_LIST_LENGTH(enumIfType) },
    { "enumIfStatus", enumIfStatus, ENUM_LIST_LENGTH(enumIfStatus) },
    { "dot1dTpFdbStatusEnum", enumPairs1, ENUM_LIST_LENGTH(enumPairs1) },
    { "s5AgTrpRcvrStatusEnum", enumPairs2, ENUM_LIST_LENGTH(enumPairs2) },
    { "enumList3", enumPairs3, ENUM_LIST_LENGTH(enumPairs2) },
    { "enumList4", enumPairs4, ENUM_LIST_LENGTH(enumPairs2) },
    { "enumList5", enumPairs5, ENUM_LIST_LENGTH(enumPairs2) },
    { "enumRouteType", enumRouteType, ENUM_LIST_LENGTH(enumRouteType) },
    { "enumRouteProto", enumRouteProto, ENUM_LIST_LENGTH(enumRouteProto) }
};

#endif /* __ENUM_HEADER__ */


