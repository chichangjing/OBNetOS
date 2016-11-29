/*  
 *  rc_auth.c
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






#include "rc.h"

#if defined(__JAVASCRIPT_DIGEST_ENABLED__) || defined(__BROWSER_DIGEST_ENABLED__)

#include <stdio.h>


/* !!!!! this should be removed at a later point....*/
#include "rcw_post.h"

#include "rc_md5.h"
#include "rc_auth.h"
#include "rc_occustom.h"

#define kIllegal_IpAddr     0

#if 0
#define __DEBUG_AUTH__
#endif



/*-----------------------------------------------------------------------*/

typedef struct authInfo
{
    ubyte4      ClientIpAddr;                   /* !!!!!!!!!!!!!!!!!!!!!! IPv6, IPv8 warning */
    Access      ClientAccessLevel;
    Boolean     ClientAuthenticated;            /* has the client been authenticated yet? */
    sbyte*      pUserName;

    Counter     ConsecutiveIllegalAttempts;     /* Covert break in attempts */
    Counter     ConsecutiveFailedLogins;        /* Basic login failures */

    ubyte4      FirstLoginSecs;                 /* used for keeping track when user first logged in */
    ubyte4      LastAccessSecs;                 /* used for detecting idle timeouts */

    sbyte       SentNonce[kMaxNonceLen];
    sbyte       ReplyDigest[kDigestMesgBufSize];

} authInfo;



/*-----------------------------------------------------------------------*/

static OS_SPECIFIC_MUTEX    mAuthMutex;
static authInfo             *LoggedInClients = NULL;

/*-----------------------------------------------------------------------*/

