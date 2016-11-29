#include "private_mib.h"
#include "lwip/snmp.h"
#include "lwip/snmp_msg.h"
#include "lwip/snmp_asn1.h"
#include "lwip/snmp_structs.h"

#include "netif/etharp.h"

/*
   ----------------------------------
   ---------- 数据类型定义 ----------
   ----------------------------------
*/



enum port_type
{
	PORT_START,
	PORT_RJ45,	/**端口类型，RJ45口*/
	PORT_OP,	/**端口类型，光口*/
    PORT_SFP,	/**端口类型，SFP*/
	PORT_OP_H,	/**端口类型，光口_横*/
    PORT_RJ45_H,	/**端口类型，RJ45口_横*/
	PORT_COMBO,	/**端口类型，COMBO口_竖*/
	PORT_END
};

        
enum fiber_mode
{
	START_MODE,
  	SINGLE_MODE,/* 单模光纤 */	
	MULTI_MODE,	/* 多模光纤 */	
	END_MODE
};

/**单纤/双纤*/
enum fiber_sigle_double
{
	START_FIBER,
    SINGLE_FIBER,	/* 单纤 */
	DOUBLE_FIBER,	/* 双纤 */
	END_FIBER
};


enum trans_distance
{	
	TRAD_START,		
	_20K,	/* 光模块传输距离20k米 */	
	_40K,	/* 光模块传输距离40k米 */	
	_60K,	/* 光模块传输距离60k米 */
	_80K,	/* 光模块传输距离80k米 */	
	_120K,	/* 光模块传输距离120k米 */
	TRAD_END	
};

enum optical_type
{
	OPTICAL_TYPE_START,
	FC,		/** 光纤接头连接器类型 */
	SC,		/** 光纤接头连接器类型 */
	ST,		/** 光纤接头连接器类型 */
	LC,		/** 光纤接头连接器类型 */
	OPTICAL_TYPE_END
};

enum electric_type
{
	ELECTRIC_TYPE_START,
	ELECTRIC_RJ45,		/** RJ45口 */
	ELECTRIC_M12,		/** M12口*/
	ELECTRIC_TYPE_END
};

enum enable_state
{
	DOWN,	/** 关闭 */
	UP	/** 开启 */
};

enum comm_mode
{
	COMM_MODE_START,
	AN,	/** 自协商(auto-negotiation) */
	GFD,	/** 千兆全双工（Gigabit full duplex） */
    GHD,    /** 千兆半双工（Gigabit half duplex） */
    FFD,    /** 百兆全双工（Fast full duplex） */
    FHD,    /** 百兆半双工（Fast half duplex） */
    TFD,    /** 十兆全双工 */
    THD,     /** 十兆半双工 */
	COMM_MODE_END
};


enum access_mode
{
	ACCESS_MODE_START,
	MDIATOU=1,/** 连线自协商(auto-negotiation) */
	MDI,	  /**  MDI*/
    MDIX,      /** MDIX */
	ACCESS_MODE_END
};


enum port_status
{
	PORT_DOWN,         /**端口关闭*/
	PORT_FORWARDING,   /**端口转发*/
    PORT_BLOCKING      /**端口阻塞*/
};

/** 端口表项类型 */
struct port_entry
{
	/**端口号*/
	u32_t portnum;
	/**端口类型*/
	enum port_type porttype;
    /**单模/多模*/
    enum fiber_mode fibermode;
    /**单纤/双纤*/
    enum fiber_sigle_double fibersigledouble;
	/**光模块传输距离*/
	enum trans_distance transdistance;
	/**光口类型，光纤从光缆的接头部分的不同，
	分为FC接口、SC接口、ST接口和LC接口*/ 
    enum optical_type opticaltype;
    /**电口类型*/
    enum electric_type electrictype;
    /**端口使能*/
    enum enable_state portenable;
    /**通信方式*/
    enum comm_mode commmode;
    /**MDI/MDIX模式*/
    enum access_mode accessmode;
    /**流控制*/
    enum enable_state flowcontrol;
    /**邻居探测*/
    enum enable_state neighdiscover;               
	/**端口状态*/
	enum port_status portstatus;
};


struct ring_ports_entry
{
	/**环网组号，唯一索引*/
	u32_t ringnum;
	/**环网端口对1号端口*/
	u32_t portnum1;
    /**环网端口对2号端口*/
    u32_t portnum2;
};


struct neigh_entry
{
	/**设备端口号，唯一索引*/
	u32_t portnum;
	/**设备MAC地址*/
	struct eth_addr ethaddr;
        /**邻居设备MAC地址*/
    struct eth_addr neighethaddr;
};


/**
 * Initialises this private MIB before use.
 *
 */
void lwip_privmib_init(void);

void ocstrncpy(u8_t *dst, u8_t *src, u8_t n);
void objectidncpy(s32_t *dst, s32_t *src, u8_t n);


/*
   ----------------------------------
   ------------ 配置信息 -------------
   ----------------------------------
*/

/*
   ----------------------------------
   ----------- 设备参数组------------
   ----------------------------------
*/


/** private.enterprises.ob6095.devParameter.devNum */
static u32_t devnum_len_default = 4;
static u32_t devnum_default = 3;
static u32_t* devnum_len_ptr = (u32_t*)&devnum_len_default;
static u32_t* devnum_ptr = (u32_t*)&devnum_default;

/** private.enterprises.ob6095.devParameter.ifPhysAddress */
static const u8_t ifphysaddress_len_default = 6;
static struct eth_addr ifphysaddress_default = {0x22,0x33,0x44,0x55,0x66,0x77};
static u8_t* ifphysaddress_len_ptr = (u8_t*)&ifphysaddress_len_default;
static struct eth_addr* ifphysaddress_ptr = (struct eth_addr*)&ifphysaddress_default;

