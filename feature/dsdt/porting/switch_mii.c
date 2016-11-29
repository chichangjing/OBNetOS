
#include "mconfig.h"
#if MARVELL_SWITCH

#include <Copyright.h>

#include <msApiDefs.h>
#include <stdint.h>
#include "os_mutex.h"

extern uint16_t ETH_ReadPHYRegister2(uint16_t PHYAddress, uint16_t PHYReg, uint16_t *RegValue);
extern uint32_t ETH_WritePHYRegister(uint16_t PHYAddress, uint16_t PHYReg, uint16_t PHYValue);
extern OS_MUTEX_T mutex_smi; 

GT_BOOL obReadMii (GT_QD_DEV* dev, unsigned int portNumber , unsigned int MIIReg, unsigned int *value)
{
	uint16_t ret;
	uint16_t reg_val;
	
	os_mutex_lock(&mutex_smi, OS_MUTEX_WAIT_FOREVER);
	if((ret = ETH_ReadPHYRegister2((uint16_t)portNumber, (uint16_t)MIIReg, &reg_val)) == 0) {
		*value = 0xffff;
		os_mutex_unlock(&mutex_smi);
		return GT_FALSE;
	} 
	*value = reg_val;
	os_mutex_unlock(&mutex_smi);

	return GT_TRUE;
}


GT_BOOL obWriteMii (GT_QD_DEV* dev, unsigned int portNumber , unsigned int MIIReg, unsigned int value)
{
	uint32_t ret;
	GT_BOOL retVal;

	os_mutex_lock(&mutex_smi, OS_MUTEX_WAIT_FOREVER);
	
	if((ret = ETH_WritePHYRegister((uint16_t)portNumber, (uint16_t)MIIReg, (uint16_t)value)) == 0) {
		retVal = GT_FALSE;
	} else { 
		retVal = GT_TRUE;
	}
	os_mutex_unlock(&mutex_smi);

	return retVal;
}

#endif