static void ClearClientRecords(authInfo *pClientRecord)
{
    MEMSET(pClientRecord, 0x00, sizeof(authInfo));

    pClientRecord->ClientAuthenticated  = FALSE;
    pClientRecord->ClientIpAddr         = kIllegal_IpAddr;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS AUTH_Construct(void)
{
    int clientIndex;

    if (NULL == (LoggedInClients = RC_MALLOC(sizeof(authInfo) * kHwMaxSimultaneousClients)))
        return SYS_ERROR_NO_MEMORY;

    for (clientIndex = 0; clientIndex < kHwMaxSimultaneousClients; clientIndex++)
        ClearClientRecords(&(LoggedInClients[clientIndex]));

    return OS_SPECIFIC_MUTEX_CREATE(&mAuthMutex);
}



/*-----------------------------------------------------------------------*/

static void AuthLock(void)
{
    OS_SPECIFIC_MUTEX_WAIT(mAuthMutex);
}



/*-----------------------------------------------------------------------*/

static void AuthUnlock(void)
{
    OS_SPECIFIC_MUTEX_RELEASE(mAuthMutex);
}



/*-----------------------------------------------------------------------*/

static Boolean VerifyCookie(environment *p_envVar, authInfo *pUser );

static sbyte4 FindClientLocation(ubyte4 IpAddr, environment *p_envVar)     /*!!!!!!!!!!!!!!! IPv6, IPv8 */
{
    int clientIndex;

    for (clientIndex = 0; clientIndex < kHwMaxSimultaneousClients; clientIndex++)
        if (IpAddr == LoggedInClients[clientIndex].ClientIpAddr)    /*!!!!!!!!!!! define as a macro. JAB */
            if (TRUE == VerifyCookie(p_envVar, &LoggedInClients[clientIndex]))
                return clientIndex;

    return ERROR_GENERAL_NOT_FOUND;
}



/*-----------------------------------------------------------------------*/

static sbyte4 FindFreeClientLocation(void)
{
    int clientIndex;

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
       to make the system more sophisticated, we should have a method for logging out 
       a lower access level user, to allow a higher user into the device.  JAB */

    /* To optimize the search time consolidate these searches.  JAB */
    for (clientIndex = 0; clientIndex < kHwMaxSimultaneousClients; clientIndex++)
    {
        if (kIllegal_IpAddr == LoggedInClients[clientIndex].ClientIpAddr)      /*!!!!!!!!! define as a macro. JAB */
            return clientIndex;
    }

    for (clientIndex = 0; clientIndex < kHwMaxSimultaneousClients; clientIndex++)
    {
        if (kHwIdleTimeoutSecs < (OS_SPECIFIC_GET_SECS() - LoggedInClients[clientIndex].LastAccessSecs))
            return clientIndex;
    }

    for (clientIndex = 0; clientIndex < kHwMaxSimultaneousClients; clientIndex++)
    {
        if (FALSE == LoggedInClients[clientIndex].ClientAuthenticated)
            return clientIndex;

        if (NULL == LoggedInClients[clientIndex].pUserName)
            return clientIndex;
    }

    /* we probably want to keep track of these buggers, so that we can lock them out.  JAB */
    for (clientIndex = 0; clientIndex < kHwMaxSimultaneousClients; clientIndex++)
    {
        if (0 < LoggedInClients[clientIndex].ConsecutiveIllegalAttempts)
            return clientIndex;

        if (0 < LoggedInClients[clientIndex].ConsecutiveFailedLogins)
            return clientIndex;
    }

    return ERROR_GENERAL_NOT_FOUND;

} /* FindFreeClientLocation */



/*-----------------------------------------------------------------------*/

static Boolean IdleTimeoutOccurred(sbyte4 clientIndex)
{
    /* if an invalid clientIndex, return TRUE --- idle timeout has occurred */
    if ((0 > clientIndex) || (kHwMaxSimultaneousClients <= clientIndex))
        return TRUE;

    /* has the user been idle for kHwIdleTimeoutSecs seconds? */
    if (kHwIdleTimeoutSecs < (OS_SPECIFIC_GET_SECS() - LoggedInClients[clientIndex].LastAccessSecs))
        return TRUE;

    /* user is still connected */
    return FALSE;
}



/*-----------------------------------------------------------------------*/

static Boolean UserAuthenticated(sbyte4 clientIndex)
{
    return LoggedInClients[clientIndex].ClientAuthenticated;
}



/*-----------------------------------------------------------------------*/

static Boolean ProperAuthRequest(environment *p_envVar, sbyte4 clientIndex)
{
    sbyte Nonce[kMaxNonceLen+1];
    sbyte HtmlDigest[128+1];

    Nonce[kMaxNonceLen]            = '\0';
    HtmlDigest[kDigestMesgBufSize] = '\0';

    if ((OK == CgiLookup(p_envVar, "nonce", Nonce, kMaxNonceLen)) &&
        (OK == CgiLookup(p_envVar, "encoded", HtmlDigest, 128)) )
    {
        sbyte *pNonce = STRCHR(HtmlDigest, ':');

        if (NULL == pNonce)
            return FALSE;

        *pNonce = '\0';
        pNonce++;

        if (NULL != LoggedInClients[clientIndex].pUserName)
        {
            RC_FREE(LoggedInClients[clientIndex].pUserName);
            LoggedInClients[clientIndex].pUserName = NULL;
        }

        if (NULL == (LoggedInClients[clientIndex].pUserName = RC_MALLOC(STRLEN(HtmlDigest)+1)))
            return FALSE;

        STRCPY(LoggedInClients[clientIndex].pUserName, HtmlDigest);
        MEMCPY(LoggedInClients[clientIndex].ReplyDigest, pNonce, kDigestMesgBufSize);

        if (0 == MEMCMP(Nonce, LoggedInClients[clientIndex].SentNonce, kDigestMesgBufSize))
            return TRUE;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------*/

#define MostSignificantHexDigit(x) ((x & 0xf0000000) >> 28)

static sbyte BitsToHex(ubyte4 num)
{
    if (10 > num)
        return '0' + num;
    else
        return 'a' + (num - 10);
}



/*-----------------------------------------------------------------------*/

static void IntToHex8(ubyte4 num, sbyte *pBuf)
{
    sbyte4 i;
    ubyte4 newDigit;

    for(i=0; i<8; i++)
    {
        newDigit = MostSignificantHexDigit(num);
        pBuf[i] = BitsToHex(newDigit);
        num = num << 4;
    }
    
    pBuf[i] = '\0';
}


/*-----------------------------------------------------------------------*/

static void AddNewUserToTable(environment *p_envVar, sbyte4 clientIndex)
{
    LoggedInClients[clientIndex].ClientIpAddr        = p_envVar->IpAddr;
    LoggedInClients[clientIndex].ClientAccessLevel   = 0;
    LoggedInClients[clientIndex].ClientAuthenticated = FALSE;

    if (NULL != LoggedInClients[clientIndex].pUserName)
    {
        RC_FREE(LoggedInClients[clientIndex].pUserName);
        LoggedInClients[clientIndex].pUserName = NULL;
    }

    LoggedInClients[clientIndex].ConsecutiveIllegalAttempts = 0;
    LoggedInClients[clientIndex].ConsecutiveFailedLogins = 0;

    /* !-!-!-!-! Use OS_SPECIFIC_GET_SECS(), since we don't have access to RAND(). JAB */
    LoggedInClients[clientIndex].FirstLoginSecs = OS_SPECIFIC_GET_SECS();
    LoggedInClients[clientIndex].LastAccessSecs = OS_SPECIFIC_GET_SECS();

    IntToHex8(p_envVar->IpAddr, LoggedInClients[clientIndex].SentNonce);
    IntToHex8(OS_SPECIFIC_GET_SECS(), &(LoggedInClients[clientIndex].SentNonce[8]));

}



/*-----------------------------------------------------------------------*/

#define kUsernameLen    16
#define kPasswordLen    16

static int VerifyUser(environment *p_envVar, authInfo *clientInfo, char *pUserName,
                      char *pAuthMessage, char *pNonce, ubyte4 IpAddr)
{
    sbyte Password[kPasswordLen];
    sbyte AssembledNonce[kUsernameLen + 1 + kPasswordLen + 1 + (kDigestMesgBufSize * 2) + 1];
    sbyte DigestedMessage[kDigestMesgBufSize];
    sbyte DigestOutput[128];
    sbyte *pDigestOutput = DigestOutput;
    int   UserAccessLevel = 0;
    sbyte4   k;
    sbyte4   assembledLen = 0;
    /* Lookup password based on username, simulation */
#ifdef CUSTOM_AUTHENTICATION_LEVEL
    if (0 > (UserAccessLevel = CUSTOM_AUTHENTICATION_LEVEL (p_envVar, pUserName, Password)))
        return 0;
#endif

/*    sprintf(AssembledNonce, "%s:%s:%s", pUserName, Password, pNonce); */
    STRCPY(AssembledNonce, pUserName);
    assembledLen += STRLEN(pUserName);
    AssembledNonce[assembledLen++] = ':';
    STRCPY(&AssembledNonce[assembledLen], Password);
    assembledLen += STRLEN(Password);
    AssembledNonce[assembledLen++] = ':';
    STRCPY(&AssembledNonce[assembledLen], pNonce);

    if (OK > MD5_MessageDigest(AssembledNonce, STRLEN(AssembledNonce), DigestedMessage))
        return 0;

    for (k = 0; k < kDigestMesgBufSize; k++)
    {
        *pDigestOutput = BitsToHex((((ubyte4)DigestedMessage[k]) & 0x0f0) >> 4);
        pDigestOutput++;
        *pDigestOutput = BitsToHex((ubyte4)DigestedMessage[k] & 0x0f);
        pDigestOutput++;
        
/*        sprintf(pDigestOutput, "%02x", (((sbyte4)DigestedMessage[k]) & 0xff));
        pDigestOutput++;
        pDigestOutput++; */
    }

    if (kMD5_Match == MD5_MessageCompare(pAuthMessage, DigestOutput))
    {
        clientInfo->ClientAuthenticated = TRUE;

        /* don't want a clear text password floating around on the stack! */
        MEMSET(Password, 0x00, kPasswordLen);
        return UserAccessLevel;
    }

    MEMSET(Password, 0x00, kPasswordLen);
    return 0;
}

/*-----------------------------------------------------------------------*/

static Boolean VerifyCookie(environment *p_envVar, authInfo *pUser )
{
    sbyte *pCookie = pUser->SentNonce;

    sbyte *pSentCookie = ENVIRONMENT_GetEnvironment(p_envVar, kenv_COOKIE);
    
    if (NULL == pSentCookie)
        return FALSE;

    do
    {
        if (NULL == (pSentCookie = STRCHR(pSentCookie, 'a')))
            return FALSE;

        if (0 == STRNCMP(pSentCookie, "auth=", 5))
            break;

        pSentCookie++;
    }
    while ('\0' != *pSentCookie);

    /* quick parsing to get to the actual cookie */
    while (('=' != *pSentCookie ) && ('\0' != *pSentCookie))
        pSentCookie++;

    /* skip the '=' */
    if ('\0' == *pSentCookie)
        return FALSE;

    pSentCookie++;

    if (0 == STRCMP(pCookie, pSentCookie))
    {
        return TRUE;
    }

    return FALSE;
}



/*-----------------------------------------------------------------------*/

static Boolean IsAuthenticationPost(environment *p_envVar)
{
    sbyte *p_reqMethod =
        ENVIRONMENT_GetEnvironment(p_envVar, kenv_REQUEST_METHOD);

    if (NULL == p_reqMethod)
        return FALSE;

    UPCASE(p_reqMethod);

    if (0 == STRCMP("POST", p_reqMethod))
    {
        sbyte Nonce[kMaxNonceLen+1];
        if (OK == CgiLookup(p_envVar, "nonce", Nonce, kMaxNonceLen))
            return TRUE;
        return FALSE;
    }
    else
        return FALSE;
}



/*-----------------------------------------------------------------------*/

extern sbyte4 AUTH_CheckAuthentication(environment *p_envVar)
{
    sbyte4  clientIndex;
    sbyte   *pSentCookie = ENVIRONMENT_GetEnvironment(p_envVar, kenv_COOKIE);

#ifdef __DEBUG_AUTH__
    if (NULL != pSentCookie)
        printf("AUTH_CheckAuthentication: pSentCookie = %s.\n", pSentCookie);
#endif

    AuthLock();

    if (0 > (clientIndex = FindClientLocation(p_envVar->IpAddr, p_envVar)))
    {
        if (0 > (clientIndex = FindFreeClientLocation()))
        {
            AuthUnlock();
            return -500;            /* "500 - Server Too Busy" */
        }

        AddNewUserToTable(p_envVar, clientIndex);
        p_envVar->clientIndex = clientIndex;
        AuthUnlock();

#ifdef __DEBUG_AUTH__
        printf("AUTH_CheckAuthentication: Able to add new user to logon list.\n");
#endif

        return 0;                   /* return lowest access level */
    }

    p_envVar->clientIndex = clientIndex;

    if (TRUE == IdleTimeoutOccurred(clientIndex))
    {
        AddNewUserToTable(p_envVar, clientIndex);

        AuthUnlock();

#ifdef __DEBUG_AUTH__
        printf("AUTH_CheckAuthentication: Idle timeout occurred.\n");
#endif

        return 0;                   /* return lowest access level */
    }

    /* If a user is authenticated, but the cookie we're receiving is NULL, 
     * the user must be logging in again before his original connection 
     * timed out.  So go ahead and create a proper authentication request.
     */

    /* addendum - we need do an additional check to see if it's a new request for validation - i.e. the
     * may have already authenticated but is switching to a higher access level.  In this case, we 
     * have to first check to see if it's a POST request, then check to see if it's a JS authentication POST,
     * then check the auth string in the CGI stream if that's the case.  Otherwise we'd ignore subsequent
     * authentication requests....
     */
#ifdef AUTH_PROXY_BEHAVIOR_FIX
    if (( FALSE == UserAuthenticated(clientIndex)) &&
        ( NULL == pSentCookie ))
#else
    if (( FALSE == UserAuthenticated(clientIndex)) ||
        ( NULL == pSentCookie ) ||
        ( TRUE == IsAuthenticationPost(p_envVar)))
#endif
    {
        if (FALSE == ProperAuthRequest(p_envVar, clientIndex))
        {
            LoggedInClients[clientIndex].ConsecutiveIllegalAttempts++;

            AuthUnlock();

#ifdef __DEBUG_AUTH__
        printf("AUTH_CheckAuthentication: Improper auth request.\n");
#endif

            return 0;                   /* disconnect user */
        }

        /* authenticate the user */
        LoggedInClients[clientIndex].ClientAccessLevel = 
            VerifyUser(p_envVar,
                       &LoggedInClients[clientIndex], 
                       LoggedInClients[clientIndex].pUserName, 
                       LoggedInClients[clientIndex].ReplyDigest, 
                       LoggedInClients[clientIndex].SentNonce, 
                       LoggedInClients[clientIndex].ClientIpAddr);
    }
    else if (FALSE == VerifyCookie(p_envVar, &LoggedInClients[clientIndex]))
    {
        AuthUnlock();

#ifdef __DEBUG_AUTH__
        printf("AUTH_CheckAuthentication: Bad cookie.\n");
#endif

        return 0;
    }

    /* reset the timer for the current entry */
    LoggedInClients[clientIndex].LastAccessSecs = OS_SPECIFIC_GET_SECS();

    AuthUnlock();

#ifdef __DEBUG_AUTH__
        printf("AUTH_CheckAuthentication: Granting access (level = %d).\n", LoggedInClients[clientIndex].ClientAccessLevel);
#endif

    return LoggedInClients[clientIndex].ClientAccessLevel;

} /* AUTH_CheckAuthentication */



/*-----------------------------------------------------------------------*/

extern sbyte *AUTH_GetSentNonce(sbyte4 clientIndex)
{
    return LoggedInClients[clientIndex].SentNonce;
}



/*-----------------------------------------------------------------------*/

extern sbyte *AUTH_GetClientCookie(sbyte4 clientIndex)
{
#ifdef __DEBUG_AUTH__
    printf("AUTH_GetClientCookie: [%s]\n", LoggedInClients[clientIndex].SentNonce);
#endif

    return LoggedInClients[clientIndex].SentNonce;
}



/*-----------------------------------------------------------------------*/

extern void AUTH_Logout(environment *p_envVar)
{
    /* make sure we have a valid index */
    if ((0 <= p_envVar->clientIndex) && (kHwMaxSimultaneousClients > p_envVar->clientIndex))
    {
        /* make sure we have a valid pointer */
        if (NULL != LoggedInClients)
        {
            AuthLock();
            ClearClientRecords(&LoggedInClients[p_envVar->clientIndex]);
            AuthUnlock();
        }
    }
}

#endif /* __JAVASCRIPT_DIGEST_ENABLED__ */