/** private.enterprises.ob6095.devParameter.ipAddress */
static const u8_t ipaddress_len_default = 4;
static struct ip_addr ipaddress_default = {0};
static u8_t* ipaddress_len_ptr = (u8_t*)&ipaddress_len_default;
static struct ip_addr* ipaddress_ptr = (struct ip_addr*)&ipaddress_default;

/** private.enterprises.ob6095.devParameter.ipSubMask */
static const u8_t ipsubmask_len_default = 4;
static struct ip_addr ipsubmask_default = {0};
static u8_t* ipsubmask_len_ptr = (u8_t*)&ipsubmask_len_default;
static struct ip_addr* ipsubmask_ptr = (struct ip_addr*)&ipsubmask_default;

/** private.enterprises.ob6095.devParameter.ipGateway */
static const u8_t ipgateway_len_default = 4;
static struct ip_addr ipgateway_default = {0};
static u8_t* ipgateway_len_ptr = (u8_t*)&ipgateway_len_default;
static struct ip_addr* ipgateway_ptr = (struct ip_addr*)&ipgateway_default;

/** private.enterprises.ob6095.devParameter.devName */
static u32_t devname_len_default = 4;
static u32_t devname_default = 3;
static u32_t* devname_len_ptr = (u32_t*)&devname_len_default;
static u32_t* devname_ptr = (u32_t*)&devname_default;

/** private.enterprises.ob6095.devParameter.serialNum */
static u32_t serialnum_len_default = 4;
static u32_t serialnum_default = 3;
static u32_t* serialnum_len_ptr = (u32_t*)&serialnum_len_default;
static u32_t* serialnum_ptr = (u32_t*)&serialnum_default;

/** private.enterprises.ob6095.devParameter.sysVerNum */
static u32_t sysvernum_len_default = 4;
static u32_t sysvernum_default = 3;
static u32_t* sysvernum_len_ptr = (u32_t*)&sysvernum_len_default;
static u32_t* sysvernum_ptr = (u32_t*)&sysvernum_default;

/** private.enterprises.ob6095.devParameter.hardVerNum */
static u32_t hardvernum_len_default = 4;
static u32_t hardvernum_default = 3;
static u32_t* hardvernum_len_ptr = (u32_t*)&hardvernum_len_default;
static u32_t* hardvernum_ptr = (u32_t*)&hardvernum_default;

/** private.enterprises.ob6095.devParameter.softVerNum */
static u32_t softvernum_len_default = 4;
static u32_t softvernum_default = 3;
static u32_t* softvernum_len_ptr = (u32_t*)&softvernum_len_default;
static u32_t* softvernum_ptr = (u32_t*)&softvernum_default;

/** private.enterprises.ob6095.devParameter.masterPowVol */
static const u32_t masterpowvol_len_default = 4;
static const u32_t masterpowvol_default = 255;
static u32_t* masterpowvol_len_ptr = (u32_t*)&masterpowvol_len_default;
static u32_t* masterpowvol_ptr = (u32_t*)&masterpowvol_default;

/** private.enterprises.ob6095.devParameter.slavePowVol */
static const u32_t slavepowvol_len_default = 4;
static const u32_t slavepowvol_default = 255;
static u32_t* slavepowvol_len_ptr = (u32_t*)&slavepowvol_len_default;
static u32_t* slavepowvol_ptr = (u32_t*)&slavepowvol_default;

/*
   ----------------------------------
   ------------- 端口组--------------
   ----------------------------------
*/

/**端口条目数量*/
#define PORT_ITEM_NUMS 8
/** 端口表 */
static struct port_entry port_table[PORT_ITEM_NUMS]={
  {1,PORT_OP,MULTI_MODE,DOUBLE_FIBER,_20K,ST,
  ELECTRIC_RJ45,UP,GHD,MDI,UP,UP,PORT_FORWARDING},
  {2,PORT_OP,MULTI_MODE,DOUBLE_FIBER,_20K,ST,
  ELECTRIC_RJ45,UP,GHD,MDI,UP,UP,PORT_FORWARDING},
  {3,PORT_RJ45,MULTI_MODE,DOUBLE_FIBER,_20K,ST,
  ELECTRIC_RJ45,UP,GHD,MDI,UP,UP,PORT_FORWARDING},
  {4,PORT_RJ45,MULTI_MODE,DOUBLE_FIBER,_20K,ST,
  ELECTRIC_RJ45,UP,GHD,MDI,UP,UP,PORT_FORWARDING},
  {5,PORT_RJ45,MULTI_MODE,DOUBLE_FIBER,_20K,ST,
  ELECTRIC_RJ45,UP,GHD,MDI,UP,UP,PORT_FORWARDING},
  {6,PORT_RJ45,MULTI_MODE,DOUBLE_FIBER,_20K,ST,
  ELECTRIC_RJ45,UP,GHD,MDI,UP,UP,PORT_FORWARDING},
  {7,PORT_RJ45,MULTI_MODE,DOUBLE_FIBER,_20K,ST,
  ELECTRIC_RJ45,UP,GHD,MDI,UP,UP,PORT_FORWARDING},
  {8,PORT_RJ45,MULTI_MODE,DOUBLE_FIBER,_20K,ST,
  ELECTRIC_RJ45,UP,GHD,MDI,UP,UP,PORT_FORWARDING}
};


/*
   ----------------------------------
   ------------ AD采样组-------------
   ----------------------------------
*/


/** private.enterprises.ob6095.samplingAD.mcuTemperature */
static u32_t mcutemperature_len_default = 4;
static u32_t mcutemperature_default = 30;
static u32_t* mcutemperature_len_ptr = (u32_t*)&mcutemperature_len_default;
static u32_t* mcutemperature_ptr = (u32_t*)&mcutemperature_default;

