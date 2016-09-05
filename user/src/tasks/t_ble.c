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
#include "bleapi.h"




extern bool DeviceConnected;
extern bool BleDataPrepareRequired;

void T_Ble(){
    
    static uint32_t     delay = ( 0u ); 
    
    /*******************************************************************
    *  Process all pending BLE events in the stack
    *******************************************************************/      
    if( delay < GetTicks() ){       
        delay = GetTicks() + (100u);
        
        if( DeviceConnected == TRUE )
    	{                       
            if( BleDataPrepareRequired == TRUE ) BleDataPrepare();
            
            UpdateAllCharacteristics(); 
            
            ( void )PrvDataOverNotification( );   
//            ( void )DebugDataOverNotification( );
        }
    }
}


/* [] END OF FILE */
