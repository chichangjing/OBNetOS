/*************************************************************
 * Filename     : sig_srv.h
 * Description  : API for CLI
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/

#ifndef _SIGNAL_SERVER_H
#define _SIGNAL_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/* include -------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "conf_global.h"
/* LwIP includes */
#include "lwip/netif.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    tKinAlarmConfig KinAlarmCfg;
    /* bit0~bit15 is channel 1~16 */
    u16 CurrAlarmStatusFlag;   
    /* bit0~bit15 is channel 1~16 */
    u16 PrevAlarmStatusFlag;    
    /* bit0~bit15 is channel 1~16 
     * If Kin normal jump to alarm status set flag,reset flag */
    u16 Normal2AlarmFlag;                   
    u8 AlarmRspStatus;
    u16 SeqId;
    int sock;
    u16 TxLen;
    u8 *TxBuff;
    u16 RxLen;
    u8 *RxBuff;
    int respTime;
    struct sockaddr_in server;
}tKinAlarmInfo;

typedef struct
{
    u8 BeginId[3];
    u8 ProtoType[2];
    u8 Version;
    u8 FrameType;
    u8 MsgLen[2];
    u8 SeqId[2];
    u8 OpCode;
}tAlarmHead;  

typedef struct
{
    u8 ChanID;
    u8 TimeStamp[6];
}tChanMsg;

typedef struct
{
    u8 RegisterID[4];
    u8 DevicdID[8];
    u8 DevicdMAC[6];
    u8 RecordCount;
}tChanHead;

typedef struct
{
    u16 crc16;
    u8  EndId[3];
}tAlarmTail;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Struct data position */
#define ALARM_ADDR_CHAN_HEAD        sizeof(tAlarmHead)
#define ALARM_ADDR_CHAN_MSG         (sizeof(tAlarmHead) + sizeof(tChanHead))

/* KIN task enable */
#define KIN_ENABLE              0x01                /*任务启动*/


/* KIN response status */
#define KIN_RESP_OK             0x00                /* 成功回应 */
#define KIN_RESP_WAIT           0x01                /* 等待回应 */
#define POLL_TIME               (10)                /* 轮询时间 ms*/
#define RESPOND_TIMEOUT         (3000/POLL_TIME)    /* 3s无应答重传 */

/* Frame type */
#define COMAND_SEND             0X91
#define COMAND_RESPONSE         0X92
#define COMAND_TRAP_SEND        0X93
#define COMAND_TRAP_RESPONE     0X94

/* Function code */
#define FUNCTION_ALARM_KIN      0X03

/* Kin input channel max number */
#define KIN_MAX_CHAN_NUM        0X01
/* Exported functions --------------------------------------------------------*/ 
void SignalTaskInit(void);

void SignalLockInit(void);
void SignalMsgSend(u8 HonuKinStatus);
void SignalMsgRecv(u8 *HonuKinStatus);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SIGNAL_SERVER_H */