/** private.enterprises.ob6095.samplingAD.devVoltage1 */
static u32_t devvoltage1_len_default = 4;
static u32_t devvoltage1_default = 30;
static u32_t* devvoltage1_len_ptr = (u32_t*)&devvoltage1_len_default;
static u32_t* devvoltage1_ptr = (u32_t*)&devvoltage1_default;

/** private.enterprises.ob6095.samplingAD.devVoltage2 */
static u32_t devvoltage2_len_default = 4;
static u32_t devvoltage2_default = 30;
static u32_t* devvoltage2_len_ptr = (u32_t*)&devvoltage2_len_default;
static u32_t* devvoltage2_ptr = (u32_t*)&devvoltage2_default;

/** private.enterprises.ob6095.samplingAD.devVoltage3 */
static u32_t devvoltage3_len_default = 4;
static u32_t devvoltage3_default = 30;
static u32_t* devvoltage3_len_ptr = (u32_t*)&devvoltage3_len_default;
static u32_t* devvoltage3_ptr = (u32_t*)&devvoltage3_default;

/** private.enterprises.ob6095.samplingAD.devVoltage4 */
static u32_t devvoltage4_len_default = 4;
static u32_t devvoltage4_default = 30;
static u32_t* devvoltage4_len_ptr = (u32_t*)&devvoltage4_len_default;
static u32_t* devvoltage4_ptr = (u32_t*)&devvoltage4_default;

/** private.enterprises.ob6095.samplingAD.electricCurrent */
static u32_t electriccurrent_len_default = 4;
static u32_t electriccurrent_default = 30;
static u32_t* electriccurrent_len_ptr = (u32_t*)&electriccurrent_len_default;
static u32_t* electriccurrent_ptr = (u32_t*)&electriccurrent_default;

/*
   ----------------------------------
   ------------- 环网组--------------
   ----------------------------------
*/



/** 环网数量 */
#define RING_ITEM_NUM 2

/** 环网端口对表 */
struct ring_ports_entry ring_ports_table[RING_ITEM_NUM]={
  {1,1,2},{2,3,4}};

/** 邻居表数量 */
#define NEIGH_ITEM_NUM 2

/** 邻居表 */
struct neigh_entry neigh_table[NEIGH_ITEM_NUM]={
  {1,{00,01,02,03,04,05},{00,01,02,03,04,06}},
  {2,{00,01,02,03,04,05},{00,01,02,03,04,07}}};

/** private.enterprises.ob6095.ringConf.ringNumIndex */
static u32_t ringnumindex_len_default = 4;
static u32_t ringnumindex_default = 2;
static u32_t* ringnumindex_len_ptr = (u32_t*)&ringnumindex_len_default;
static u32_t* ringnumindex_ptr = (u32_t*)&ringnumindex_default;

/** private.enterprises.ob6095.ringConf.ringNumOption */
static u32_t ringnumoption_len_default = 4;
static u32_t ringnumoption_default = 0;
static u32_t* ringnumoption_len_ptr = (u32_t*)&ringnumoption_len_default;
static u32_t* ringnumoption_ptr = (u32_t*)&ringnumoption_default;


/*
   ----------------------------------
   ------------ 函数申明 -------------
   ----------------------------------
*/


void snmp_inc_ring_ports_list(void);

/*
   ----------------------------------
   ------------ 函数接口 ------------
   ----------------------------------
*/


static void neighEntry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  u8_t id;

  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
  if (ident_len == 2)
  {
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;

    id = ident[0];
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def private neighEntry.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1:    /* portNum  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 2:    /* ifPhyAddress  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        od->v_len = 6;
        break;
      case 3:    /* neighPhyAddress  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        od->v_len = 6;
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("neighEntry_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("private neighEntry_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

static void neighEntry_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  u8_t index;  

  /* the index value can be found in: od->id_inst_ptr[1] */
  
  index = od->id_inst_ptr[1];
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* portNum  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = index; /* todo: set appropriate value */
      }
      break;
    case 2:    /* ifPhyAddress  */
      {
        ocstrncpy(value,(u8_t *)&ifphysaddress_default,ifphysaddress_len_default);
      }
      break;
    case 3:    /* neighPhyAddress  */
      {
        ocstrncpy(value, (u8_t *)&neigh_table[index-1].neighethaddr, 6);
      }
      break;
  };
}

struct mib_list_rootnode neighEntry_root = {
  &neighEntry_get_object_def,
  &neighEntry_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_LR,
  0,
  NULL,
  NULL,  0,
};

/* neighEntry  .1.3.6.1.4.1.42421.3.3.1    */
const s32_t neighEntry_ids[3] = { 1, 2, 3 };
struct mib_node* const neighEntry_nodes[3] = { 
  (struct mib_node* const)&neighEntry_root,
  (struct mib_node* const)&neighEntry_root,
  (struct mib_node* const)&neighEntry_root
};

const struct mib_array_node neighEntry = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  3,
  neighEntry_ids,
  neighEntry_nodes
};

/* neighTable  .1.3.6.1.4.1.42421.3.3    */
s32_t neighTable_ids[1] = { 1 };
struct mib_node* neighTable_nodes[1] = { 
  (struct mib_node* const)&neighEntry
};

struct mib_ram_array_node neighTable = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_RA,
  0,
  neighTable_ids,
  neighTable_nodes
};

static void ringPortsEntry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  u8_t id;

  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
  if (ident_len == 2)
  {
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;

    id = ident[0];
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def private ringPortsEntry.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1:    /* ringNum  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 2:    /* portNum1  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 3:    /* portNum2  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("ringPortsEntry_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("private ringPortsEntry_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

static void ringPortsEntry_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  u8_t index;

  /* the index value can be found in: od->id_inst_ptr[1] */
  index = od->id_inst_ptr[1];
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* ringNum  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = ring_ports_table[index-1].ringnum; /* todo: set appropriate value */
      }
      break;
    case 2:    /* portNum1  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = ring_ports_table[index-1].portnum1; /* todo: set appropriate value */
      }
      break;
    case 3:    /* portNum2  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = ring_ports_table[index-1].portnum2; /* todo: set appropriate value */
      }
      break;
  };
}

