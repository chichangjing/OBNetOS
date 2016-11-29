/*************************************************************
 * Filename     : halcmd_msg.h
 * Description  : API for fpga command message I/O  
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/

#ifndef _HALCMD_MSG_H
#define _HALCMD_MSG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/* include -------------------------------------------------------------------*/
#include "stdint.h"
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/* Exported types ------------------------------------------------------------*/
#define HAL_CMD_MSG_LEN 30
    
typedef struct HalCmdMsg
{
   uint16_t len;
   uint8_t  buff[HAL_CMD_MSG_LEN];
}StHalCmdMsg;

typedef struct HalCmdQueueMsg
{
    uint16_t len;
    xQueueHandle QueueMsg;
}StHalCmdQueueMsg;

/* Exported constants --------------------------------------------------------*/
    
/* Exported macro ------------------------------------------------------------*/
    
/* Exported functions --------------------------------------------------------*/ 
void HalQueueInit(void);
xQueueHandle HalQueueCreat(StHalCmdQueueMsg *HalCmdQueueMsg);
portBASE_TYPE HalQueueWrite(StHalCmdMsg *CmdMsg, portBASE_TYPE xTicksToWait);
uint16_t HalQueueRead(int8_t *buff, portBASE_TYPE xTicksToWait);
void HalWrite(int8_t *buff, uint16_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* _HALCMD_MSG_H */