/*************************************************************
 * Filename     : sig_srv.c
 * Description  : switch signal server
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/
/* include -------------------------------------------------------------------*/
#include "sig_srv.h"
#include "conf_signal.h"
#include "conf_comm.h"

/* stadard library. */
#include "string.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "os_mutex.h"

/* LwIP includes */
#include "lwip/netif.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"

/* BSP includes */
#include "stm32f2xx_gpio.h"
#include "stm32f2xx.h"
#include "misc_drv.h"
#include "fpga_api.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SIGNAL_TX_BUFF_LEN  128
#define SIGNAL_RX_BUFF_LEN  128
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

tKinAlarmInfo KinAlarmInfo;
tKinAlarmInfo *pKinAlarmInfo = (tKinAlarmInfo *)&KinAlarmInfo;
u8 txbuff[SIGNAL_TX_BUFF_LEN];
u8 rxbuff[SIGNAL_RX_BUFF_LEN];

OS_MUTEX_T kin_mutex;
u8 KinStatus = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void SignalLockInit(void)
{
    os_mutex_init(&kin_mutex);
}

void SignalMsgSend(u8 HonuKinStatus)
{
    os_mutex_lock(&kin_mutex, OS_MUTEX_WAIT_FOREVER);
    KinStatus = HonuKinStatus;
    os_mutex_unlock(&kin_mutex); 
}

void SignalMsgRecv(u8 *HonuKinStatus)
{
    os_mutex_lock(&kin_mutex, OS_MUTEX_WAIT_FOREVER);
    *HonuKinStatus = KinStatus;
    os_mutex_unlock(&kin_mutex); 
}

int SignalCreatSocket(tKinAlarmInfo *KinAlarmInfo)
{
    u8 workmode;
    
    workmode = KinAlarmInfo->KinAlarmCfg.WorkMode;
    switch(workmode)
    {
        case WORKMODE_UDP:
            if((KinAlarmInfo->sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) {
		    return 1;
            }
            break;
        default:
            return 1;
	}
    
    return 0;
}

void SignalAlarmInit(tKinAlarmInfo *KinAlarmInfo)
{   
    u16 serverport;
    u32 serverip;
    
#if BOARD_GE22103MA
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIOG's AHB interface clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    /* Initialize channel input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
#endif /* BOARD_GE22103MA */
    if(signal_cfg_fetch((tKinAlarmConfig *)&(KinAlarmInfo->KinAlarmCfg)) != CONF_ERR_NONE)
    {
        printf("Error: Get Kin alarm configration failed \r\n");
		vTaskDelete( NULL );
        return;
    }

    if(KinAlarmInfo->KinAlarmCfg.FeatureEnable != KIN_ENABLE)
    {
		vTaskDelete( NULL );
        return;
    }
    
    if(KinAlarmInfo->KinAlarmCfg.ChanConfig.ChanNum > KIN_MAX_CHAN_NUM)
    {
        printf("Error: Get Kin channel number failed \r\n");
		vTaskDelete( NULL );
        return;
    }

    /* Initialize configration */
    KinAlarmInfo->CurrAlarmStatusFlag = 0;
    KinAlarmInfo->PrevAlarmStatusFlag = 0;
    KinAlarmInfo->AlarmRspStatus = 0;
    KinAlarmInfo->RxBuff = rxbuff;
    KinAlarmInfo->TxBuff = txbuff;

    /* Creat socket */
    if(SignalCreatSocket(KinAlarmInfo) == 1) {
		printf("Error: Create socket failed \r\n");
		vTaskDelete( NULL );
		return;
	}
    
    memset(&KinAlarmInfo->server, 0, sizeof(KinAlarmInfo->server));
    serverip = *(u32 *)KinAlarmInfo->KinAlarmCfg.ModeConfig.UdpParam.ServerIP;
    serverport = *(u16_t *)KinAlarmInfo->KinAlarmCfg.ModeConfig.UdpParam.ServerPort;
    KinAlarmInfo->server.sin_addr.s_addr = serverip;
    KinAlarmInfo->server.sin_port = serverport;
    KinAlarmInfo->server.sin_family = AF_INET;

}

void ChanGetStatus(u8 chanid, u8 *bitstatus)
{
    u8 state;
    u8 HonuKinStatus;
    
    switch(chanid)
    {
        case 1:
#if BOARD_GE22103MA
            *bitstatus = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11);
#elif BOARD_GV3S_HONUE_QM
            fpga_get_k_signal((FPGA_BOOL *)&state);
            *bitstatus = state;
#elif BOARD_GE1040PU
            SignalMsgRecv(&HonuKinStatus);
			*bitstatus = HonuKinStatus;   
#endif /* BOARD_GE22103MA */
            break;
        default:
            break;
    }
}