static u8_t ringPortsEntry_set_test(struct obj_def *od, u16_t len, void *value)
{
  u8_t id, set_ok;

  /* the index value can be found in: od->id_inst_ptr[1] */
  set_ok = 0;
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* ringNum  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 2:    /* portNum1  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 3:    /* portNum2  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
  };
  return set_ok;
}

static void ringPortsEntry_set_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  u8_t index;

  /* the index value can be found in: od->id_inst_ptr[1] */
  index = od->id_inst_ptr[1];
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* ringNum  */
      {
        s32_t *sint_ptr = value;
        ring_ports_table[index-1].ringnum = *sint_ptr;  /* do something with the value */
      }
      break;
    case 2:    /* portNum1  */
      {
        s32_t *sint_ptr = value;
        ring_ports_table[index-1].portnum1 = *sint_ptr;  /* do something with the value */
      }
      break;
    case 3:    /* portNum2  */
      {
        s32_t *sint_ptr = value;
        ring_ports_table[index-1].portnum2 = *sint_ptr;  /* do something with the value */
      }
      break;
  };
}

struct mib_list_rootnode ringPortsEntry_root = {
  &ringPortsEntry_get_object_def,
  &ringPortsEntry_get_value,
  &ringPortsEntry_set_test,
  &ringPortsEntry_set_value,
  MIB_NODE_LR,
  0,
  NULL,
  NULL,  0,
};

/* ringPortsEntry  .1.3.6.1.4.1.42421.3.2.1    */
const s32_t ringPortsEntry_ids[3] = { 1, 2, 3 };
struct mib_node* const ringPortsEntry_nodes[3] = { 
  (struct mib_node* const)&ringPortsEntry_root,
  (struct mib_node* const)&ringPortsEntry_root,
  (struct mib_node* const)&ringPortsEntry_root
};

const struct mib_array_node ringPortsEntry = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  3,
  ringPortsEntry_ids,
  ringPortsEntry_nodes
};

/* ringPortsTable  .1.3.6.1.4.1.42421.3.2    */
s32_t ringPortsTable_ids[1] = { 1 };
struct mib_node* ringPortsTable_nodes[1] = { 
  (struct mib_node* const)&ringPortsEntry
};

struct mib_ram_array_node ringPortsTable = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_RA,
  0,
  ringPortsTable_ids,
  ringPortsTable_nodes
};

static void portEntry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  u8_t id;
  u8_t index;
  enum port_type type;

  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
  if (ident_len == 2)
  {
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;

    id = ident[0];
	index = ident[1];
	/* 端口类型 */
	type = port_table[index-1].porttype;
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def private portEntry.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1:    /* portNum  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 2:    /* portType  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 3:    /* fiberMode  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
		if(type == PORT_OP || type == PORT_SFP ||
			type == PORT_OP_H || type == PORT_COMBO)
		{
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
		}else{
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_NUL);
			od->v_len = 0;// todo:  set the appropriate length eg. sizeof(u32_t);
		}

        break;
      case 4:    /* fiberSigleDouble  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
		if(type == PORT_OP || type == PORT_SFP ||
			type == PORT_OP_H || type == PORT_COMBO)
		{
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
		}else{
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_NUL);
			od->v_len = 0;// todo:  set the appropriate length eg. sizeof(u32_t);
		}
        break;
      case 5:    /* transDistance  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
		if(type == PORT_OP || type == PORT_SFP ||
			type == PORT_OP_H || type == PORT_COMBO)
		{
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
		}else{
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_NUL);
			od->v_len = 0;// todo:  set the appropriate length eg. sizeof(u32_t);
		}
        break;
      case 6:    /* opticalType  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
		if(type == PORT_OP || type == PORT_SFP ||
			type == PORT_OP_H || type == PORT_COMBO)
		{
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
		}else{
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_NUL);
			od->v_len = 0;// todo:  set the appropriate length eg. sizeof(u32_t);
		}
        break;
      case 7:    /* electricType  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
		if(type == PORT_RJ45 || 
			type == PORT_RJ45_H || type == PORT_COMBO)
		{
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
			od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
		}else{
			od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_NUL);
			od->v_len = 0;// todo:  set the appropriate length eg. sizeof(u32_t);
		}
        break;
      case 8:    /* portEnable  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 9:    /* commMode  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 10:    /* accessMode  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 11:    /* flowControl  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 12:    /* neighDiscover  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 13:    /* portStatus  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("portEntry_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("private portEntry_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

static void portEntry_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  u8_t index;
  enum port_type type;

  /* the index value can be found in: od->id_inst_ptr[1] */
  index = od->id_inst_ptr[1];
  /* 端口类型 */
  type = port_table[index-1].porttype;
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* portNum  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = index; /* todo: set appropriate value */
      }
      break;
    case 2:    /* portType  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = port_table[index-1].porttype; /* todo: set appropriate value */
      }
      break;
    case 3:    /* fiberMode  */
      {
        s32_t *sint_ptr = value;

		if(type == PORT_OP || type == PORT_SFP || type == PORT_OP_H || type == PORT_COMBO)
			*sint_ptr = port_table[index-1].fibermode; /* todo: set appropriate value */

      }
      break;
    case 4:    /* fiberSigleDouble  */
      {
        s32_t *sint_ptr = value;

		if(type == PORT_OP || type == PORT_SFP || type == PORT_OP_H || type == PORT_COMBO)
        *sint_ptr = port_table[index-1].fibersigledouble; /* todo: set appropriate value */
      }
      break;
    case 5:    /* transDistance  */
      {
        s32_t *sint_ptr = value;

		if(type == PORT_OP || type == PORT_SFP || type == PORT_OP_H || type == PORT_COMBO)
        *sint_ptr = port_table[index-1].transdistance; /* todo: set appropriate value */
      }
      break;
    case 6:    /* opticalType  */
      {
        s32_t *sint_ptr = value;

		if(type == PORT_OP || type == PORT_SFP || type == PORT_OP_H || type == PORT_COMBO)
        *sint_ptr = port_table[index-1].opticaltype; /* todo: set appropriate value */
      }
      break;
    case 7:    /* electricType  */
      {
        s32_t *sint_ptr = value;
		
		if(type == PORT_RJ45 || type == PORT_RJ45_H || type == PORT_COMBO)
        *sint_ptr = port_table[index-1].electrictype; /* todo: set appropriate value */
      }
      break;
    case 8:    /* portEnable  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = port_table[index-1].portenable; /* todo: set appropriate value */
      }
      break;
    case 9:    /* commMode  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = port_table[index-1].commmode; /* todo: set appropriate value */
      }
      break;
    case 10:    /* accessMode  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = port_table[index-1].accessmode; /* todo: set appropriate value */
      }
      break;
    case 11:    /* flowControl  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = port_table[index-1].flowcontrol; /* todo: set appropriate value */
      }
      break;
    case 12:    /* neighDiscover  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = port_table[index-1].neighdiscover; /* todo: set appropriate value */
      }
      break;
    case 13:    /* portStatus  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = port_table[index-1].portstatus; /* todo: set appropriate value */
      }
      break;
  };
}

