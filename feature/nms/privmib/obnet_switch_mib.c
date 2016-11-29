#include "mconfig.h"

#if MODULE_SNMP
#include "private_mib.h"
#include "lwip/snmp.h"
#include "lwip/snmp_msg.h"
#include "lwip/snmp_asn1.h"
#include "lwip/snmp_structs.h"

#include "conf_map.h"
#include "soft_i2c.h"

#include "string.h"

#if SWITCH_CHIP_88E6095
#include "msApi.h"
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>
#endif

#include "hal_swif_port.h"

/**
 * Initialises this private MIB before use.
 *
 */
void lwip_privmib_init(void);

void ocstrncpy(u8_t *dst, u8_t *src, u8_t n);
void objectidncpy(s32_t *dst, s32_t *src, u8_t n);

#define SWITCHTYPE_LEN 8
static u8 BoardType[SWITCHTYPE_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#if SWITCH_CHIP_88E6095
extern GT_QD_DEV *dev;
#endif

static void portConfEntry_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  u8_t id;
  u8_t index;
  u8_t PortConfig[4];
  enum port_type type;
  u8_t port;
  HAL_PORT_LINK_STATE link_state;

  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
  if (ident_len == 2)
  {
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;

    index = ident[1];
    eeprom_read(NVRAM_PORT_CFG_BASE+(1+(index-1)*4), PortConfig, 4); 
    
    /* 端口类型*/
    type = (PortConfig[0] >> 6) & 0x01;
    id = ident[0];
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def private portConfEntry.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1:    /* portIndex  */
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
      case 3:    /* portOpMode  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
       /* 判断是光口时该对象是INTEGER类型 */
        if(type == OPTICAL_TYPE)
        {   		
          od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);		
          od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);	
        }
        /* 判断是电口时该对象是NULL类型 */
        else{		
          od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_NUL);		
          od->v_len = 0;// todo:  set the appropriate length eg. sizeof(u32_t);	
        }
        break;
      case 4:    /* portOpFiber  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        /* 判断是光口时该对象是INTEGER类型 */
        if(type == OPTICAL_TYPE)
        {	
          od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);	
          od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        }
        /* 判断是电口时该对象是NULL类型 */
        else{		
          od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_NUL);	
          od->v_len = 0;// todo:  set the appropriate length eg. sizeof(u32_t);
        }
        break;
      case 5:    /* portOpTransDist  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        /* 判断是光口时该对象是INTEGER类型 */
        if(type == OPTICAL_TYPE)
        {	
          od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);	
          od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        }
        /* 判断是电口时该对象是NULL类型 */
        else{	
          od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_NUL);	
          od->v_len = 0;// todo:  set the appropriate length eg. sizeof(u32_t);
        }
        break;
      case 6:    /* portOpType  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        /* 判断是光口时该对象是INTEGER类型 */
        if(type == OPTICAL_TYPE)
        {	
          od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);            
          od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        }
        /* 判断是电口时该对象是NULL类型 */
        else{	
          od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_NUL);	
          od->v_len = 0;// todo:  set the appropriate length eg. sizeof(u32_t);
        }
        break;
      case 7:    /* portElecType  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        /* 判断是光口时该对象是INTEGER类型 */
        if(type == ELECTRIC_TYPE)
        {	
          od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);	
          od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        }
        /* 判断是电口时该对象是NULL类型 */
        else{	
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
      case 9:    /* portComMode  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 10:    /* portConnMode  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 11:    /* portFlowCtrl  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 12:    /* portNeighDisc  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 13:    /* portLinkState  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 14:    /* portComState  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_ONLY;
        //port = hal_swif_lport_2_hport(index);
        hal_swif_port_get_link_state(index, &link_state);
        if(link_state == LINK_UP)
        {
            od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
            od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        }else{
            od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_NUL);	
            od->v_len = 0;// todo:  set the appropriate length eg. sizeof(u32_t);
        }
        break;
      case 15:    /* portConnState  */
        od->instance = MIB_OBJECT_TAB;
        od->access = MIB_OBJECT_READ_ONLY;
        //port = hal_swif_lport_2_hport(index);
        hal_swif_port_get_link_state(index, &link_state);
        if(link_state == LINK_UP)
        {
            od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
            od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        }else{
            od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_NUL);	
            od->v_len = 0;// todo:  set the appropriate length eg. sizeof(u32_t);
        }
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("portConfEntry_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("private portConfEntry_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

static void portConfEntry_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  u8_t index;
  u8_t port;
  u8_t PortConfig[4];
  enum port_type type;
  HAL_PORT_LINK_STATE link_state;
  HAL_PORT_SPEED_STATE speed;
  HAL_PORT_DUPLEX_STATE duplex;
  HAL_MDI_MDIX_STATE mdi_mdix;
  u16 regval;
  
  /* the index value can be found in: od->id_inst_ptr[1] */
  index = od->id_inst_ptr[1];
  /* read port configration */
  eeprom_read(NVRAM_PORT_CFG_BASE+(1+(index-1)*4), PortConfig, 4);
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* portIndex  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = index; /* todo: set appropriate value */
      }
      break;
  case 2:    /* portType  */
      {
        s32_t *sint_ptr = value;
        type = (PortConfig[0] >> 6) & 0x01;
        *sint_ptr = type; /* todo: set appropriate value */
      }
      break;
    case 3:    /* portOpMode  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = ( ( (PortConfig[0]>>2)&0x01 ) == 0x00 )?
        MULTI_MODE:SINGLE_MODE; /* todo: set appropriate value */
      }
      break;
    case 4:    /* portOpFiber  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = ( ( (PortConfig[0]>>1)&0x01 ) == 0x00 )?
        SINGLE_FIBER:DOUBLE_FIBER; /* todo: set appropriate value */
      }
      break;
    case 5:    /* portOpTransDist  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = ( ( (PortConfig[1]>>5)&0x07 ) == 0x01 )?_20K:
                    ( ( (PortConfig[1]>>5)&0x07 ) == 0x02 )?_40K:
                    ( ( (PortConfig[1]>>5)&0x07 ) == 0x03 )?_60K:
                    ( ( (PortConfig[1]>>5)&0x07 ) == 0x04 )?_80K:
                        _120K; /* todo: set appropriate value */
      }
      break;
    case 6:    /* portOpType  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = ( ( (PortConfig[1]>>2)&0x07 ) == 0x00 )?FC:
                    ( ( (PortConfig[1]>>2)&0x07 ) == 0x01 )?SC:
                    ( ( (PortConfig[1]>>2)&0x07 ) == 0x02 )?ST:
                        FC; /* todo: set appropriate value */
      }
      break;
  case 7:    /* portElecType  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = ( ( (PortConfig[1])&0x03 ) == 0x00 )?ELECTRIC_RJ45:
                        ELECTRIC_M12; /* todo: set appropriate value */
      }
      break;
  case 8:    /* portEnable  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = (PortConfig[2]>>6)&0x01; /* todo: set appropriate value */
      }
      break;
  case 9:    /* portComMode  */
      {
        s32_t *sint_ptr = value;
        if((PortConfig[3]>>6)&0x01 == 0x01)
        {
            *sint_ptr = AN; /* todo: set appropriate value */
        }else{
            *sint_ptr = ( ( (PortConfig[3]>>3)&0x07 ) == 0x05 )?GFD:
                        ( ( (PortConfig[3]>>3)&0x07 ) == 0x04 )?GHD:
                        ( ( (PortConfig[3]>>3)&0x07 ) == 0x03 )?FFD:
                        ( ( (PortConfig[3]>>3)&0x07 ) == 0x02 )?FHD:
                        ( ( (PortConfig[3]>>3)&0x07 ) == 0x01 )?TFD:
                            THD; /* todo: set appropriate value */
        }
      }
      break;
    case 10:    /* portConnMode  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = ( ( (PortConfig[3]>>1)&0x03 ) == 0x00 )?CONN_MDIX:
                    ( ( (PortConfig[3]>>1)&0x03 ) == 0x01 )?CONN_MDI:
                        CONN_MDIATOU; /* todo: set appropriate value */
      }
      break;
    case 11:    /* portFlowCtrl  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = (PortConfig[3])&0x01; /* todo: set appropriate value */
      }
      break;
    case 12:    /* portNeighDisc  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = (PortConfig[2]>>7)&0x01; /* todo: set appropriate value */
      }
      break;
  case 13:    /* portLinkState  */
      {
        s32_t *sint_ptr = value;
        //port = hal_swif_lport_2_hport(index);
        hal_swif_port_get_link_state(index, &link_state);
        
        *sint_ptr = ((link_state == LINK_UP)? UP : DOWN); /* todo: set appropriate value */
      }
      break;
  case 14:    /* portComState  */
      {
        s32_t *sint_ptr = value;
        //port = hal_swif_lport_2_hport(index);
        hal_swif_port_get_speed(index, &speed);
		hal_swif_port_get_duplex(index, &duplex);
        
        *sint_ptr = (speed == SPEED_1000M && duplex == FULL_DUPLEX) ? GFD:
                    (speed == SPEED_1000M && duplex == HALF_DUPLEX) ? GHD:
                    (speed == SPEED_100M && duplex == FULL_DUPLEX) ? FFD:
                    (speed == SPEED_100M && duplex == HALF_DUPLEX) ? FHD:
                    (speed == SPEED_10M && duplex == FULL_DUPLEX) ? TFD:
                            THD; /* todo: set appropriate value */
      }
      break;
    case 15:    /* portConnState  */
      {
        s32_t *sint_ptr = value;
        //port = hal_swif_lport_2_hport(index);
#if 0
       if(port == 8) {
            gprtGetPagedPhyReg(dev, port, 17, 0, &regval);
            if(regval & 0x0040)
                mdi_mdix = MODE_MDIX;
            else
                mdi_mdix = MODE_MDI;
        } else {
            gprtGetPhyReg(dev, port, SW_PHY_SPEC_STATUS_REG, &regval);
            if(regval & 0x0040)
                mdi_mdix = MODE_MDI;
            else
                mdi_mdix = MODE_MDIX;
        }
#else
        hal_swif_port_get_mdi_mdix(index, &mdi_mdix);       
#endif
        *sint_ptr = ((mdi_mdix == MODE_MDI) ? CONN_MDI : CONN_MDIX); /* todo: set appropriate value */
      }
      break;
  };

}

