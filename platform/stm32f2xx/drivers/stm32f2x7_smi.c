

  
/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "os_mutex.h"

/* BSP include */
#include "stm32f2xx.h"
#include "stm32f2x7_eth.h"
#include "stm32f2x7_smi.h"


OS_MUTEX_T mutex_smi; 

/**
  * @brief  Read the switch register
  * @param  phyAddr, regAddr, data
  * @retval 
  *		1: success
  *		0: failed
  */
int smi_getreg(u16 phyAddr, u16 regAddr, u16 *data)
{
	u16 ret;
	
	os_mutex_lock(&mutex_smi, OS_MUTEX_WAIT_FOREVER);
	ret = ETH_ReadPHYRegister2(phyAddr,regAddr,data);
	if(ret == ETH_ERROR) {
		os_mutex_unlock(&mutex_smi);
		return SMI_DRV_FAIL;
	}
	os_mutex_unlock(&mutex_smi);

	return SMI_DRV_SUCCESS;
}

/**
  * @brief  Write the switch register
  * @param  phyAddr, regAddr, data
  * @retval 
  *		1: success
  *		0: failed
  */
int smi_setreg(u16 phyAddr, u16 regAddr, u16 data)
{
	int retVal;
	
	os_mutex_lock(&mutex_smi, OS_MUTEX_WAIT_FOREVER);
	ETH_WritePHYRegister(phyAddr,regAddr,data);
	
	if(ETH_WritePHYRegister(phyAddr,regAddr,data) == ETH_ERROR)
		retVal = SMI_DRV_FAIL;
	else 
		retVal = SMI_DRV_SUCCESS;
	
	os_mutex_unlock(&mutex_smi);

	return SMI_DRV_SUCCESS;
}

/**
  * @brief  reads a specified field from a switch's port phy register.
  * @param  
  *		phyAddr     - The register's address.
  *		fieldOffset - The field start bit index. (0 - 15)
  *		fieldLength - Number of bits to write.
  *		data        - Data to be written.  
  * @retval 
  *		1: success
  *		0: failed
  *	@note: The sum of fieldOffset & fieldLength parameters must be smaller-equal to 16.
  */
int smi_getregfield(u16 phyAddr, u16 regAddr, u16 fieldOffset, u16 fieldLength, u16 *data)
{
	u16 mask, ret;
	u16 tmpData;
	int retVal;

	os_mutex_lock(&mutex_smi, OS_MUTEX_WAIT_FOREVER);
	ret =  ETH_ReadPHYRegister2(phyAddr,regAddr,&tmpData);
	if(ret == ETH_ERROR) {
		os_mutex_unlock(&mutex_smi);
		return SMI_DRV_FAIL;
	}
	os_mutex_unlock(&mutex_smi);
	
	/* Calculate the mask */
	if((fieldLength + fieldOffset) >= 16)
		mask = (0 - (1 << fieldOffset));
	else
		mask = (((1 << (fieldLength + fieldOffset))) - (1 << fieldOffset));
		
	tmpData = (tmpData & mask) >> fieldOffset;
	*data = tmpData;

	return SMI_DRV_SUCCESS;
}

/**
  * @brief  Writes to specified field in a switch's register.
  * @param  
  *		phyAddr     - The register's address.
  *		fieldOffset - The field start bit index. (0 - 15)
  *		fieldLength - Number of bits to write.
  *		data        - Data to be written.  
  * @retval 
  *		1: success
  *		0: failed
  *	@note: The sum of fieldOffset & fieldLength parameters must be smaller-equal to 16.  
  */
  
int smi_setregfield(u16 phyAddr, u16 regAddr, u16 fieldOffset, u16 fieldLength, u16 data)
{
	u16 mask, ret;
	u16 tmpData;
	int retVal;

	os_mutex_lock(&mutex_smi, OS_MUTEX_WAIT_FOREVER);
	ret =  ETH_ReadPHYRegister2(phyAddr,regAddr,&tmpData);
	if(ret == ETH_ERROR) {
		os_mutex_unlock(&mutex_smi);
		return SMI_DRV_FAIL;
	}
	
	/* Calculate the mask */
	if((fieldLength + fieldOffset) >= 16)
		mask = (0 - (1 << fieldOffset));
	else
		mask = (((1 << (fieldLength + fieldOffset))) - (1 << fieldOffset));

	/* Set the desired bits to 0. */
	tmpData &= ~mask;
	/* Set the given data into the above reset bits. */
	tmpData |= ((data << fieldOffset) & mask);

	if(ETH_WritePHYRegister(phyAddr, regAddr,tmpData) == ETH_ERROR)
		retVal = SMI_DRV_FAIL;
	else 
		retVal = SMI_DRV_SUCCESS;

	os_mutex_unlock(&mutex_smi);
	
	return retVal;
}

#if 0
/**
  * @brief  Probe the Switch chip
  * @param  None
  * @retval None
  */
int smi_probe(void)
{
	u16 reg_val=0;

	os_mutex_init(&mutex_smi);
	
	smi_getreg(PHYADDR_PORT(0), 0x03, &reg_val); 
#if BOARD_GE11144MD
	/* Setup PCS control register, force link up, and speed 100M */
	reg_val=0x31;
	smi_setregfield(PHYADDR_PORT(9), 0x01, 0, 6, reg_val);
	return SW_DRV_SUCCESS;
#endif
	if((reg_val & 0xfff0) == 0x0950) {
		return SMI_DRV_SUCCESS;
	} else {
		return SMI_DRV_FAIL;
	}
}
#endif


