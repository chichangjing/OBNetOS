
#ifndef __CLI_SYS_H__
#define __CLI_SYS_H__

#include "mconfig.h"

#include "rc.h"
#include "rc_ignition.h"
#include "rcc.h"
#include "rcc_cli.h"

#ifdef __FreeRTOS_OS__
#include "FreeRTOS.h"
#include "task.h"

#include <stm32f2xx.h>
#endif

typedef struct _ip_info
{
	ubyte	ip[4];
	ubyte	netmask[4];
	ubyte	gateway[4];
}ip_info_t;

/* cli_sys.c */
RLSTATUS cli_exec_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_exec_exit_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_exec_reset_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_task_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_task_runtime_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_config_bootdelay_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_config_ip_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_memory_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_system_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_register_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_version_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_port_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_port_status_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_port_neigbor_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_port_traffic_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_device_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_mac_addr_table_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_priority_queue_map_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_qos_set(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_vlan_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_show_counters_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_clear_counters_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_clear_mac_addr_table_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_config_traffic_statistic_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);

/* cli_ping.c */
RLSTATUS cli_exec_ping_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);

/* cli_tftp.c */
RLSTATUS cli_exec_tftp_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);

/* cli_debug.c */
RLSTATUS cli_debug_switch_getreg_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_debug_switch_setreg_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_debug_eeprom_dump_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_debug_eeprom_clear_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_debug_eeprom_test_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_debug_eeprom_format_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_debug_module_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_debug_login_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_debug_diag_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);

#if MODULE_UART_SERVER
/* cli_uart.c */
RLSTATUS cli_show_uart_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
#endif


/* cli_port.c */
RLSTATUS cli_config_port_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_port_security_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_port_no_security_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_port_security_maximum_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_port_security_macaddr_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);

/* cli_trap.c */
extern RLSTATUS cli_config_trap_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_trap_no_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_trap_server_mac_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_trap_add_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_trap_delete_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_trap_show_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);

#if MODULE_SIGNAL
/* cli_signal.c */
RLSTATUS cli_show_signal_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_jitter_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_work_mode_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_chan_num_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_chan_param_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_signal_enable_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_signal_no_enable_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
RLSTATUS cli_samp_cycle_config_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
#endif
/* cli_signal.c */
RLSTATUS cli_no_support_display_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);

/* cli_obring.h */
extern RLSTATUS cli_config_obring_new_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_obring_delete_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_obring_mode_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_obring_domain_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_obring_domain_ring_port_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_obring_domain_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_obring_domain_disable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_obring_domain_hello_times_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_obring_domain_fail_times_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_obring_domain_primary_port_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_config_obring_domain_primary_port_disable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_show_obring_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_show_obring_topo_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_debug_obring_disable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_debug_obring_enable_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
extern RLSTATUS cli_debug_obring_reboot_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf);
#endif

