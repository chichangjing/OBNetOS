/*************************************************************
 * Filename     : fpga_api.h
 * Description  : API for fpga regester feature  
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/

#ifndef _FPGA_API_H
#define _FPGA_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/* include -------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

typedef enum{
    FPGA_FALSE = 0,
    FPGA_TURE
}FPGA_BOOL;

/* Exported constants --------------------------------------------------------*/
    
/* Exported macro ------------------------------------------------------------*/
/* FPGA ²¿·Ö¼Ä´æÆ÷µØÖ· */
#define FPGA_OPT1_STA_ADDR  0X00 /* bit7 is lock if 1£¬unlock if 0 */
#define FPGA_SDI_DATA_ADDR0 0XF0 /* BT_GV3S_HONUE_QM bit15~bit1 invalid£¬only */
#define FPGA_SDI_DATA_ADDR1 0XF1 /* one SDI port£¬bit0 = 1 if valid data */
#define FPGA_K_SIGNAL_ADDR  0XFA /* Kin signal check bit7 */
#define FPGA_CODE_ID_ADDR   0XFB /* FPGA code id address range 0XFB~0XFE */
/* Exported functions --------------------------------------------------------*/ 
int fpga_get_sdi_status(int sdiport, FPGA_BOOL *state);
int fpga_get_opt_lock(int sdiport, FPGA_BOOL *state);
int fpga_get_k_signal(FPGA_BOOL *state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FPGA_API_H */