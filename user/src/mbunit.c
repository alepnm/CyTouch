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

#include "mbunit.h"
#include "buffers.h"

static CYBLE_API_RESULT_T result = CYBLE_ERROR_OK;

/*********************** Modbus Slave Device Typedef *************************/
static uint8_t TotalSlaves = 0;
static mbdev_t slave[MB_MASTER_TOTAL_SLAVE_NUM];



mbdev_t *CreateSlave( uint8_t addr ){
    
    mbdev_t *dev = NULL;
    
    //tikrinam, ar adresas teisingas ir devaiso su tokiu pat adresu nera
    if( addr < MB_ADDRESS_MIN || addr > MB_ADDRESS_MAX || GetSlave( addr ) != NULL ){
        result = CYBLE_ERROR_INVALID_PARAMETER;
        return NULL;
    } 
    
    //tikrinam, ar registruotu devaisu kiekis nevirsyja maksimalaus
    if( TotalSlaves >= MB_MASTER_TOTAL_SLAVE_NUM )
    {
        TotalSlaves = MB_MASTER_TOTAL_SLAVE_NUM;
        result = CYBLE_ERROR_INSUFFICIENT_RESOURCES;
        return NULL;
    }
    
    dev = slave + TotalSlaves;
    
    dev->Data.addr = addr;
    dev->Data.BoostCounter = (0u);
    
    dev->Buffers.SlaveId = &xMRepSlaveIdBuf[TotalSlaves];    
    dev->Buffers.Coils = ucMCoilBuf[TotalSlaves];
    dev->Buffers.HoldingRegisters = usMRegHoldBuf[TotalSlaves];
    dev->Buffers.DiscreteInputs = ucMDiscInBuf[TotalSlaves];
    dev->Buffers.InputRegisters = usMRegInBuf[TotalSlaves]; 
    
    
    
    dev->ShortBuffers.Coils = &Coils[TotalSlaves];
    Coils[TotalSlaves].Tail = (0u);
    
    dev->ShortBuffers.HoldingRegisters = &HRegs[TotalSlaves];
    HRegs[TotalSlaves].Tail = (0u);
    
    dev->ShortBuffers.DiscreteInputs = &DInputs[TotalSlaves];
    DInputs[TotalSlaves].Tail = (0u);
    
    dev->ShortBuffers.InputRegisters = &IRegs[TotalSlaves];
    IRegs[TotalSlaves].Tail = (0u);

    
    dev->Status.Type = UNKNOWN;
    dev->Status.IsAlive = false;
    dev->Status.IsRecognized = false;
    dev->Status.ConnectionStateCounter = (0u);    
    
    TotalSlaves++;
  
    return dev;
}
mbdev_t *GetSlave(uint8_t addr){
    uint8_t i = 0;
    mbdev_t *dev;
    
    while( TotalSlaves > (0u) && i <= TotalSlaves - (1u) )
    {        
        dev = slave + i; 
        
        if( dev->Data.addr == addr ) return dev;        
        i++;
    }    
    
    return NULL;
}

void CheckPrvType(mbdev_t *dev){
    uint8_t ver = 0;
    
    uint8_t isr = CyEnterCriticalSection();    
    
    if( dev->Buffers.SlaveId->Add[0] == 'P' && dev->Buffers.SlaveId->Add[1] == 'R' && dev->Buffers.SlaveId->Add[2] == 'V'){
        
        ver = (dev->Buffers.SlaveId->Add[7] - 0x30) * 10 + (dev->Buffers.SlaveId->Add[9] - 0x30);
        
        if( ver < 24 || ver > 28 ){
            dev->Status.IsRecognized = false;
            dev->Status.Type = UNKNOWN;
            goto quit;
        }        
        
        if( dev->Buffers.SlaveId->Add[5] == 'E' ){
            dev->Status.IsRecognized = true;
            dev->Status.Type = PRVE;
        }
        
        if( dev->Buffers.SlaveId->Add[5] == 'W' ){
            dev->Status.IsRecognized = true;
            dev->Status.Type = PRVW;
        }        
    }
    
    if( dev->Buffers.SlaveId->Add[0] == 'K' && dev->Buffers.SlaveId->Add[1] == 'E' ){
        if( dev->Buffers.SlaveId->Add[3] == 'E' ){
            dev->Status.IsRecognized = true;
            dev->Status.Type = VENTIKE;
        }
        
        if( dev->Buffers.SlaveId->Add[3] == 'W' ){
            dev->Status.IsRecognized = true;
            dev->Status.Type = VENTIKW;
        }        
    }
   
quit:
    CyExitCriticalSection(isr);
}
void ClearSlaveData( mbdev_t *dev ){
    
    uint8_t count = 0;
    uint8_t i = 0;
    
    count = sizeof(dev->Buffers.SlaveId->data);
    
    dev->Status.IsAlive = false;
    dev->Status.IsRecognized = false;
    
    while( count-- ) { dev->Buffers.SlaveId->data[i++] = 0u; }
}

/* [] END OF FILE */
