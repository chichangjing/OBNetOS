

#ifndef __CONF_GLOBAL_H
#define __CONF_GLOBAL_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"

/********************************************************************
 *	      Trap and KinAlarm Configuration
 ********************************************************************/
#define TRAP_ENABLE					0x01
#define TRAP_DISABLE				0x00
typedef struct {
	u8	TrapEnable;					/* 0x00: Disable; 0x01: Enable */
	u8	TrapServerMac[6];
	u8	TrapFrameGate[4];
} tTrapConfig;

typedef struct {
	u8 ServerIP[4];
	u8 ServerPort[2];
	u8 Res[6];
} tUdpParam;

#define MAX_KSIG_IN_ALARM_CHANNEL_NUM			16
typedef struct {
	u8  ChanNum;
	u8  Data[MAX_KSIG_IN_ALARM_CHANNEL_NUM];
} tChanParam;

#define KSIG_IN_ALARM_WORK_MODE_UDP				0x00
#define KSIG_IN_ALARM_DEFAULT_SAMPLE_CYCLE		10		/* 10ms */
#define KSIG_IN_ALARM_DEFAULT_JITTER_TIME		15		/* 15ms */
typedef struct{
	u8	FeatureEnable;				/* 0x00: Disable; 0x01: Enable */
	u8	SampleCycle;
	u8	RemoveJitterEnable;			/* 0x00: Disable; 0x01: Enable */	
	u8	JitterProbeTime;
	u8	WorkMode;					/* 0x00: UDP; Others: Not define */
	union {
		u8 Data[12];
		tUdpParam  UdpParam;
	} ModeConfig;
	tChanParam	ChanConfig;
}tKinAlarmConfig;

/********************************************************************
 *	      Trap and KinAlarm Funcitons
 ********************************************************************/
int conf_set_trap_enable(u8 trap_enable);
int conf_get_trap_enable(u8 *trap_enable);
int conf_set_trap_server_mac(u8 *mac);
int conf_get_trap_server_mac(u8 *mac);
int conf_set_trap_frame_gate(u32 trap_gate);
int conf_get_trap_frame_gate(u32 *trap_gate);

int conf_trap_init(void);
int conf_alarm_kin_init(void);

#ifdef __cplusplus
}
#endif

#endif






