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
#include "adc.h"

/* ADC SAR sequencer component header to access Vref value */
#include <ADC_SAR_SEQ.h>

/*============================================================================
Defines
============================================================================*/
/* Get actual Vref. value from ADC SAR sequencer */
#define ADC_VREF_VALUE_V    ((float)ADC_SAR_Seq_DEFAULT_VREF_MV_VALUE/1000.0)

/*============================================================================
Global functions
============================================================================*/
void AdcConvIsrHandler(void);

/*============================================================================
Global variables
============================================================================*/
struct _adc AdcData;


void AdcInit()
{
    /* Init and start sequencing SAR ADC */
    ADC_SAR_Seq_Start();
    ADC_SAR_Seq_StartConvert();
    /* Enable interrupt and set interrupt handler to local routine */
    ADC_SAR_Seq_IRQ_StartEx(AdcConvIsrHandler);    
}

void AdcConvStart()
{
    /* Enables injection channel for next scan */
    ADC_SAR_Seq_EnableInjection();
}

void AdcProcess()
{
    int16 ADCCountsCorrected;

    /* When conversion of sequencing channels has completed */
    if((AdcData.dataReady & ADC_SAR_Seq_EOS_MASK) != 0u)
    {
        /* Get voltage, measured by ADC */
        AdcData.dataReady &= ~ADC_SAR_Seq_EOS_MASK;
        AdcData.res = ADC_SAR_Seq_CountsTo_mVolts(CH0_N, AdcData.result[CH0_N]);
    }    
    
    /* When conversion of the injection channel has completed */
    if((AdcData.dataReady & ADC_SAR_Seq_INJ_EOC_MASK) != 0u)
    {
        AdcData.dataReady &= ~ADC_SAR_Seq_INJ_EOC_MASK;

        /******************************************************************************
        * Adjust data from ADC with respect to Vref value.
        * This adjustment is to be done if Vref is set to any other than
        * internal 1.024V.
        * For more detailed description see Functional Description section
        * of DieTemp P4 datasheet.
        *******************************************************************************/
        ADCCountsCorrected = AdcData.result[TEMP_CH]*((int16)((float)ADC_VREF_VALUE_V/1.024));
        AdcData.temperature.val = ( int8_t )DieTemp_CountsTo_Celsius(ADCCountsCorrected);
    }   
}


void AdcConvIsrHandler()
{
    extern struct _adc AdcData;
    uint32 intr_status;
    uint32 range_status;

    /* Read interrupt status registers */
    intr_status = ADC_SAR_Seq_SAR_INTR_MASKED_REG;
    /* Check for End of Scan interrupt */
    if((intr_status & ADC_SAR_Seq_EOS_MASK) != 0u)
    {
        /* Read range detect status */
        range_status = ADC_SAR_Seq_SAR_RANGE_INTR_MASKED_REG;
        /* Verify that the conversion result met the condition Low_Limit <= Result < High_Limit  */
        if((range_status & (uint32)(1ul << CH0_N)) != 0u) 
        {
            /* Read conversion result */
            AdcData.result[CH0_N] = ADC_SAR_Seq_GetResult16(CH0_N);
        }    

        /* Clear range detect status */
        ADC_SAR_Seq_SAR_RANGE_INTR_REG = range_status;
        AdcData.dataReady |= ADC_SAR_Seq_EOS_MASK;
    }    

    /* Check for Injection End of Conversion */
    if((intr_status & ADC_SAR_Seq_INJ_EOC_MASK) != 0u)
    {
        AdcData.result[TEMP_CH] = ADC_SAR_Seq_GetResult16(TEMP_CH);
        AdcData.dataReady |= ADC_SAR_Seq_INJ_EOC_MASK;
    }    

    /* Clear handled interrupt */
    ADC_SAR_Seq_SAR_INTR_REG = intr_status;
}

/* [] END OF FILE */