static u8_t portEntry_set_test(struct obj_def *od, u16_t len, void *value)
{
  u8_t id, set_ok;
  u8_t index;
  enum port_type type;

  /* the index value can be found in: od->id_inst_ptr[1] */
  index = od->id_inst_ptr[1];
  /* 端口类型 */
  type = port_table[index-1].porttype;
  set_ok = 0;
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 2:    /* portType  */
  /* validate the value argument and set ok  */
		{
			enum port_type *sint_ptr = value;

			if( *sint_ptr > PORT_START && *sint_ptr < PORT_END )
			set_ok = 1;
		}
      
      break;
    case 3:    /* fiberMode  */
  /* validate the value argument and set ok  */
		{
			enum fiber_mode *sint_ptr = value;
     
			if(type == PORT_OP || type == PORT_SFP || type == PORT_OP_H || type == PORT_COMBO)
      
			{	
				if( *sint_ptr > START_MODE && *sint_ptr < END_MODE )
					set_ok = 1;   
			}
		}
      break;
    case 4:    /* fiberSigleDouble  */
  /* validate the value argument and set ok  */
		{
			enum fiber_sigle_double *sint_ptr = value;
 
			if(type == PORT_OP || type == PORT_SFP || type == PORT_OP_H || type == PORT_COMBO)
			{
				if( *sint_ptr > START_FIBER && *sint_ptr < END_FIBER )
					set_ok = 1;
			}
		}
      break;
    case 5:    /* transDistance  */
  /* validate the value argument and set ok  */
		{	
			enum trans_distance *sint_ptr = value;
 
			if(type == PORT_OP || type == PORT_SFP || type == PORT_OP_H || type == PORT_COMBO)
			{
				if( *sint_ptr > TRAD_START && *sint_ptr < TRAD_END )
					set_ok = 1;
			}
		}
      break;
    case 6:    /* opticalType  */
  /* validate the value argument and set ok  */
		{
			enum optical_type *sint_ptr = value;
 
			if(type == PORT_OP || type == PORT_SFP || type == PORT_OP_H || type == PORT_COMBO)
			{
				if( *sint_ptr > OPTICAL_TYPE_START && *sint_ptr < OPTICAL_TYPE_END )
					set_ok = 1;
			}
		}
      break;
    case 7:    /* electricType  */
  /* validate the value argument and set ok  */
		{
			enum electric_type *sint_ptr = value;
       
			if(type == PORT_RJ45 || type == PORT_RJ45_H || type == PORT_COMBO)    
			{	
				if( *sint_ptr > ELECTRIC_TYPE_START && *sint_ptr < ELECTRIC_TYPE_END )	
					set_ok = 1;	
			}
		}
      break;
    case 8:    /* portEnable  */
  /* validate the value argument and set ok  */
		{
			enum enable_state  *sint_ptr = value;
		
			if( *sint_ptr == DOWN || *sint_ptr == UP )				
				set_ok = 1;
		}
      break;
    case 9:    /* commMode  */
  /* validate the value argument and set ok  */
		{
			enum comm_mode  *sint_ptr = value;

			if( *sint_ptr > COMM_MODE_START && *sint_ptr < COMM_MODE_END )
				set_ok = 1;
		}
      break;
    case 10:    /* accessMode  */
  /* validate the value argument and set ok  */
		{
			enum access_mode *sint_ptr = value;
			
			if( *sint_ptr > ACCESS_MODE_START && *sint_ptr < ACCESS_MODE_END )
				set_ok = 1;
		}
     
      break;
    case 11:    /* flowControl  */
  /* validate the value argument and set ok  */
		{
			enum enable_state  *sint_ptr = value;

				if( *sint_ptr == DOWN || *sint_ptr == UP )
					set_ok = 1;
		}
      
      break;
    case 12:    /* neighDiscover  */
  /* validate the value argument and set ok  */
		{
			enum enable_state  *sint_ptr = value;
							
			if( *sint_ptr == DOWN || *sint_ptr == UP )
					set_ok = 1;
		}
      break;
  };
  return set_ok;
}

