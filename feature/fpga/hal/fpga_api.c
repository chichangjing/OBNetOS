/*************************************************************
 * Filename     : fpga_api.c
 * Description  : API for fpga regester feature 
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/
/* include -------------------------------------------------------------------*/
#include "fpga_api.h"
#include "halsw_i2c.h"
#include "common.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/******************************************************************************
	OPTIN4-QM与GV3S-HONUE-QM寄存器表
    --------------------------------------------------------------------------
    |  寄存器地址 |  属性  |             状态描述                           |  
    --------------------------------------------------------------------------
    | 0x00(1)     |  R     |  B7：光口锁定，1锁定，                         |  
    | ...         |  ...   |  B6：光口中SDI视频锁定，1锁定                  | 
    |             |        |  B5：光口接收SDI中附属音频开启，1开启          | 
    |             |        |  B4：光口接收SDI中附属网络开启，1开启          | 
    | ...         |  ...   |  B3：光口接收SDI中附属数据开启，1开启          | 
    |             |        |  B2：光口接收SDI中附属网管开启，1开启          | 
    |             |        |  B1：保留                                      | 
    | 0x0F        |  R     |  B0：保留                                      | 
    --------------------------------------------------------------------------
    | 0x10        |        |  B7：发给光口的SDI测试彩条，1有效              | 
    |             |  W     |                                                | 
    | 0x0F        |        |                                                | 
    --------------------------------------------------------------------------
    | ...         |        |                                                | 
    | ...         |        |                                                | 
    --------------------------------------------------------------------------
    | 0xF0        |  R     |  输入电口16-9状态，1有效，B7为第16视频电口     | 
    --------------------------------------------------------------------------
    | 0xF1        |  R     |  输入电口8-1状态，1有效，B7为第8视频电口       | 
    --------------------------------------------------------------------------
    | 0XF2        |  W     |  输出背板彩条设置，1有效，第16-8路，B7为第16路 | 
    --------------------------------------------------------------------------
    | 0XF3        |  W     |  输出背板彩条设置，1有效，第16-8路，B7为第16路 | 
    --------------------------------------------------------------------------
    | 0XF4        |  R     |  背板输入电口16-9状态，1有效，B7为第16电口     | 
    --------------------------------------------------------------------------
    | 0xF5        |  R     |  背板输入电口8-1状态，1有效，B7为第8电口       | 
    --------------------------------------------------------------------------
    | ...         |  R     |                                                | 
    --------------------------------------------------------------------------
    | 0xFA        |  R     |  B7：K信号输入，1表示K信号断开  B6-B0：保留    | 
    --------------------------------------------------------------------------
    | 0xFB        |  R     |  FPGA代码ID字节1，AC(2)                        | 
    --------------------------------------------------------------------------
    | 0xFC        |  R     |  FPGA代码ID字节2，BE                           | 
    --------------------------------------------------------------------------
    | 0xFD        |  R     |  FPGA代码ID字节3，DF                           | 
    --------------------------------------------------------------------------
    | 0xFE        |  R     |  FPGA代码ID字节4（代码版本），HF               | 
    --------------------------------------------------------------------------
    | 0xFF        |  W     |  机箱拨码                                      | 
    --------------------------------------------------------------------------
    | ...         |        |                                                | 
    --------------------------------------------------------------------------
    注：(1)地址低4位表示光口号，现有系统最大为16光
        (2)FPGA ID号为A.BC.DEF.HF
*******************************************************************************/
/**
  * @brief  Get sdi data status valid
  * @param  sdiport - SDI port number(between 1 and 16)，but GV3S-HONUE-QM only 1 port
  *         state - FPGA_TURE for valid，FPGA_FALSE for invalid，
  * @retval 1 if false,0 if true
  */
int fpga_get_sdi_status(int sdiport, FPGA_BOOL *state)
{
    int status;
    short reg = 0;
    char regtmp = 0;
    
    status = SWI2cSequentialRead(I2CDEV_0, FPGAADD1, FPGA_SDI_DATA_ADDR0, (unsigned char *)&regtmp, 1);
    if(status != 0)
        return 1;
    
    reg = regtmp << 8;
    
    status = SWI2cSequentialRead(I2CDEV_0, FPGAADD1, FPGA_SDI_DATA_ADDR1, (unsigned char *)&regtmp, 1);
    if(status != 0)
        return 1;
    
    reg = reg | (regtmp & 0xff);
    
    *state =(FPGA_BOOL)((reg >> (sdiport-1)) & 0x0001);
       
    return 0;
}

/**
  * @brief  Get OPT port lock or unlock
  * @param  optport - OPT port number(between 1 and 16)，but GV3S-HONUE-QM only 1 port
  *         state - FPGA_TURE for lock，FPGA_FALSE for unlock，
  * @retval 1 if false,0 if true
  */
int fpga_get_opt_lock(int optport, FPGA_BOOL *state)
{
    int status;
    char regtmp = 0;
    
    status = SWI2cSequentialRead(I2CDEV_0, FPGAADD1, optport-1, (unsigned char *)&regtmp, 1);
    if(status != 0)
        return 1;
    
    *state = (regtmp>>7) & 0x01;
    
    return 0;
}

/**
  * @brief  Get k signal state
  * @param  state - FPGA_TURE for OPEN，FPGA_FALSE for CLOSE， 
  * @retval 1 if false,0 if true
  */
int fpga_get_k_signal(FPGA_BOOL *state)
{
    int status;
    char regtmp = 0;
    
    status = SWI2cSequentialRead(I2CDEV_0, FPGAADD1, FPGA_K_SIGNAL_ADDR, (unsigned char *)&regtmp, 1);
    if(status != 0)
        return 1;
    
    *state = (regtmp>>7) & 0x01;
    
    return 0;
}