static u8_t portConfEntry_set_test(struct obj_def *od, u16_t len, void *value)
{
  u8_t id, set_ok;

  /* the index value can be found in: od->id_inst_ptr[1] */
  set_ok = 0;
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 2:    /* portType  */
  /* validate the value argument and set ok  */
      break;
    case 3:    /* portOpMode  */
  /* validate the value argument and set ok  */
      break;
    case 4:    /* portOpFiber  */
  /* validate the value argument and set ok  */
      break;
    case 5:    /* portOpTransDist  */
  /* validate the value argument and set ok  */
      break;
    case 6:    /* portOpType  */
  /* validate the value argument and set ok  */
      break;
    case 7:    /* portElecType  */
  /* validate the value argument and set ok  */
      break;
    case 8:    /* portEnable  */
  /* validate the value argument and set ok  */
      break;
    case 9:    /* portComMode  */
  /* validate the value argument and set ok  */
      break;
    case 10:    /* portConnMode  */
  /* validate the value argument and set ok  */
      break;
    case 11:    /* portFlowCtrl  */
  /* validate the value argument and set ok  */
      break;
    case 12:    /* portNeighDisc  */
  /* validate the value argument and set ok  */
      break;
  };
  return set_ok;
}

static void portConfEntry_set_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
#if 0
  /* the index value can be found in: od->id_inst_ptr[1] */
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 2:    /* portType  */
      {
        s32_t *sint_ptr = value;
         = *sint_ptr;  /* do something with the value */
      }
      break;
    case 3:    /* portOpMode  */
      {
        s32_t *sint_ptr = value;
         = *sint_ptr;  /* do something with the value */
      }
      break;
    case 4:    /* portOpFiber  */
      {
        s32_t *sint_ptr = value;
         = *sint_ptr;  /* do something with the value */
      }
      break;
    case 5:    /* portOpTransDist  */
      {
        s32_t *sint_ptr = value;
         = *sint_ptr;  /* do something with the value */
      }
      break;
    case 6:    /* portOpType  */
      {
        s32_t *sint_ptr = value;
         = *sint_ptr;  /* do something with the value */
      }
      break;
    case 7:    /* portElecType  */
      {
        s32_t *sint_ptr = value;
         = *sint_ptr;  /* do something with the value */
      }
      break;
    case 8:    /* portEnable  */
      {
        s32_t *sint_ptr = value;
         = *sint_ptr;  /* do something with the value */
      }
      break;
    case 9:    /* portComMode  */
      {
        s32_t *sint_ptr = value;
         = *sint_ptr;  /* do something with the value */
      }
      break;
    case 10:    /* portConnMode  */
      {
        s32_t *sint_ptr = value;
         = *sint_ptr;  /* do something with the value */
      }
      break;
    case 11:    /* portFlowCtrl  */
      {
        s32_t *sint_ptr = value;
         = *sint_ptr;  /* do something with the value */
      }
      break;
    case 12:    /* portNeighDisc  */
      {
        s32_t *sint_ptr = value;
         = *sint_ptr;  /* do something with the value */
      }
      break;
  };
  
