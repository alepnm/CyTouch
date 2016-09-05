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
#if !defined(MBQUEUE_H)
#define MBQUEUE_H

#include <project.h>
#include <stdbool.h>
#include "mb_m.h"
#include "mbunit.h"
    
/* Modbus Queue definitions */
#define MAX_MBQUEUE_LEN     32    
 
typedef struct{
    uint8_t     SlaveAddr;
    uint8_t     MbFunc;
    uint16_t    RegAddr;
    uint16_t    Reg;
    uint16_t    *Buffer;
}mbcmd_t;
typedef struct{
    mbcmd_t Queue[MAX_MBQUEUE_LEN];
    uint8_t QueueTail;    
}mbqdat_t;


/* Modbus funkciju pointeriai */
void (*ptrGetDataFromSlave)(mbdev_t *slave);
void (*ptrSendDataToSlave)( mbdev_t *slave );


//bool (*ptrCmdWriteSpeedReg)( mbdev_t *sl, uint8_t value );
//bool (*ptrCmdWriteTSetReg)( mbdev_t *sl, uint8_t value );
//bool (*ptrCmdWriteBoostCoil)( mbdev_t *sl, bool value );
//bool (*ptrCmdWriteBoostCounterReg)( mbdev_t *sl, uint8_t value );


void MbQueueInit( void );
bool PutMbCmdToEndQueue( mbcmd_t cmd );
eMBMasterReqErrCode ExecMbCmdFromQueue( void );
uint8_t QueueGetTail( void );


bool QueueCmdWriteHoldingReg( uint8_t addr, uint16_t regstart, uint16_t val );
bool QueueCmdReadInputRegs( uint8_t addr, uint16_t regstart, uint16_t nregs );
bool QueueCmdReadDescreetInputs( uint8_t addr, uint16_t regstart, uint16_t nregs );
bool QueueCmdReadCoils( uint8_t addr, uint16_t regstart, uint16_t nregs );
bool QueueCmdReadHoldingRegs( uint8_t addr, uint16_t regstart, uint16_t nregs );
bool QueueCmdWriteCoil( uint8_t addr, uint16_t regstart, bool val );
bool QueueCmdWriteMultipleCoils( uint8_t addr, uint16_t regstart, uint16_t nregs, uint8_t *buffer );
bool QueueCmdReportSlaveId(mbdev_t *sl);
   
/*****************************************************************************/
#endif
/* [] END OF FILE */
