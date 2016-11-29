
#include "rc.h"

#ifdef __RCC_ENABLED__
#include "rcc.h"
#include "rcc_user.h"

#ifndef RL_STATIC
#define RL_STATIC static
#endif

/* passwords should be stored in encrypted form */ 
#ifndef RCC_ENCRYPT_FUNC
# define RCC_ENCRYPT_FUNC(password, salt)     password
#endif


extern DTTypeInfo    mAccessInfo;

userInfo userBase[kRCC_MAX_USERS] =
{
    {kRCC_DEFAULT_LOGIN,  kRCC_DEFAULT_PASSWORD, 10},
    {"guest",   "guest",    0}
};

/*-----------------------------------------------------------------------*/

RL_STATIC void USER_GetSalt(sbyte salt[2])
{
    sbyte   buffer[32];
    ubyte4  timeNow = OS_SPECIFIC_GET_SECS();
    sbyte4  len;
    sbyte4  offset;
    sbyte4  seed = 0;

    ASCTIME(timeNow, buffer);
    len = STRLEN(buffer) - 1;

    offset = timeNow % len;
    while (1)
    {
        seed = buffer[offset];
        if (ISALPHANUMERIC(seed))
            break;
        offset = ++offset % len;
    }
    salt[0] = (sbyte) seed;

    offset = seed % len;
    while (1)
    {
        seed = buffer[offset];
        if (ISALPHANUMERIC(seed))
            break;
        offset = ++offset % len;
    }

    salt[1] = (sbyte) seed;
}

/*-----------------------------------------------------------------------*/

RL_STATIC userInfo * USER_GetUser(sbyte *pLogin)
{
    sbyte2      index;
    userInfo   *pUser = userBase;

    for (index = 0; index < kRCC_MAX_USERS; index++, pUser++)
    {
        if (0 == STRCMP(pUser->login, pLogin))
            return pUser;
    }
        
    return NULL;
}


/*-----------------------------------------------------------------------*/

RL_STATIC userInfo * USER_GetPasswordOwner(sbyte *pPassword)
{
    sbyte2      index;
    sbyte       salt[2];
    sbyte      *pEncrypted = NULL;
    userInfo   *pUser      = userBase;

    USER_GetSalt(salt);
    pEncrypted = RCC_ENCRYPT_FUNC(pPassword, salt);

    for (index = 0; index < kRCC_MAX_USERS; index++, pUser++)
    {
        if (NULL_STRING(pUser->password))
            continue;

        if (0 == STRCMP(pUser->password, pEncrypted))
            return pUser;
    }
        
    return NULL;
}

/*-----------------------------------------------------------------------*/

RL_STATIC userInfo * USER_NewUser(void)
{
    sbyte2      index;
    userInfo   *pUser = userBase;

    for (index = 0; index < kRCC_MAX_USERS; index++, pUser++)
    {
        if (NULL_STRING(pUser->login))
            return pUser;
    }
        
    return NULL;
}


/*-----------------------------------------------------------------------*/

