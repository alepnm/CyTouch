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
#ifndef ADC_H
#define ADC_H
#include <project.h>    
    
    
/* ADC defines */
#define CH0_N       (0x00u)
#define TEMP_CH     (0x01u)

    
/*============================================================================
Typedefs
============================================================================*/    
struct _adc
{
    union{
        uint8_t data[4];
        int8_t val;
    }temperature;
    int16_t res;
    __IO uint32_t dataReady;
    __IO int16_t result[ADC_SAR_Seq_TOTAL_CHANNELS_NUM];
};

/*============================================================================
Global functions
============================================================================*/
void AdcInit(void);
void AdcConvStart(void);
void AdcProcess(void);

#endif /* ADC_H */
/* [] END OF FILE */
