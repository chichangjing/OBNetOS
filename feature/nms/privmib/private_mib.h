/**
 * @file
 * Exports Private lwIP MIB 
 */

#ifndef __LWIP_PRIVATE_MIB_H__
#define __LWIP_PRIVATE_MIB_H__

#include "arch/cc.h"
#include "lwip/opt.h"

#if LWIP_SNMP
#include "lwip/snmp_structs.h"
#include "netif/etharp.h"
extern const struct mib_array_node private;

/** @todo remove this?? */

/**
struct private_msg
{
  u8_t dummy;
};
*/

/*
   ----------------------------------
   ---------- 数据类型定义----------
   ----------------------------------
*/


/* 端口类型 */
enum port_type
{
    ELECTRIC_TYPE,
    OPTICAL_TYPE
};


/* 单模/多模 */        
enum port_op_mode
{
  START_MODE,
  SINGLE_MODE,  /* 单模光纤 */	
  MULTI_MODE,	/* 多模光纤 */	
  END_MODE
};

/* 单纤/双纤 */
enum port_op_fiber
{
  START_FIBER,
  SINGLE_FIBER,	/* 单纤*/
  DOUBLE_FIBER,	/* 双纤*/
  END_FIBER
};

/* 光模块传输距离 */
enum port_op_trans_dist
{
  TRAD_START,		
  _20K,	        /* 20k米*/	
  _40K,	        /* 40k米*/	
  _60K,	        /* 60k米*/
  _80K,	        /* 80k米*/	
  _120K,  	    /* 120k米*/
  TRAD_END	
};

/* 光纤接头连接器类型 */
enum port_op_type
{
  OPTICAL_TYPE_START,
  FC,	
  SC,	
  ST,	
  LC,	
  OPTICAL_TYPE_END
};

/* 电口类型 */
enum port_elec_type
{
  ELECTRIC_TYPE_START,
  ELECTRIC_RJ45,	    /* RJ45口*/
  ELECTRIC_M12,		    /* M12口*/
  ELECTRIC_TYPE_END
};

/* 使能 */
enum port_state
{
  DOWN,	                /* 关闭*/
  UP	                /* 开启*/
};

/* 通信速率模式 */
enum port_com_mode
{
  COMM_MODE_START,
  AN,	                /* 自协商(auto-negotiation) */
  GFD,	                /* 千兆全双工（Gigabit full duplex）*/
  GHD,                  /* 千兆半双工（Gigabit half duplex）*/
  FFD,                  /* 百兆全双工（Fast full duplex）*/
  FHD,                  /* 百兆半双工（Fast half duplex）*/
  TFD,                  /* 十兆全双工*/
  THD,                  /* 十兆半双工*/
  COMM_MODE_END
};

enum port_conn_mode
{
  ACCESS_MODE_START,
  CONN_MDIATOU=1,        /* 连线自协商(auto-negotiation) */
  CONN_MDI,	             /*  MDI*/
  CONN_MDIX,             /* MDIX */
  ACCESS_MODE_END
};

/*
   ----------------------------------
   ----------- 外部宏定义 -----------
   ----------------------------------
*/

#define SNMP_PRIVATE_MIB_INIT() lwip_privmib_init()

/*
   ----------------------------------
   ---------- 接口函数声明 ----------
   ----------------------------------
*/
void lwip_privmib_init(void);

#endif /* LWIP_SNMP */

#endif /* __LWIP_PRIVATE_MIB_H__ */