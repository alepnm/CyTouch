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
#include "mbqueue.h"
#include "mbunit.h"
#include "mbutils.h"
#include "leds.h"


extern USHORT   usMRegHoldStart;
extern USHORT   usMCoilStart;
extern USHORT   usMRegInStart;
extern USHORT   usMDiscInStart;

extern cydata_t DevData;
extern mbdev_t  *prv;
extern bool     PrvDataPrepared;


/**************************** Static functions *******************************/
static mbqdat_t MbQueueData;
static mbcmd_t GetMbCmdFromQueue( void );


/*********************** Modbus Queue Implementation *************************/
void MbQueueInit(){
    
    uint8_t i = ( 0u );
    
    while( i < MAX_MBQUEUE_LEN)
    {
        MbQueueData.Queue[i].SlaveAddr = ( 0u );
        MbQueueData.Queue[i].MbFunc = ( 0u );
        MbQueueData.Queue[i].RegAddr = ( 0u );
        MbQueueData.Queue[i].Reg = ( 0u );
        
        i++;
    }
    
    MbQueueData.QueueTail = ( 0u );
}
bool PutMbCmdToEndQueue( mbcmd_t cmd ){
    bool result = false;    
    
    uint8_t isr = CyEnterCriticalSection();
    
    if( MbQueueData.QueueTail < MAX_MBQUEUE_LEN )
    {
        MbQueueData.Queue[MbQueueData.QueueTail] = cmd;
        MbQueueData.QueueTail++;
        result = true;
    }
    
    CyExitCriticalSection( isr );
    
    return result;
}
eMBMasterReqErrCode ExecMbCmdFromQueue(){
    volatile static mbcmd_t cmd;
    volatile eMBMasterReqErrCode result = MB_MRE_MASTER_BUSY;
    
    if( xMBMasterGetIsBusy( ) == true ) return MB_MRE_MASTER_BUSY;          // masteris uzimtas
    if( GetException() == MB_EX_SLAVE_BUSY ){
        result = MB_MRE_SLAVE_EXCE;    // slaivas uzimtas (exception), kartojam paskutine komanda
    }else{    
        if( MbQueueData.QueueTail > (0u) ) cmd = GetMbCmdFromQueue();   // imam sekancia komanda is mb eiles
        else return MB_MRE_NO_RES;     // tuscia eile   
    }
    
    uint8_t isr = CyEnterCriticalSection(); 
    
    if( GetException() == MB_EX_NONE ){
        if( prv->Status.ConnectionStateCounter++ >= MB_M_CONNECT_NTEST ){
            prv->Status.IsAlive = false;
            prv->Status.IsRecognized = false;
            PrvDataPrepared = false;
        }
    }
    
    switch(cmd.MbFunc){
        case MB_FUNC_READ_COILS:
        result = eMBMasterReqReadCoils( cmd.SlaveAddr, cmd.RegAddr, cmd.Reg );
        break;
        case MB_FUNC_READ_DISCRETE_INPUTS:
        result = eMBMasterReqReadDiscreteInputs( cmd.SlaveAddr, cmd.RegAddr, cmd.Reg );
        break;            
        case MB_FUNC_READ_HOLDING_REGISTER:
        result = eMBMasterReqReadHoldingRegister( cmd.SlaveAddr, cmd.RegAddr, cmd.Reg );
        break;
        case MB_FUNC_READ_INPUT_REGISTER:
        result = eMBMasterReqReadInputRegister( cmd.SlaveAddr, cmd.RegAddr, cmd.Reg );
        break;
        case MB_FUNC_WRITE_SINGLE_COIL:
        result = eMBMasterReqWriteCoil( cmd.SlaveAddr, cmd.RegAddr, cmd.Reg );
        break;
        case MB_FUNC_WRITE_REGISTER:
        result = eMBMasterReqWriteHoldingRegister( cmd.SlaveAddr, cmd.RegAddr, cmd.Reg );
        break;
        case MB_FUNC_WRITE_MULTIPLE_COILS:
        result = eMBMasterReqWriteMultipleCoils( cmd.SlaveAddr, cmd.RegAddr, cmd.Reg, (uint8_t *)cmd.Buffer );
        break;
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
        result = eMBMasterReqWriteMultipleHoldingRegister( cmd.SlaveAddr, cmd.RegAddr, cmd.Reg, cmd.Buffer );
        break;   
        case MB_FUNC_OTHER_REPORT_SLAVEID:
        result = eMBMasterReqReportSlaveId( cmd.SlaveAddr );
        break;
        default:
        break;
    }
    
    CyExitCriticalSection(isr);
    
    return result;
}