#endif
}

struct mib_list_rootnode portConfEntry_root = {
  &portConfEntry_get_object_def,
  &portConfEntry_get_value,
  &portConfEntry_set_test,
  &portConfEntry_set_value,
  MIB_NODE_LR,
  0,
  NULL,
  NULL,  0,
};

/* portConfEntry  .1.3.6.1.4.1.42421.1.2.2.1    */
const s32_t portConfEntry_ids[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
struct mib_node* const portConfEntry_nodes[15] = { 
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root,
  (struct mib_node* const)&portConfEntry_root
};

const struct mib_array_node portConfEntry = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  15,
  portConfEntry_ids,
  portConfEntry_nodes
};

/* portConfTable  .1.3.6.1.4.1.42421.1.2.2    */
s32_t portConfTable_ids[1] = { 1 };
struct mib_node* portConfTable_nodes[1] = { 
  (struct mib_node* const)&portConfEntry
};

struct mib_ram_array_node portConfTable = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_RA,
  0,
  portConfTable_ids,
  portConfTable_nodes
};

static void portConf_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
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
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def private portConf.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1:    /* portNumber  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("portConf_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("private portConf_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

static void portConf_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  s32_t port_num;
  hal_port_conf_t PortConfig;
  
  /* Read Configuration from EEPROM */
  if(eeprom_read(NVRAM_PORT_CFG_BASE, (uint8 *)&PortConfig, sizeof(hal_port_conf_t)) != I2C_SUCCESS)
    return;

  port_num = PortConfig.PortNum;
  if(port_num > MAX_PORT_NUM)
    port_num = MAX_PORT_NUM;

  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* portNumber  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = port_num; /* todo: set appropriate value */
      }
      break;
  };
}

