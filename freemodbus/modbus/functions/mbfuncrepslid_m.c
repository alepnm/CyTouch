/* 
 * FreeModbus Libary: A portable Modbus implementation for Modbus ASCII/RTU.
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * File: $Id: mbfuncholding_m.c,v 1.60 2013/09/02 14:13:40 Armink Add Master Functions  Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbframe.h"

#include "mbunit.h"

/* ----------------------- Defines ------------------------------------------*/

#define MB_PDU_FUNC_REPORT_SLAVE_ID_SIZE_MIN        ( 2 )
#define MB_PDU_FUNC_REPORT_SLAVE_ID_BYTECNT_OFF     ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_REPORT_SLAVE_ID_VALUES_OFF      ( MB_PDU_DATA_OFF + 1 )

/* ----------------------- Static variables ---------------------------------*/
/* ----------------------- Static functions ---------------------------------*/
/* ----------------------- Start implementation -----------------------------*/
#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0

eMBMasterReqErrCode
eMBMasterReqReportSlaveId( UCHAR ucSndAddr )
{
    UCHAR                 *ucMBFrame;
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;

    if( xMBMasterGetIsBusy( ) ) return MB_MRE_MASTER_BUSY;

    if( GetSlave( ucSndAddr ) == NULL ) return MB_MRE_ILL_ARG;

	vMBMasterGetPDUSndBuf(&ucMBFrame);
	vMBMasterSetDestAddress(ucSndAddr);
    ucMBFrame[MB_PDU_FUNC_OFF]                = MB_FUNC_OTHER_REPORT_SLAVEID; 
    vMBMasterSetPDUSndLength( MB_PDU_SIZE_MIN );
	( void ) xMBMasterPortEventPost( EV_MASTER_FRAME_SENT );    
        
    CleanSlaveIdBuffer(); 
    
    return eErrStatus;
}


eMBException
eMBMasterFuncReportSlaveId( UCHAR * pucFrame, USHORT * usLen )
{
    UCHAR           usRegReadCount;
    UCHAR           *ucMBFrame;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen >= MB_PDU_FUNC_REPORT_SLAVE_ID_SIZE_MIN)
    {
    	vMBMasterGetPDUSndBuf(&ucMBFrame);

        usRegReadCount = pucFrame[MB_PDU_FUNC_REPORT_SLAVE_ID_BYTECNT_OFF];
        
		eRegStatus = eMBReportSlaveIdCB( &pucFrame[MB_PDU_FUNC_REPORT_SLAVE_ID_VALUES_OFF], usRegReadCount );
    }
    
    return eStatus;
}



#endif //MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0
#endif //MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0