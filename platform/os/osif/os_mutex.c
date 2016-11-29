
#include "os_mutex.h"

/*
 *  Description : This routine initializes mutex. 
 *  Parameters  : m - mutex pointer 
 *  Return value:
 *    OS_MUTEX_SUCCESS
 *	  OS_MUTEX_ERROR
 */
OS_MUTEX_STATUS os_mutex_init(OS_MUTEX_T *m)
{
#if defined(OS_FREERTOS)

#if 1
    int ret;
	xSemaphoreHandle xSemaphore;

	vSemaphoreCreateBinary(xSemaphore);
	*m = xSemaphore;
	if(*m == NULL) {
		ret = OS_MUTEX_ERROR;
	} else {
		ret = OS_MUTEX_SUCCESS;
	}	

	return ret;
#else	
	int ret;

    *m = (OS_MUTEX_T)xSemaphoreCreateMutex();
    if(*m != NULL) {
		//xSemaphoreGive(*m);
        ret = OS_MUTEX_SUCCESS;
    } else
        ret = OS_MUTEX_ERROR;

    return ret;
#endif
#else
	return OS_MUTEX_SUCCESS;
#endif   
}

/*
 *  Description : This routine lock a specific mutex. 
 *  Parameters  : m - mutex pointer 
 *  Return value:
 *    OS_MUTEX_SUCCESS
 *	  OS_MUTEX_ERROR
 */
OS_MUTEX_STATUS os_mutex_lock(OS_MUTEX_T *m, OS_MUTEX_WAIT timeout)
{
#if defined(OS_FREERTOS)
	if(*m == NULL) {
		return OS_MUTEX_ERROR;
	}
	
	if(timeout != OS_MUTEX_WAIT_FOREVER) {
		if(xSemaphoreTake(*m, timeout/portTICK_RATE_MS) == pdTRUE)
			return OS_MUTEX_SUCCESS;
		else
			return OS_MUTEX_ERROR_TIMEOUT;
	} else {
		while(xSemaphoreTake(*m, portMAX_DELAY) != pdTRUE) {}
		return OS_MUTEX_SUCCESS;
	}
	
#else
	return OS_MUTEX_SUCCESS;
#endif   
}

/*
 *  Description : This routine unlock a specific mutex. 
 *  Parameters  : m - mutex pointer 
 *  Return value:
 *    OS_MUTEX_SUCCESS
 *	  OS_MUTEX_ERROR
 */
OS_MUTEX_STATUS os_mutex_unlock(OS_MUTEX_T *m)
{
#if defined(OS_FREERTOS)
	if(*m == NULL) {
		return OS_MUTEX_ERROR;
	}
	xSemaphoreGive(*m);
	return OS_MUTEX_SUCCESS;
#else
	return OS_MUTEX_SUCCESS;
#endif   
}


