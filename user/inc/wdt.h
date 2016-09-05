/* ========================================
 *
 * Copyright VENTMATIKA, 2016
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Ventmatika.
 *
 * ========================================
*/
#ifndef _WDT_H
#define _WDT_H

#include <project.h>
    
    
#define WDT0_RESET      CySysWdtResetCounters(CY_SYS_WDT_COUNTER0_RESET);
#define WDT0_ENABLE     CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK);
#define WDT0_DISABLE    CySysWdtDisable(CY_SYS_WDT_COUNTER0_MASK);

    
/* -------------------------- Global functions ------------------------------*/
void WdtInit(void);
    
#endif /* _WDT_H */ 
/* [] END OF FILE */