static u8_t portConf_set_test(struct obj_def *od, u16_t len, void *value)
{
  u8_t id, set_ok;

  set_ok = 0;
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* portNumber  */
  /* validate the value argument and set ok  */
        set_ok = 1;
      break;
  };
  return set_ok;
}

static void portConf_set_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  s32_t port_num;
  hal_port_conf_t PortConfig;
  
  /* Read Configuration from EEPROM */
  if(eeprom_read(NVRAM_PORT_CFG_BASE, (uint8 *)&PortConfig, sizeof(hal_port_conf_t)) != I2C_SUCCESS)
    return;

  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* portNumber  */
      {
        s32_t *sint_ptr = value;
        port_num = *sint_ptr;  /* do something with the value */
        PortConfig.PortNum = port_num;
        if(eeprom_page_write(NVRAM_PORT_CFG_BASE, (uint8 *)&PortConfig, sizeof(hal_port_conf_t)) != I2C_SUCCESS)
          return;
      }
      break;
  };
}

const mib_scalar_node portConf_scalar = {
  &portConf_get_object_def,
  &portConf_get_value,
  &portConf_set_test,
  &portConf_set_value,
  MIB_NODE_SC,
  0
};

/* portConf  .1.3.6.1.4.1.42421.1.2    */
const s32_t portConf_ids[2] = { 1, 2 };
struct mib_node* const portConf_nodes[2] = { 
  (struct mib_node* const)&portConf_scalar,
  (struct mib_node* const)&portConfTable
};