extern Boolean 
RCC_USER_ValidateUser(cli_env *pCliEnv, sbyte *pLogin, sbyte *pPassword, Access *pAccess)
{
    sbyte     *pEncrypted = NULL;
    sbyte      salt[2];
    userInfo  *pUser;

#ifdef __USE_PASSWORD_ONLY__
    if (NULL != (pUser = USER_GetPasswordOwner(pPassword)))
    {
        *pAccess = pUser->accessLevel;
        return TRUE;
    }
    else
    {
        *pAccess = 0;
        return FALSE;
    }
#endif

    if (NULL == (pUser = USER_GetUser(pLogin)))
        return FALSE;

    *pAccess = pUser->accessLevel;
    USER_GetSalt(salt);
    pEncrypted = RCC_ENCRYPT_FUNC(pPassword, salt);
    return (0 == STRCMP(pUser->password, pEncrypted));
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_USER_AddUser(cli_env *pCliEnv, sbyte *pLogin, sbyte *pAccess)
{
    Access      accessLevel;
    userInfo  *pUser = USER_GetUser(pLogin);

    /* check for pre-existing account */
    if (NULL != pUser)
    {
        RCC_EXT_WriteStr(pCliEnv, "User: ");
        RCC_EXT_WriteStr(pCliEnv, pLogin);
        RCC_EXT_WriteStrLine(pCliEnv, " already exists.");
        return RCC_ERROR;
    }

    if (NULL == pAccess)
        pAccess = kRCC_DEFAULT_ACCESS;

    if (OK != CONVERT_StrTypeTo(pAccess, &accessLevel, &mAccessInfo))
    {
        RCC_EXT_WriteStr(pCliEnv, "Error: Access level ");
        RCC_EXT_WriteStr(pCliEnv, pAccess);
        RCC_EXT_WriteStrLine(pCliEnv, " is not valid.");
        return RCC_ERROR;
    }

    if (NULL == (pUser = USER_NewUser()))
    {
        RCC_EXT_WriteStrLine(pCliEnv, "User database is full");
        return RCC_ERROR;
    }

    STRCPY(pUser->login, pLogin);
    pUser->accessLevel = accessLevel;
    RCC_USER_ChangePassword(pCliEnv, pLogin);

    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS RCC_USER_DeleteUser(cli_env *pCliEnv, sbyte *pLogin)
{
    userInfo  *pUser = USER_GetUser(pLogin);

    if (NULL == pUser)
        return ERROR_RCC_INVALID_USER;

    MEMSET(pUser->login, 0, sizeof(pUser->login));
    MEMSET(pUser->password, 0, sizeof(pUser->password));
    pUser->accessLevel = 0;

    return OK;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_USER_ChangePassword(cli_env *pCliEnv, sbyte *pLogin)
{
    sbyte      password1[32];
    sbyte      password2[32];
    sbyte      seed[2];
    sbyte4     byteCount;
    userInfo  *pUser = USER_GetUser(pLogin);

    if (NULL == pUser)
        return ERROR_RCC_INVALID_USER;

    RCC_EXT_WriteStr(pCliEnv, "Enter new password:");
    RCC_EXT_Gets(pCliEnv, password1, 32, &byteCount, FALSE);
    RCC_EXT_WriteStrLine(pCliEnv, "");

    RCC_EXT_WriteStr(pCliEnv, "Confirm new password:");
    RCC_EXT_Gets(pCliEnv, password2, 32, &byteCount, FALSE);
    RCC_EXT_WriteStrLine(pCliEnv, "");

    if (0 != STRCMP(password1, password2))
        RCC_EXT_WriteStrLine(pCliEnv, "Passwords did not match!");
    else
    {
        USER_GetSalt(seed);
        STRCPY(pUser->password,RCC_ENCRYPT_FUNC(password1, seed));
    }

    return OK;
}


/*-----------------------------------------------------------------------*/

extern RLSTATUS 
RCC_USER_ChangeAccess(cli_env *pCliEnv, sbyte *pLogin, sbyte *pAccess)
{
    userInfo  *pUser = USER_GetUser(pLogin);
    Access    accessLevel;
    RLSTATUS  status;

    if (NULL == pUser)
        return ERROR_RCC_INVALID_USER;

    status = CONVERT_StrTypeTo(pAccess, &accessLevel, &mAccessInfo);
    if (OK != status)
        return status;

    pUser->accessLevel = accessLevel;
    
    return OK;
}


/*-----------------------------------------------------------------------*/

/* for debugging */

extern void
RCC_USER_ShowUsers(cli_env *pCliEnv)
{
    sbyte4      index;
    sbyte       access[30];
    userInfo   *pUser = userBase;

    RCC_EXT_PrintString(pCliEnv, "User",   30, ' ');
    RCC_EXT_Write(pCliEnv, " ", 1);
    RCC_EXT_PrintString(pCliEnv, "Access", 30, ' ');
    RCC_EXT_WriteStrLine(pCliEnv, "");

    RCC_EXT_PrintString(pCliEnv, "",   30, '-');
    RCC_EXT_Write(pCliEnv, " ", 1);
    RCC_EXT_PrintString(pCliEnv, "",   30, '-');
    RCC_EXT_WriteStrLine(pCliEnv, "");
    
    for (index = 0; index < kRCC_MAX_USERS; index++, pUser++)
    {
        if (NULL_STRING(pUser->login))
            continue;

        CONVERT_AccessToStr(pUser->accessLevel, access, &mAccessInfo);

        RCC_EXT_PrintString(pCliEnv, pUser->login, 30, ' ');
        RCC_EXT_Write(pCliEnv, " ", 1);
        RCC_EXT_PrintString(pCliEnv, access,       30, ' ');
        RCC_EXT_WriteStrLine(pCliEnv, "");
    }
        
}

#endif /*  __RCC_ENABLED__ */

