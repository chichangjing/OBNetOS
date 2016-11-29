/*************************************************************
 * Filename     : 
 * Description  : 
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/
/* include -------------------------------------------------------------------*/
#include "fpga_debug.h"
#include "stdio.h"
#include "stdarg.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief print message
 * @param 
 * @return none
 */
void St_Print(char* format, ...)
{
    va_list argP;
    static char dbgStr[1000] = "";

    va_start(argP, format);

    vsprintf(dbgStr, format, argP);
    
    printf(dbgStr);
    
    return;

}