const struct mib_array_node portConf = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  2,
  portConf_ids,
  portConf_nodes
};

static void devParam_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  u8_t id;
  u8 version[32+1];
  u8 lenth = 0;

  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
  if (ident_len == 2)
  {
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;

    id = ident[0];
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def private devParam.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1:    /* devNum  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        od->v_len = SWITCHTYPE_LEN;// todo:  set the appropriate length eg. sizeof(char_buffer);
        break;
      case 2:    /* ifPhysAddress  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        od->v_len = NETIF_MAX_HWADDR_LEN;
        break;
      case 3:    /* ipSwitch  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 4:    /* ipAddress  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
        od->v_len = sizeof(u32_t);
        break;
      case 5:    /* ipSubMask  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
        od->v_len = sizeof(u32_t);
        break;
      case 6:    /* ipGateway  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
        od->v_len = sizeof(u32_t);
        break;
      case 7:    /* devName  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        eeprom_read(NVRAM_DEVICE_NAME, version, 32);
        lenth = strlen((char *)version);
        if(lenth >= 32)
          lenth = 32;
        od->v_len = lenth;// todo:  set the appropriate length eg. sizeof(char_buffer);
        break;
      case 8:    /* productNum  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        eeprom_read(NVRAM_SERIAL_NUMBER, version, 16);
        lenth = strlen((char *)version);
        if(lenth >= 16)
        lenth = 16;
        od->v_len = lenth;// todo:  set the appropriate length eg. sizeof(char_buffer);
        break;
      case 9:    /* sysVerNum  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        eeprom_read(NVRAM_SYS_VERSION, version, CONF_SYS_VERSION_SIZE);
        lenth = strlen((char *)version);
        if(lenth >= CONF_SYS_VERSION_SIZE)
        lenth = CONF_SYS_VERSION_SIZE;
        od->v_len = lenth;// todo:  set the appropriate length eg. sizeof(char_buffer);
        break;
      case 10:    /* hardVerNum  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        eeprom_read(NVRAM_HARDWARE_VERSION, version, CONF_HARDWARE_VERSION_SIZE);
        lenth = strlen((char *)version);
        if(lenth >= CONF_HARDWARE_VERSION_SIZE)
        lenth = CONF_HARDWARE_VERSION_SIZE;
        od->v_len = lenth;// todo:  set the appropriate length eg. sizeof(char_buffer);
        break;
      case 11:    /* softVerNum  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_WRITE;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        eeprom_read(NVRAM_SOFT_VERSION, version, CONF_SOFT_VERSION_SIZE);
        lenth = strlen((char *)version);
        if(lenth >= CONF_SOFT_VERSION_SIZE)
        lenth = CONF_SOFT_VERSION_SIZE;
        od->v_len = lenth;// todo:  set the appropriate length eg. sizeof(char_buffer);
        break;
      case 12:    /* powMasterVol  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      case 13:    /* powSlaveVol  */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(u32_t);// todo:  set the appropriate length eg. sizeof(u32_t);
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("devParam_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("private devParam_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

static void devParam_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  u8 version[32];
  char lenth = 0;
  struct netif *netif = netif_list;
 
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 1:    /* devNum  */
      BoardType[0] = (u8)((BOARD_TYPE & 0xFF000000) >> 24);
	  BoardType[1] = (u8)((BOARD_TYPE & 0x00FF0000) >> 16);
	  BoardType[2] = (u8)((BOARD_TYPE & 0x0000FF00) >> 8);
	  BoardType[3] = (u8)(BOARD_TYPE & 0x000000FF);
      ocstrncpy(value,(u8_t*)BoardType,SWITCHTYPE_LEN);
      break;
    case 2:    /* ifPhysAddress  */
      ocstrncpy(value,(u8_t *)&netif->hwaddr[0],NETIF_MAX_HWADDR_LEN);
      break;
    case 3:    /* ipSwitch  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = 1; /* todo: set appropriate value */
      }
      break;
    case 4:    /* ipAddress  */
      {
        struct ip_addr *dst = value;
        *dst = netif->ip_addr;
      }
      break;
    case 5:    /* ipSubMask  */
      {
        struct ip_addr *dst = value;
	    *dst = netif->netmask;
      }
      break;
    case 6:    /* ipGateway  */
      {
        struct ip_addr *dst = value;
	    *dst = netif->gw;
      }
      break;
    case 7:    /* devName  */
      eeprom_read(NVRAM_DEVICE_NAME, version, 32);
      lenth = strlen((char *)version);
      if(lenth >= 32)
          lenth = 32;
      ocstrncpy(value,(u8_t*)version,lenth);
      break;
    case 8:    /* productNum  */
      eeprom_read(NVRAM_SERIAL_NUMBER, version, 16);
      lenth = strlen((char *)version);
      if(lenth >= 16)
          lenth = 16;
      ocstrncpy(value,(u8_t*)version,lenth);
      break;
    case 9:    /* sysVerNum  */
      eeprom_read(NVRAM_SYS_VERSION, version, CONF_SYS_VERSION_SIZE);
      lenth = strlen((char *)version);
      if(lenth >= CONF_SYS_VERSION_SIZE)
          lenth = CONF_SYS_VERSION_SIZE;
      ocstrncpy(value,(u8_t*)version,lenth);
      break;
    case 10:    /* hardVerNum  */
      eeprom_read(NVRAM_HARDWARE_VERSION, version, CONF_HARDWARE_VERSION_SIZE);
      lenth = strlen((char *)version);
      if(lenth >= CONF_HARDWARE_VERSION_SIZE)
          lenth = CONF_HARDWARE_VERSION_SIZE;
      ocstrncpy(value,(u8_t*)version,lenth);
      break;
    case 11:    /* softVerNum  */
      eeprom_read(NVRAM_SOFT_VERSION, version, CONF_SOFT_VERSION_SIZE);
      lenth = strlen((char *)version);
      if(lenth >= CONF_SOFT_VERSION_SIZE)
          lenth = CONF_SOFT_VERSION_SIZE;
      ocstrncpy(value,(u8_t*)version,lenth);
      break;
    case 12:    /* powMasterVol  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = 5; /* todo: set appropriate value */
      }
      break;
    case 13:    /* powSlaveVol  */
      {
        s32_t *sint_ptr = value;
        *sint_ptr = 5; /* todo: set appropriate value */
      }
      break;
  };
}