static void portEntry_set_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  u8_t index;

  /* the index value can be found in: od->id_inst_ptr[1] */
  index = od->id_inst_ptr[1];
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 2:    /* portType  */
      {
        s32_t *sint_ptr = value;
        port_table[index-1].porttype = *sint_ptr;  /* do something with the value */
      }
      break;
    case 3:    /* fiberMode  */
      {
        s32_t *sint_ptr = value;
        port_table[index-1].fibermode = *sint_ptr;  /* do something with the value */
      }
      break;
    case 4:    /* fiberSigleDouble  */
      {
        s32_t *sint_ptr = value;
        port_table[index-1].fibersigledouble = *sint_ptr;  /* do something with the value */
      }
      break;
    case 5:    /* transDistance  */
      {
        s32_t *sint_ptr = value;
        port_table[index-1].transdistance = *sint_ptr;  /* do something with the value */
      }
      break;
    case 6:    /* opticalType  */
      {
        s32_t *sint_ptr = value;
        port_table[index-1].opticaltype = *sint_ptr;  /* do something with the value */
      }
      break;
    case 7:    /* electricType  */
      {
        s32_t *sint_ptr = value;
        port_table[index-1].electrictype = *sint_ptr;  /* do something with the value */
      }
      break;
    case 8:    /* portEnable  */
      {
        s32_t *sint_ptr = value;
        port_table[index-1].portenable = *sint_ptr;  /* do something with the value */
      }
      break;
    case 9:    /* commMode  */
      {
        s32_t *sint_ptr = value;
        port_table[index-1].commmode = *sint_ptr;  /* do something with the value */
      }
      break;
    case 10:    /* accessMode  */
      {
        s32_t *sint_ptr = value;
        port_table[index-1].accessmode = *sint_ptr;  /* do something with the value */
      }
      break;
    case 11:    /* flowControl  */
      {
        s32_t *sint_ptr = value;
        port_table[index-1].flowcontrol = *sint_ptr;  /* do something with the value */
      }
      break;
    case 12:    /* neighDiscover  */
      {
        s32_t *sint_ptr = value;
        port_table[index-1].neighdiscover = *sint_ptr;  /* do something with the value */
      }
      break;
  };
}

struct mib_list_rootnode portEntry_root = {
  &portEntry_get_object_def,
  &portEntry_get_value,
  &portEntry_set_test,
  &portEntry_set_value,
  MIB_NODE_LR,
  0,
  NULL,
  NULL,  0,
};

/* portEntry  .1.3.6.1.4.1.42421.2.2.1    */
const s32_t portEntry_ids[13] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
struct mib_node* const portEntry_nodes[13] = { 
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root,
  (struct mib_node* const)&portEntry_root
};

const struct mib_array_node portEntry = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  13,
  portEntry_ids,
  portEntry_nodes
};

/* portTable  .1.3.6.1.4.1.42421.2.2    */
s32_t portTable_ids[1] = { 1 };
struct mib_node* portTable_nodes[1] = { 
  (struct mib_node* const)&portEntry
};

struct mib_ram_array_node portTable = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_RA,
  0,
  portTable_ids,
  portTable_nodes
};

static void samplingAD_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  u8_t id;

  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
  if (ident_len == 2)
  {
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;

    id = ident[0];
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def private samplingAD.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1:    /* mcuTemperature  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 2:    /* devVoltage1  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 3:    /* devVoltage2  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 4:    /* devVoltage3  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 5:    /* devVoltage4  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 6:    /* electricCurrent  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("samplingAD_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("private samplingAD_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

static void samplingAD_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;

  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* mcuTemperature  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *mcutemperature_ptr; /* todo: set appropriate value */
      }
      break;
    case 2:    /* devVoltage1  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *devvoltage1_ptr; /* todo: set appropriate value */
      }
      break;
    case 3:    /* devVoltage2  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *devvoltage2_ptr; /* todo: set appropriate value */
      }
      break;
    case 4:    /* devVoltage3  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *devvoltage3_ptr; /* todo: set appropriate value */
      }
      break;
    case 5:    /* devVoltage4  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *devvoltage4_ptr; /* todo: set appropriate value */
      }
      break;
    case 6:    /* electricCurrent  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *electriccurrent_ptr; /* todo: set appropriate value */
      }
      break;
  };
}

const mib_scalar_node samplingAD_scalar = {
  &samplingAD_get_object_def,
  &samplingAD_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_SC,
  0
};

/* samplingAD  .1.3.6.1.4.1.42421.4    */
const s32_t samplingAD_ids[6] = { 1, 2, 3, 4, 5, 6 };
struct mib_node* const samplingAD_nodes[6] = { 
  (struct mib_node* const)&samplingAD_scalar,
  (struct mib_node* const)&samplingAD_scalar,
  (struct mib_node* const)&samplingAD_scalar,
  (struct mib_node* const)&samplingAD_scalar,
  (struct mib_node* const)&samplingAD_scalar,
  (struct mib_node* const)&samplingAD_scalar
};

const struct mib_array_node samplingAD = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  6,
  samplingAD_ids,
  samplingAD_nodes
};

static void ringConf_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  u8_t id;

  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
  if (ident_len == 2)
  {
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;

    id = ident[0];
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def private ringConf.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1:    /* ringPortsNum  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
	  case 4:    /* ringNumIndex  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 5:    /* ringNumOption  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("ringConf_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("private ringConf_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

static void ringConf_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;

  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* ringPortsNum  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = RING_ITEM_NUM; /* todo: set appropriate value */
      }
	  break;
	case 4:    /* ringNumIndex  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = ringnumindex_default ; /* todo: set appropriate value */
      }
      break;
    case 5:    /* ringNumOption  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = ringnumoption_default; /* todo: set appropriate value */
      }
      break;
  };
}

static u8_t ringConf_set_test(struct obj_def *od, u16_t len, void *value)
{
  u8_t id, set_ok;

  set_ok = 0;
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 4:    /* ringNumIndex  */
  /* validate the value argument and set ok  */
		set_ok = 1;
      break;
    case 5:    /* ringNumOption  */
  /* validate the value argument and set ok  */
      {
        s32_t *sint_ptr = value;
       if( *sint_ptr==1 || *sint_ptr==0 )  /* do something with the value */
		   		set_ok = 1;
      }

      break;
  };
  return set_ok;
}

