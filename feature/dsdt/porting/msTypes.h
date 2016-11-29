#include <Copyright.h>

#ifndef __MS_TYPES_H
#define __MS_TYPES_H

#ifdef _VXWORKS
#include "vxWorks.h"
#include "logLib.h"
#endif
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"

#include "msApi.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define MSG_PRINT(x) printf x
#define MSG_PRINT(x)

#define USE_SEMAPHORE

#ifdef USE_SEMAPHORE
GT_SEM osSemCreate(GT_SEM_BEGIN_STATE state);
GT_STATUS osSemDelete(GT_SEM smid);
GT_STATUS osSemWait(GT_SEM smid, GT_U32 timeOut);
GT_STATUS osSemSignal(GT_SEM smid);
#endif

GT_BOOL obReadMii ( GT_QD_DEV* dev, unsigned int portNumber , unsigned int MIIReg,
                      unsigned int* value);
GT_BOOL obWriteMii ( GT_QD_DEV* dev, unsigned int portNumber , unsigned int MIIReg,
                       unsigned int value);

#ifdef __cplusplus
}
#endif

#endif   /* __pfTesth */

