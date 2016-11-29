#include "lwip/opt.h"
 
#if LWIP_SNMP
 
#include "lwip/inet.h"
#include "lwip/snmp_msg.h"
#include "lwip/snmp_asn1.h"
#include "lwip/tcpip.h"
//#include "private_trap.h"
#include "string.h"
#include "task.h"
 
#define mainSNMP_Trap_TASK_PRIORITY    ( tskIDLE_PRIORITY + 3 )

#define NUM_PRIVATE_TRAP_LIST    5

#define MY_SNMP_ENTERPRISE_ID 33444
#define MY_SNMP_SYSOBJID_LEN 7
#define MY_SNMP_SYSOBJID {1, 3, 6, 1, 4, 1, MY_SNMP_ENTERPRISE_ID}


 
static unsigned char SNMP_TRAP_0_FLAG = 1;
static struct ip_addr SNMP_TRAP_0_ADDR;
 
extern struct snmp_msg_trap trap_msg;
 
struct trap_list
{
   struct snmp_varbind_root vb_root;
   struct snmp_obj_id *ent_oid;
   s32_t spc_trap;
   u8_t in_use;   
};
 
struct trap_list trap_list_bank[NUM_PRIVATE_TRAP_LIST];
xSemaphoreHandle getTrapListSema = NULL;
 
void vSNMP_Trap_Init()
{
   /* Set SNMP Trap destination */
   IP4_ADDR( &SNMP_TRAP_0_ADDR, 192, 168, 2, 40);
   snmp_trap_dst_ip_set(0,&SNMP_TRAP_0_ADDR);
   snmp_trap_dst_enable(0,SNMP_TRAP_0_FLAG);
   
   getTrapListSema = sys_sem_new(0);
}
 
struct trap_list * getNextFreePrivateTrapList()
{
   u8_t index;
   struct trap_list * result = NULL;
   
   xSemaphoreTake(getTrapListSema, (portTickType) 10);
   
   for(index = 0; index < NUM_PRIVATE_TRAP_LIST; index++)
   {
      if(!trap_list_bank[index].in_use)
      {
         trap_list_bank[index].in_use = 1;
         result = &trap_list_bank[index];         
         break;
      }
   }
   
   xSemaphoreGive(getTrapListSema);
   
   return result;  
}
 
void freePrivateTrapList(struct trap_list * list)
{
   snmp_varbind_list_free(&list->vb_root);
   list->ent_oid = NULL;
   list->spc_trap = 0;
   list->in_use = 0;
}
 
void vSendTrapCallback( void * parameters )
{
   struct trap_list * param;
   
   if( parameters != NULL )
   {
      param = (struct trap_list *) parameters;
      
      trap_msg.outvb = param->vb_root;
      
      snmp_send_trap(SNMP_GENTRAP_ENTERPRISESPC,
                     param->ent_oid,
                     param->spc_trap);
                     
      freePrivateTrapList(param);
   }
}

 
void vSendTrapTaskDemo( void * pvParameters )
{
   portTickType xLastWakeTime;
   struct snmp_obj_id objid = {MY_SNMP_SYSOBJID_LEN, MY_SNMP_SYSOBJID};
   static unsigned char msg[]  = "Alexandre_Malo-mpbc_ca";
   static unsigned char msg2[] = "salut simon, c'est mal";
   static unsigned char msglen= 22;
   
   struct snmp_varbind *vb;
   struct trap_list *vb_list;
   
   (void) pvParameters;
   

   for ( ;; )
   {
      xLastWakeTime = xTaskGetTickCount();
      
      vb_list = getNextFreePrivateTrapList();
      vb_list->ent_oid = &objid;
      vb_list->spc_trap = 12;
      
      vb = snmp_varbind_alloc(&objid, SNMP_ASN1_OPAQUE, msglen);
      
      if (vb != NULL)
      {
         memcpy (vb->value, &msg, msglen);         
         snmp_varbind_tail_add(&vb_list->vb_root,vb);
      }
      
      vb = snmp_varbind_alloc(&objid, SNMP_ASN1_OPAQUE, msglen);
      
      if (vb != NULL)
      {
         memcpy (vb->value, &msg2, msglen);         
         snmp_varbind_tail_add(&vb_list->vb_root,vb);
      }

      tcpip_callback(vSendTrapCallback, vb_list);
      
      // Wait for the next cycle.
      vTaskDelayUntil( &xLastWakeTime, 5000);
   }
   
}

 /* Start SendTrapTask server */ 
void SendTrapTaskInit(void)
{
    vSNMP_Trap_Init( );
    /* Start SendTrapTask server */
    sys_thread_new("tSnmpTrap", vSendTrapTaskDemo, NULL, configMINIMAL_STACK_SIZE,mainSNMP_Trap_TASK_PRIORITY );

}   


#endif//LWIP_SNMP