uint8_t QueueGetTail(){
    return  MbQueueData.QueueTail;
}
static mbcmd_t GetMbCmdFromQueue(){
    
    uint8_t i = 0;    
    uint8_t isr = CyEnterCriticalSection();
    
    mbcmd_t cmd = MbQueueData.Queue[0];
    
    while(i < MAX_MBQUEUE_LEN - 1)
    {
        MbQueueData.Queue[i] = MbQueueData.Queue[i+1];
        i++;
    }
    
    MbQueueData.QueueTail--;
    
    CyExitCriticalSection(isr);
    
    return cmd;
}



bool QueueCmdReportSlaveId( mbdev_t *sl ){
    mbcmd_t cmd;
    
    cmd.MbFunc = MB_FUNC_OTHER_REPORT_SLAVEID;
    cmd.SlaveAddr = sl->Data.addr;
    cmd.Buffer = (uint16_t*)sl->Buffers.SlaveId->Add;
    return PutMbCmdToEndQueue(cmd);
}
bool QueueCmdReadHoldingRegs( uint8_t addr, uint16_t regstart, uint16_t nregs ){
    mbcmd_t cmd; 
    
    cmd.MbFunc = MB_FUNC_READ_HOLDING_REGISTER;
    cmd.SlaveAddr = addr;
    cmd.RegAddr = usMRegHoldStart + regstart;
    cmd.Reg = nregs;
    cmd.Buffer = NULL;
    
    return PutMbCmdToEndQueue(cmd);
}
bool QueueCmdWriteHoldingReg( uint8_t addr, uint16_t regstart, uint16_t val ){
    mbcmd_t cmd; 
    
    cmd.MbFunc = MB_FUNC_WRITE_REGISTER;
    cmd.SlaveAddr = addr;
    cmd.RegAddr = usMRegHoldStart + regstart;
    cmd.Reg = val;
    cmd.Buffer = NULL;
    
    return PutMbCmdToEndQueue(cmd);
}
bool QueueCmdReadInputRegs( uint8_t addr, uint16_t regstart, uint16_t nregs ){
    mbcmd_t cmd;
    
    cmd.MbFunc = MB_FUNC_READ_INPUT_REGISTER;
    cmd.SlaveAddr = addr;
    cmd.RegAddr = usMRegInStart + regstart;
    cmd.Reg = nregs;
    cmd.Buffer = NULL;
    
    return PutMbCmdToEndQueue(cmd);
}
bool QueueCmdReadDescreetInputs( uint8_t addr, uint16_t regstart, uint16_t nregs ){
    mbcmd_t cmd;
    
    cmd.MbFunc = MB_FUNC_READ_DISCRETE_INPUTS;
    cmd.SlaveAddr = addr;
    cmd.RegAddr = usMDiscInStart + regstart;
    cmd.Reg = nregs;
    cmd.Buffer = NULL;
    
    return PutMbCmdToEndQueue(cmd);
}
bool QueueCmdReadCoils( uint8_t addr, uint16_t regstart, uint16_t nregs ){
    mbcmd_t cmd;
    
    cmd.MbFunc = MB_FUNC_READ_COILS;
    cmd.SlaveAddr = addr;
    cmd.RegAddr = usMCoilStart + regstart;
    cmd.Reg = nregs;  
    cmd.Buffer = NULL;
    
    return PutMbCmdToEndQueue(cmd);
}
bool QueueCmdWriteCoil( uint8_t addr, uint16_t regstart, bool val ){
    mbcmd_t cmd;
    
    cmd.MbFunc = MB_FUNC_WRITE_SINGLE_COIL;
    cmd.SlaveAddr = addr;
    cmd.RegAddr = usMCoilStart + regstart;
    cmd.Reg = (val) ? 0xFF00 : 0x0000;
    cmd.Buffer = NULL;
    
    return PutMbCmdToEndQueue(cmd);
}
bool QueueCmdWriteMultipleCoils( uint8_t addr, uint16_t regstart, uint16_t nregs, uint8_t *buffer ){
    mbcmd_t cmd;
    
    cmd.MbFunc = MB_FUNC_WRITE_MULTIPLE_COILS;
    cmd.SlaveAddr = addr;
    cmd.RegAddr = usMCoilStart + regstart;
    cmd.Reg = nregs;  
    cmd.Buffer = (uint16_t *)buffer;
    
    return PutMbCmdToEndQueue(cmd);
}



/* [] END OF FILE */