static u8_t devParam_set_test(struct obj_def *od, u16_t len, void *value)
{
  u8_t id, set_ok;

  set_ok = 0;
  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 2:    /* ifPhysAddress  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 3:    /* ipSwitch  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 4:    /* ipAddress  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 5:    /* ipSubMask  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 6:    /* ipGateway  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 7:    /* devName  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 8:    /* productNum  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 9:    /* sysVerNum  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 10:    /* hardVerNum  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
    case 11:    /* softVerNum  */
  /* validate the value argument and set ok  */
      set_ok = 1;
      break;
  };
  return set_ok;
}

static void devParam_set_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  u8_t lenth = 0;
  s32_t ip_switch;

  id = od->id_inst_ptr[0];
  switch (id)
  {
    case 2:    /* ifPhysAddress  */
      eeprom_write(EPROM_ADDR_MAC, value, 6);
      break;
    case 3:    /* ipSwitch  */
      {
        s32_t *sint_ptr = value;
        ip_switch = *sint_ptr;  /* do something with the value */
      }
      break;
    case 4:    /* ipAddress  */
      eeprom_write(EPROM_ADDR_IP, value, 4);
      break;
    case 5:    /* ipSubMask  */
      eeprom_write(EPROM_ADDR_NETMASK, value, 4);
      break;
    case 6:    /* ipGateway  */
      eeprom_write(EPROM_ADDR_GATEWAY, value, 4);
      break;
    case 7:    /* devName  */
       if(len >= 32)
       { 
           eeprom_write(NVRAM_DEVICE_NAME, value, 32);
       }else{
           eeprom_write(NVRAM_DEVICE_NAME, value, len);
           eeprom_write(NVRAM_DEVICE_NAME+len, "\0", 1);
       }
      
      break;
    case 8:    /* productNum  */
        if(len >= 16)
        {
           eeprom_write(NVRAM_SERIAL_NUMBER, value, 16);
        }else{
           eeprom_write(NVRAM_SERIAL_NUMBER, value, len);
           eeprom_write(NVRAM_SERIAL_NUMBER+len, "\0", 1);
        }
      break;
    case 9:    /* sysVerNum  */
       if(len >= 32)
       {
           eeprom_write(NVRAM_SYS_VERSION, value, 32);
       }else{
           eeprom_write(NVRAM_SYS_VERSION, value, len);
           eeprom_write(NVRAM_SYS_VERSION+len, "\0", 1);
       }
      break;
    case 10:    /* hardVerNum  */
       if(len >= 20)
       {
           eeprom_write(NVRAM_HARDWARE_VERSION, value, 20);
       }else{
           eeprom_write(NVRAM_HARDWARE_VERSION, value, len);
           eeprom_write(NVRAM_HARDWARE_VERSION+len, "\0", 1);
       }
      break;
    case 11:    /* softVerNum  */
      if(len >= 20)
      {
          eeprom_write(NVRAM_SOFT_VERSION, value, 20);
      }else
      {
          eeprom_write(NVRAM_SOFT_VERSION, value, len);
          eeprom_write(NVRAM_SOFT_VERSION+len, "\0", 1);
      }
      break;
  };
}

