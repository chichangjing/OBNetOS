#ifndef _FPGA_DEBUG_H
#define _FPGA_DEBUG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"

/* Exported types ------------------------------------------------------------*/
   
/*define debug level*/
#define ST_FPGA_DEBUG  4
#define ST_FPGA_INFO   3
#define ST_FPGA_ERROR  2
#define ST_FPGA_FATAL  1
#define ST_FPGA_LEVEL  4

/*enable print*/ 
//#define ST_FPGA_DEBUG_ENABLE
#ifdef ST_FPGA_DEBUG_ENABLE     
/*it's must less than 1000byte each print*/   
#define st_fpga_debug(level,s,params...) do{ if(level <= ST_FPGA_LEVEL) \
   St_Print(s"\n",##params);}while(0);
   //St_Print("[Function:%s,Line:%d]:"s"\n",__FUNCTION__,__LINE__,##params);}while(0);
#else
#define st_fpga_debug(level,s,params...)
#endif //ST_FPGA_DEBUG_ENABLE   

/* Exported functions --------------------------------------------------------*/     
void St_Print(char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* _FPGA_DEBUG_H */