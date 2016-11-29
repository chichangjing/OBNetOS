/*************************************************************
 * Filename     : conf_signal.h
 * Description  : API for CLI
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/

#ifndef _CONF_SIGNAL_H
#define _CONF_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/* include -------------------------------------------------------------------*/
#include "stm32f2xx.h"  
#include "conf_global.h"
    
/* Exported types ------------------------------------------------------------*/
typedef struct signal_cfg
{
    u8  enable;
    u8  AlarmType;
    u8  AlarmMode;
    u32 ip;
    u16 port;
    u16 time;
}signal_cfg_t;

typedef enum
{
    WORKMODE_UDP,
    WORKMODE_TCP
}eWorkMode;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/    
/* Alarm Type */
#define ALARMTYPE_CLOSE             0x00    /* 闭合告警 */
#define ALARMTYPE_OPEN              0x01    /* 断开告警 */
#define ALARMTYPE_DOWN              0x02    /* 掉电告警 */
#define ALARMTYPE_UNKNOW            0x03    /* 未知告警 */

/* Channel alarm enable*/
#define CHAN_GET_EN(param)          ((param >> 0x07) & 0x01)
#define CHAN_GET_ALARM_TYPE(param)  (param & 0x03)

/* Exported functions --------------------------------------------------------*/
int get_signal_en(u8 *sig_en);
int set_signal_en(u8 sig_en);

int get_sampcycle_time(u8 *time);
int set_sampcycle_time(u8 time);

int get_jitter_en(u8 *en);
int set_jitter_en(u8 en);

int get_jitter_probe(u8 *time);
int set_jitter_probe(u8 time);

int get_work_mode(u8 *sig_mode);
int set_work_mode(u8 sig_mode);

int get_server_ip(u32 *ip_info);
int set_server_ip(u32 ip_info);

int get_server_port(u16 *port);
int set_server_port(u16 port);

int get_chan_num(u8 *num);
int set_chan_num(u8 num);

int get_chan_en(u8 chanid, u8 *en);
int set_chan_en(u8 chanid, u8 en);

int get_chan_type(u8 chanid, u8 *type);
int set_chan_type(u8 chanid, u8 type);

int set_signal_cfg(signal_cfg_t * sigcfg);
int signal_cfg_fetch(tKinAlarmConfig *KinAlarmCfg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONF_SIGNAL_H */