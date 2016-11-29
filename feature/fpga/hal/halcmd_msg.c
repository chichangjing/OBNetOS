/*************************************************************
 * Filename     :  
 * Description  :  
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/
/* include -------------------------------------------------------------------*/
#include "halcmd_msg.h"
#include "console.h"
#include "stdio.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static StHalCmdQueueMsg HalCmdQueueMsg;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the queue.
  * @param  None
  * @retval None
  */
void HalQueueInit(void)
{
	HalCmdQueueMsg.QueueMsg = NULL;
	HalCmdQueueMsg.len = 3;

	if(HalQueueCreat(&HalCmdQueueMsg) == NULL)
    {
		printf("creat HalCmdQueueMsg queue failed \r\n");
    }
}

/**
  * @brief  Creat the queue.
  * @param  HalCmdQueueMsg
  * @retval Queue handler
  */
static xQueueHandle HalQueueCreat(StHalCmdQueueMsg *HalCmdQueueMsg)
{
    HalCmdQueueMsg->QueueMsg = xQueueCreate(HalCmdQueueMsg->len, (unsigned portBASE_TYPE)sizeof(StHalCmdMsg));
    
    return HalCmdQueueMsg->QueueMsg;
}

/**
  * @brief  Receice  messege from queue .
  * @param  @HalCmdQueueMsg : quece handler
  *         @buff           : receive messege buff
  *         @xTicksToWait   : receive waiting time(ms)
  * @retval messege length if true,0 if false
  */
static uint16_t HalQueueReceive(StHalCmdQueueMsg *HalCmdQueueMsg, int8_t *buff, portBASE_TYPE xTicksToWait)
{
	StHalCmdMsg CmdMsg; 

    if(xQueueReceive(HalCmdQueueMsg->QueueMsg,&CmdMsg,xTicksToWait) == errQUEUE_FULL)
		return  0;
    
    for(int i=0; i<CmdMsg.len; i++)
        buff[i] = CmdMsg.buff[i];

	return CmdMsg.len;
}

/**
  * @brief  read messege from queue .
  * @param  @buff           : receive messege buff
  *         @xTicksToWait   : receive waiting time(ms)
  * @retval messege length if true,0 if false
  */
uint16_t HalQueueRead(int8_t *buff, portBASE_TYPE xTicksToWait)
{
	return HalQueueReceive(&HalCmdQueueMsg, buff, xTicksToWait);
}

/**
  * @brief  Send messege to queue .
  * @param  @HalCmdQueueMsg : quece handler
  *         @StHalCmdMsg    : send messege format
  *         @xTicksToWait   : send waiting time(ms)
  * @retval pdPASS if true,errQUEUE_FULL if false
  */
static portBASE_TYPE HalQueueSend(StHalCmdQueueMsg *HalCmdQueueMsg, StHalCmdMsg *CmdMsg, portBASE_TYPE xTicksToWait)
{
	return xQueueSendToBack(HalCmdQueueMsg->QueueMsg,CmdMsg,xTicksToWait);
}

/**
  * @brief  Write messege to queue .
  * @param  @StHalCmdMsg    : write messege format
  *         @xTicksToWait   : write waiting time(ms)
  * @retval pdPASS if true,errQUEUE_FULL if false
  */
portBASE_TYPE HalQueueWrite(StHalCmdMsg *CmdMsg, portBASE_TYPE xTicksToWait)
{
	portBASE_TYPE xStatus;

	xStatus = HalQueueSend(&HalCmdQueueMsg, CmdMsg, xTicksToWait);

	if(xStatus != pdPASS)
		{
			printf("Could not send to the queue \r\n");
		}
	
	return xStatus;
}

/**
  * @brief  Write to uart3.
  * @param  none
  * @retval none
  */
void HalWrite(int8_t *buff, uint16_t len)
{
    for(uint16_t i=0; i<len; i++)
        ConsolePutChar(buff[i]);
}

