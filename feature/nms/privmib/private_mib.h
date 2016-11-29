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
   ---------- �������Ͷ���----------
   ----------------------------------
*/


/* �˿����� */
enum port_type
{
    ELECTRIC_TYPE,
    OPTICAL_TYPE
};


/* ��ģ/��ģ */        
enum port_op_mode
{
  START_MODE,
  SINGLE_MODE,  /* ��ģ���� */	
  MULTI_MODE,	/* ��ģ���� */	
  END_MODE
};

/* ����/˫�� */
enum port_op_fiber
{
  START_FIBER,
  SINGLE_FIBER,	/* ����*/
  DOUBLE_FIBER,	/* ˫��*/
  END_FIBER
};

/* ��ģ�鴫����� */
enum port_op_trans_dist
{
  TRAD_START,		
  _20K,	        /* 20k��*/	
  _40K,	        /* 40k��*/	
  _60K,	        /* 60k��*/
  _80K,	        /* 80k��*/	
  _120K,  	    /* 120k��*/
  TRAD_END	
};

/* ���˽�ͷ���������� */
enum port_op_type
{
  OPTICAL_TYPE_START,
  FC,	
  SC,	
  ST,	
  LC,	
  OPTICAL_TYPE_END
};

/* ������� */
enum port_elec_type
{
  ELECTRIC_TYPE_START,
  ELECTRIC_RJ45,	    /* RJ45��*/
  ELECTRIC_M12,		    /* M12��*/
  ELECTRIC_TYPE_END
};

/* ʹ�� */
enum port_state
{
  DOWN,	                /* �ر�*/
  UP	                /* ����*/
};

/* ͨ������ģʽ */
enum port_com_mode
{
  COMM_MODE_START,
  AN,	                /* ��Э��(auto-negotiation) */
  GFD,	                /* ǧ��ȫ˫����Gigabit full duplex��*/
  GHD,                  /* ǧ�װ�˫����Gigabit half duplex��*/
  FFD,                  /* ����ȫ˫����Fast full duplex��*/
  FHD,                  /* ���װ�˫����Fast half duplex��*/
  TFD,                  /* ʮ��ȫ˫��*/
  THD,                  /* ʮ�װ�˫��*/
  COMM_MODE_END
};

enum port_conn_mode
{
  ACCESS_MODE_START,
  CONN_MDIATOU=1,        /* ������Э��(auto-negotiation) */
  CONN_MDI,	             /*  MDI*/
  CONN_MDIX,             /* MDIX */
  ACCESS_MODE_END
};

/*
   ----------------------------------
   ----------- �ⲿ�궨�� -----------
   ----------------------------------
*/

#define SNMP_PRIVATE_MIB_INIT() lwip_privmib_init()

/*
   ----------------------------------
   ---------- �ӿں������� ----------
   ----------------------------------
*/
void lwip_privmib_init(void);

#endif /* LWIP_SNMP */

#endif /* __LWIP_PRIVATE_MIB_H__ */