const mib_scalar_node devParam_scalar = {
  &devParam_get_object_def,
  &devParam_get_value,
  &devParam_set_test,
  &devParam_set_value,
  MIB_NODE_SC,
  0
};

/* devParam  .1.3.6.1.4.1.42421.1.1    */
const s32_t devParam_ids[13] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
struct mib_node* const devParam_nodes[13] = { 
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar,
  (struct mib_node* const)&devParam_scalar
};

const struct mib_array_node devParam = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  13,
  devParam_ids,
  devParam_nodes
};

/* obswitch  .1.3.6.1.4.1.42421.1    */
const s32_t obswitch_ids[2] = { 1, 2 };
struct mib_node* const obswitch_nodes[2] = { 
  (struct mib_node* const)&devParam,
  (struct mib_node* const)&portConf
};

const struct mib_array_node obswitch = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  2,
  obswitch_ids,
  obswitch_nodes
};

/* obnet  .1.3.6.1.4.1.42421    */
const s32_t obnet_ids[1] = { 1 };
struct mib_node* const obnet_nodes[1] = { 
  (struct mib_node* const)&obswitch
};

const struct mib_array_node obnet = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  1,
  obnet_ids,
  obnet_nodes
};

/* enterprises  .1.3.6.1.4.1    */
const s32_t enterprises_ids[1] = { 42421 };
struct mib_node* const enterprises_nodes[1] = { 
  (struct mib_node* const)&obnet
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


/* 添加端口表项*/
void snmp_inc_portlist(void)
{
  struct mib_list_node *port_node = NULL;
  s32_t i;
  s32_t port_num;
  hal_port_conf_t PortConfig;
  
  /* Read Configuration from EEPROM */
  if(eeprom_read(NVRAM_PORT_CFG_BASE, (uint8 *)&PortConfig, sizeof(hal_port_conf_t)) != I2C_SUCCESS)
    return;

  port_num = PortConfig.PortNum;
  if(port_num > MAX_PORT_NUM)
    port_num = MAX_PORT_NUM;

  for(i = 0 ; i<port_num ; i++)
  {
    snmp_mib_node_insert(&portConfEntry_root, portConfEntry_root.count + 1, &port_node);
  }
	
  /* enable getnext traversal on filled table */
  portConfTable.maxlength = 1;
}
void
lwip_privmib_init(void)
{
    snmp_inc_portlist();
}

#endif