void SignalAlarmGetStatus(tKinAlarmInfo *KinAlarmInfo)
{
    u8 ChanNum;
    u8 bitstatus;
    u8 AlarmType;

    ChanNum = KinAlarmInfo->KinAlarmCfg.ChanConfig.ChanNum;
    for(u8 chanid=1; chanid<=ChanNum; chanid++)
    {
        AlarmType = CHAN_GET_ALARM_TYPE(KinAlarmInfo->KinAlarmCfg.ChanConfig.Data[chanid-1]);
        switch(AlarmType)
        {
            case ALARMTYPE_CLOSE:
                ChanGetStatus(chanid, &bitstatus);
                if(bitstatus == Bit_RESET){
                    KinAlarmInfo->CurrAlarmStatusFlag |= (0x0001 << (chanid-1));
                }else{
                    KinAlarmInfo->CurrAlarmStatusFlag &= (0xfffe << (chanid-1));
                }
                break;
            case ALARMTYPE_OPEN:
                ChanGetStatus(chanid, &bitstatus);
                if(bitstatus == Bit_SET){
                    KinAlarmInfo->CurrAlarmStatusFlag |= (0x0001 << (chanid-1));
                }else{
                    KinAlarmInfo->CurrAlarmStatusFlag &= (0xfffe << (chanid-1));
                }
                break;
            case ALARMTYPE_DOWN: 
                ChanGetStatus(chanid, &bitstatus);
                if(bitstatus == Bit_SET){
                    KinAlarmInfo->CurrAlarmStatusFlag |= (0x0001 << (chanid-1));
                }else{
                    KinAlarmInfo->CurrAlarmStatusFlag &= (0xfffe << (chanid-1));
                }
                break;
            case ALARMTYPE_UNKNOW:
                break;
        }

    }
    
    KinAlarmInfo->Normal2AlarmFlag = KinAlarmInfo->CurrAlarmStatusFlag
                                     & (KinAlarmInfo->CurrAlarmStatusFlag
                                     ^KinAlarmInfo->PrevAlarmStatusFlag);

    KinAlarmInfo->PrevAlarmStatusFlag = KinAlarmInfo->CurrAlarmStatusFlag;
}

void PrepareAlarmHead(tKinAlarmInfo *KinAlarmInfo)
{
    struct netif *netif = netif_list;
    tAlarmHead *pAlarmHead = (tAlarmHead *)(KinAlarmInfo->TxBuff);
    tChanHead *pChanHead = (tChanHead *)(KinAlarmInfo->TxBuff + ALARM_ADDR_CHAN_HEAD);

    pAlarmHead->BeginId[0] = 'O';
    pAlarmHead->BeginId[1] = 'B';
    pAlarmHead->BeginId[2] = '*';
    pAlarmHead->ProtoType[0] = 0xB0;
    pAlarmHead->ProtoType[1] = 0x01;
    pAlarmHead->Version = 0x01;
    pAlarmHead->FrameType = 0x93;
    pAlarmHead->MsgLen[0] = 0x00;
    pAlarmHead->MsgLen[1] = 0x00;
    pAlarmHead->SeqId[0] = 0x00;
    pAlarmHead->SeqId[1] = 0x00;
    pAlarmHead->OpCode = FUNCTION_ALARM_KIN;

    pChanHead->RegisterID[0] = 0x00;
    pChanHead->RegisterID[0] = 0x00;
    pChanHead->RegisterID[0] = 0x00;
    pChanHead->RegisterID[0] = 0x00;
    
    pChanHead->DevicdID[0] = (u8)((BOARD_TYPE & 0xFF000000) >> 24);
	pChanHead->DevicdID[1] = (u8)((BOARD_TYPE & 0x00FF0000) >> 16);
	pChanHead->DevicdID[2] = (u8)((BOARD_TYPE & 0x0000FF00) >> 8);
	pChanHead->DevicdID[3] = (u8)(BOARD_TYPE & 0x000000FF);
    pChanHead->DevicdID[4] = 0xFF;
    pChanHead->DevicdID[5] = 0xFF;
    pChanHead->DevicdID[6] = 0xFF;
    pChanHead->DevicdID[7] = 0xFF;  
     /* ifPhysAddress  */
    memcpy((char *)pChanHead->DevicdMAC,(char *)&netif->hwaddr[0],NETIF_MAX_HWADDR_LEN);
    
    pChanHead->RecordCount = 0x00;
    
    KinAlarmInfo->TxLen = sizeof(tAlarmHead) + sizeof(tChanHead);
}

