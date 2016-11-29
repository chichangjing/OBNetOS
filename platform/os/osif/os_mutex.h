

#ifndef _OS_MUTEX_H_
#define _OS_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif


/* include OS header files */
#if defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "semphr.h"
#endif   


/* typedef OS_MUTEX_T, this type will wrapper the actual OS mutex type */
#if defined(OS_FREERTOS)
typedef xSemaphoreHandle OS_MUTEX_T;
typedef	portTickType OS_MUTEX_WAIT;
#define OS_MUTEX_WAIT_FOREVER portMAX_DELAY
#define OS_MUTEX_NO_WAIT 0
#else
typedef int OS_MUTEX_T;
typedef	int OS_MUTEX_WAIT;
#define OS_MUTEX_WAIT_FOREVER 0xffffffff
#define OS_MUTEX_NO_WAIT 0
#endif   

enum {
    OS_MUTEX_SUCCESS = 0,
    OS_MUTEX_ERROR,
	OS_MUTEX_ERROR_TIMEOUT    
};

typedef int OS_MUTEX_STATUS;

OS_MUTEX_STATUS os_mutex_init(OS_MUTEX_T *m);
OS_MUTEX_STATUS os_mutex_lock(OS_MUTEX_T *m, OS_MUTEX_WAIT timeout);
OS_MUTEX_STATUS os_mutex_unlock(OS_MUTEX_T *m);

#ifdef __cplusplus
}
#endif

#endif /* _OS_MUTEX_H_ */