static void ringConf_set_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;

  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 4:    /* ringNumIndex  */
      {
        s32_t *sint_ptr = value;
        *ringnumindex_ptr = *sint_ptr;  /* do something with the value */
      }
      break;
    case 5:    /* ringNumOption  */
      {
        s32_t *sint_ptr = value;
        switch( *sint_ptr )  /* do something with the value */
		{
		case 0:
			break;
		case 1:
			{	
				struct mib_list_node *ringPorts_node = NULL;
				snmp_mib_node_insert(&ringPortsEntry_root, 2, &ringPorts_node);			
				ringPortsTable.maxlength = 1;
			}

			break;
		}
      }
      break;
  };
}

const mib_scalar_node ringConf_scalar = {
  &ringConf_get_object_def,
  &ringConf_get_value,
  &ringConf_set_test,
  &ringConf_set_value,
  MIB_NODE_SC,
  0
};

/* ringConf  .1.3.6.1.4.1.42421.3    */
const s32_t ringConf_ids[5] = { 1, 2, 3, 4, 5 };
struct mib_node* const ringConf_nodes[5] = { 
  (struct mib_node* const)&ringConf_scalar,
  (struct mib_node* const)&ringPortsTable,
  (struct mib_node* const)&neighTable,
  (struct mib_node* const)&ringConf_scalar,
  (struct mib_node* const)&ringConf_scalar
};

const struct mib_array_node ringConf = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  5,
  ringConf_ids,
  ringConf_nodes
};

static void portsConf_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  u8_t id;

  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
  if (ident_len == 2)
  {
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;

    id = ident[0];
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def private portsConf.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1:    /* portNumber  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("portsConf_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("private portsConf_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

static void portsConf_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;

  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* portNumber  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = PORT_ITEM_NUMS; /* todo: set appropriate value */
      }
      break;
  };
}

static u8_t portsConf_set_test(struct obj_def *od, u16_t len, void *value)
{
  u8_t id, set_ok;

  set_ok = 0;
  id = od->id_inst_ptr[0];
  switch (id)
  {
  };
  return set_ok;
}

static void portsConf_set_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;

  id = od->id_inst_ptr[0];
  switch (id)
  {
  };
}

const mib_scalar_node portsConf_scalar = {
  &portsConf_get_object_def,
  &portsConf_get_value,
  &portsConf_set_test,
  &portsConf_set_value,
  MIB_NODE_SC,
  0
};

/* portsConf  .1.3.6.1.4.1.42421.2    */
const s32_t portsConf_ids[2] = { 1, 2 };
struct mib_node* const portsConf_nodes[2] = { 
  (struct mib_node* const)&portsConf_scalar,
  (struct mib_node* const)&portTable
};

const struct mib_array_node portsConf = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  2,
  portsConf_ids,
  portsConf_nodes
};

static void devParameters_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  u8_t id;

  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
  if (ident_len == 2)
  {
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;

    id = ident[0];
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def private devParameters.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1:    /* devNum  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 2:    /* ifPhysAddress  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        od->v_len = 6;
        break;
      case 3:    /* ipAddress  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
        od->v_len = 4;
        break;
      case 4:    /* ipSubMask  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
        od->v_len = 4;
        break;
      case 5:    /* ipGateway  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
        od->v_len = 4;
        break;
      case 6:    /* devName  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 7:    /* serialNum  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 8:    /* sysVerNum  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 9:    /* hardVerNum  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 10:    /* softVerNum  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 11:    /* masterPowVol  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 12:    /* slavePowVol  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("devParameters_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("private devParameters_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

static void devParameters_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  struct netif *netif = netif_list;

  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* devNum  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *devnum_ptr ; /* todo: set appropriate value */
      }
      break;
    case 2:    /* ifPhysAddress  */
      //ocstrncpy(value,netif->hwaddr,netif->hwaddr_len);
      
      {
        struct eth_addr *dst = value;

        *dst = ifphysaddress_default;
      }
      break;
    case 3:    /* ipAddress  */
        {
          struct ip_addr *dst = value;
       //   *dst = netif->ip_addr;
          *dst = ipaddress_default;
        }
      break;
    case 4:    /* ipSubMask  */
      	{
          struct ip_addr *dst = value;
	//  *dst = netif->netmask;
          *dst = ipsubmask_default;
        }
      break;
    case 5:    /* ipGateway  */
        {
          struct ip_addr *dst = value;
	//  *dst = netif->gw;
          *dst = ipgateway_default;
        }
      break;
    case 6:    /* devName  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *devname_ptr ; /* todo: set appropriate value */
      }
      break;
    case 7:    /* serialNum  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *serialnum_ptr; /* todo: set appropriate value */
      }
      break;
    case 8:    /* sysVerNum  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *sysvernum_ptr; /* todo: set appropriate value */
      }
      break;
    case 9:    /* hardVerNum  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *hardvernum_ptr; /* todo: set appropriate value */
      }
      break;
    case 10:    /* softVerNum  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *softvernum_ptr; /* todo: set appropriate value */
      }
      break;
    case 11:    /* masterPowVol  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *masterpowvol_ptr; /* todo: set appropriate value */
      }
      break;
    case 12:    /* slavePowVol  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = *slavepowvol_ptr; /* todo: set appropriate value */
      }
      break;
  };
}