void SignalAlarmSend(tKinAlarmInfo *KinAlarmInfo)
{
    u16 len = 0;
    u16 msglen = 0;
    u8 ChanNum = 0;
    u8 RecordCount = 0;
    tAlarmHead *pTxAlarmHead = (tAlarmHead *)KinAlarmInfo->TxBuff;
    tChanMsg *pTxChanMsg = (tChanMsg *)(KinAlarmInfo->TxBuff + ALARM_ADDR_CHAN_MSG);
    tChanHead *pTxChanHead = (tChanHead *)(KinAlarmInfo->TxBuff + ALARM_ADDR_CHAN_HEAD);

    len = KinAlarmInfo->TxLen;
    ChanNum = KinAlarmInfo->KinAlarmCfg.ChanConfig.ChanNum;
    
    /* determine current alarm status flag */
    if(KinAlarmInfo->CurrAlarmStatusFlag > 0){
        /* determine current seqid whether add */
        if(KinAlarmInfo->Normal2AlarmFlag > 0){
            pTxChanHead->RecordCount = 0;
            for(u8 chanid=1; chanid<=ChanNum; chanid++)
            {
                if(KinAlarmInfo->Normal2AlarmFlag & 1<<(chanid - 1))
                {
                    pTxChanHead->RecordCount++;
                    pTxChanMsg[chanid-1].ChanID = chanid;
                    /* @todo: timestamp */
                }
            }
    
            *(u16 *)(pTxAlarmHead->SeqId) = htons((KinAlarmInfo->SeqId)++);
            len = len + (pTxChanHead->RecordCount)*sizeof(tChanMsg) + sizeof(tAlarmTail);
            msglen = sizeof(tChanHead) + (pTxChanHead->RecordCount)*sizeof(tChanMsg);
            pTxAlarmHead->MsgLen[0] = (msglen>>8) & 0x00FF;
            pTxAlarmHead->MsgLen[1] = msglen & 0x00FF;
            /* Send alarm message */
            if(lwip_sendto(KinAlarmInfo->sock, KinAlarmInfo->TxBuff, len, 0, 
                            (struct sockaddr *)&(KinAlarmInfo->server), 
                            sizeof(KinAlarmInfo->server)) != len){
                printf("Error: lwip send signal failed \r\n");
            }
    
            KinAlarmInfo->respTime = 0;
            KinAlarmInfo->AlarmRspStatus = KIN_RESP_WAIT;
        }else{
            /* Retransmission if non-ack */
            if(KinAlarmInfo->AlarmRspStatus == KIN_RESP_WAIT){
                KinAlarmInfo->respTime++;
                /* retransmission if respond timeout */
                if(KinAlarmInfo->respTime > RESPOND_TIMEOUT){
                    len = len + (pTxChanHead->RecordCount)*sizeof(tChanMsg) + sizeof(tAlarmTail);
                    /* Send alarm message */
                    if(lwip_sendto(KinAlarmInfo->sock, KinAlarmInfo->TxBuff, len, 0, 
                                    (struct sockaddr *)&(KinAlarmInfo->server), 
                                    sizeof(KinAlarmInfo->server)) != len){
                        printf("Error: lwip send signal failed \r\n");
                    }
                    KinAlarmInfo->respTime = 0;
                }
            }
        }
    }else{
    }

}

void SignalAlarmRspProc(tKinAlarmInfo *KinAlarmInfo)
{
    tAlarmHead *pRxAlarmHead = (tAlarmHead *)KinAlarmInfo->RxBuff;
    tAlarmHead *pTxAlarmHead = (tAlarmHead *)KinAlarmInfo->RxBuff;
    u8 opcode;
    u16 len;

    while(lwip_recv(KinAlarmInfo->sock, KinAlarmInfo->RxBuff, SIGNAL_RX_BUFF_LEN, MSG_DONTWAIT) > 0);
    opcode = pRxAlarmHead->FrameType;
    switch(opcode)
    {
        case COMAND_TRAP_RESPONE:
            if(*(u16 *)(pRxAlarmHead->SeqId) == *(u16 *)(pTxAlarmHead->SeqId))
                KinAlarmInfo->AlarmRspStatus = KIN_RESP_OK;
            memset(pRxAlarmHead, 0, SIGNAL_RX_BUFF_LEN);
            break;
        default:
            break;
    }
}

void SignalTask(void * arg)
{
    SignalAlarmInit(pKinAlarmInfo);
    PrepareAlarmHead(pKinAlarmInfo);
    
    for(;;){
        /* Check input signal per 50ms */
        SignalAlarmGetStatus(pKinAlarmInfo);
        SignalAlarmSend(pKinAlarmInfo);
        SignalAlarmRspProc(pKinAlarmInfo);
        
        /* Task poll about POLL_TIME(ms) */
        vTaskDelay(POLL_TIME);
    }
}

void SignalTaskInit(void)
{
#if BOARD_GE22103MA
	extern u8 gHardwareVer87;
	extern dev_base_info_t	DeviceBaseInfo;
#endif

#if BOARD_GE22103MA
	/* PCB version v1.10 */
	if(gHardwareVer87 == 0x03)
		return;
	
	/* 1SFP(1+7)RJ45+Data or 2SFP(1+7)RJ45+Data */
	if((DeviceBaseInfo.HardwareVer[1] != 4) && (DeviceBaseInfo.HardwareVer[1] != 9))
		return;
#endif
    
#if (BOARD_GV3S_HONUE_QM || BOARD_GE1040PU || BOARD_GE22103MA)
#else
    return;
#endif 
    
    /* create fpga upgrade task */
    xTaskCreate(SignalTask,	"tSIGNAL", 	configMINIMAL_STACK_SIZE*3, NULL,	tskIDLE_PRIORITY + 3, NULL);
}

