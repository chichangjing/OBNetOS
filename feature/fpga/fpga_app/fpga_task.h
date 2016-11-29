/*************************************************************
 * Filename     : fpga_task.h
 * Description  : API for fpga task
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/

#ifndef _FPGA_TASK_H
#define _FPGA_TASK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/* include */   

/* Exported constants --------------------------------------------------------*/
    
/* Exported macro ------------------------------------------------------------*/
    
/* Exported functions --------------------------------------------------------*/ 
void fpga_task_init(void);
void dev_reset(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* _FPGA_TASK_H */