static u8_t devParameters_set_test(struct obj_def *od, u16_t len, void *value)
{
  u8_t id, set_ok;

  set_ok = 0;
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* devNum  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 2:    /* ifPhysAddress  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 3:    /* ipAddress  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 4:    /* ipSubMask  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 5:    /* ipGateway  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 6:    /* devName  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 7:    /* serialNum  */
  /* validate the value argument and set ok  */
      break;
    case 8:    /* sysVerNum  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 9:    /* hardVerNum  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 10:    /* softVerNum  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
  };
  return set_ok;
}

static void devParameters_set_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;

  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* devNum  */
      {
        s32_t *sint_ptr = value;
        *devnum_ptr = *sint_ptr;  /* do something with the value */
      }
      break;
    case 2:    /* ifPhysAddress  */
      {
        struct eth_addr *sint_ptr = value;
        ifphysaddress_default = *sint_ptr;

      }
      break;
    case 3:    /* ipAddress  */
      {
        struct ip_addr *sint_ptr = value;
        ipaddress_default = *sint_ptr;
      }
      break;
    case 4:    /* ipSubMask  */
      {
        struct ip_addr *sint_ptr = value;
        ipsubmask_default = *sint_ptr;
      }
      break;
    case 5:    /* ipGateway  */
      {
        struct ip_addr *sint_ptr = value;
        ipgateway_default = *sint_ptr;
      }
      break;
    case 6:    /* devName  */
      {
        s32_t *sint_ptr = value;
        *devname_ptr = *sint_ptr;  /* do something with the value */
      }
      break;
    case 7:    /* serialNum  */
      {
        s32_t *sint_ptr = value;
        *serialnum_ptr = *sint_ptr;  /* do something with the value */
      }
      break;
    case 8:    /* sysVerNum  */
      {
        s32_t *sint_ptr = value;
        *sysvernum_ptr = *sint_ptr;  /* do something with the value */
      }
      break;
    case 9:    /* hardVerNum  */
      {
        s32_t *sint_ptr = value;
        *hardvernum_ptr = *sint_ptr;  /* do something with the value */
      }
      break;
    case 10:    /* softVerNum  */
      {
        s32_t *sint_ptr = value;
        *softvernum_ptr = *sint_ptr;  /* do something with the value */
      }
      break;
  };
}

const mib_scalar_node devParameters_scalar = {
  &devParameters_get_object_def,
  &devParameters_get_value,
  &devParameters_set_test,
  &devParameters_set_value,
  MIB_NODE_SC,
  0
};

/* devParameters  .1.3.6.1.4.1.42421.1    */
const s32_t devParameters_ids[12] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
struct mib_node* const devParameters_nodes[12] = { 
  (struct mib_node* const)&devParameters_scalar,
  (struct mib_node* const)&devParameters_scalar,
  (struct mib_node* const)&devParameters_scalar,
  (struct mib_node* const)&devParameters_scalar,
  (struct mib_node* const)&devParameters_scalar,
  (struct mib_node* const)&devParameters_scalar,
  (struct mib_node* const)&devParameters_scalar,
  (struct mib_node* const)&devParameters_scalar,
  (struct mib_node* const)&devParameters_scalar,
  (struct mib_node* const)&devParameters_scalar,
  (struct mib_node* const)&devParameters_scalar,
  (struct mib_node* const)&devParameters_scalar
};

const struct mib_array_node devParameters = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  12,
  devParameters_ids,
  devParameters_nodes
};

/* ob6095  .1.3.6.1.4.1.42421    */
const s32_t ob6095_ids[4] = { 1, 2, 3, 4 };
struct mib_node* const ob6095_nodes[4] = { 
  (struct mib_node* const)&devParameters,
  (struct mib_node* const)&portsConf,
  (struct mib_node* const)&ringConf,
  (struct mib_node* const)&samplingAD
};

const struct mib_array_node ob6095 = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  4,
  ob6095_ids,
  ob6095_nodes
};

/* enterprises  .1.3.6.1.4.1    */
const s32_t enterprises_ids[1] = { 42421 };
struct mib_node* const enterprises_nodes[1] = { 
  (struct mib_node* const)&ob6095
};

const struct mib_array_node enterprises = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  1,
  enterprises_ids,
  enterprises_nodes
};

/* private  .1.3.6.1.4    */
const s32_t private_ids[1] = { 1 };
struct mib_node* const private_nodes[1] = { 
  (struct mib_node* const)&enterprises
};

const struct mib_array_node private = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  1,
  private_ids,
  private_nodes
};


void snmp_inc_portlist(void)
{
  struct mib_list_node *port_node = NULL;
  s32_t i;

  for(i = 0 ; i<PORT_ITEM_NUMS ; i++)
  {
    snmp_mib_node_insert(&portEntry_root, portEntry_root.count + 1, &port_node);
  }
	
  /* enable getnext traversal on filled table */
  portTable.maxlength = 1;
}

void snmp_inc_ring_ports_list(void)
{
  struct mib_list_node *ringPorts_node = NULL;
  s32_t i;

  for(i = 0 ; i<RING_ITEM_NUM ; i++)
  {
    snmp_mib_node_insert(&ringPortsEntry_root, ringPortsEntry_root.count + 1, &ringPorts_node);
  }
	
  /* enable getnext traversal on filled table */
  ringPortsTable.maxlength = 1;
}


void snmp_inc_neigh_list(void)
{
  struct mib_list_node *neigh_node = NULL;
  s32_t i;

  for(i = 0 ; i<NEIGH_ITEM_NUM ; i++)
  {
    snmp_mib_node_insert(&neighEntry_root, neighEntry_root.count + 1, &neigh_node);
  }
	
  /* enable getnext traversal on filled table */
  neighTable.maxlength = 1;
}

void
lwip_privmib_init(void)
{
  IP4_ADDR(&ipaddress_default,192,168,1,16);
  IP4_ADDR(&ipsubmask_default,255,255,255,0);
  IP4_ADDR(&ipgateway_default,192,168,1,1);
  
  snmp_inc_portlist();
//  snmp_inc_ring_ports_list();
  snmp_inc_neigh_list();
}

