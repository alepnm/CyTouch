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
#if !defined(BUFFERS_H)
#define BUFFERS_H
    
#include "user_mb_app.h"
    
/*-----------------------Master mode use these variables----------------------*/
#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
   
//Master mode:DiscreteInputs variables
extern USHORT   usMDiscInStart;
#if (M_DISCRETE_INPUT_NDISCRETES % 8)
    extern UCHAR    ucMDiscInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_DISCRETE_INPUT_NDISCRETES/8+1];
#else
    extern UCHAR    ucMDiscInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_DISCRETE_INPUT_NDISCRETES/8];
#endif

//Master mode:Coils variables
extern USHORT   usMCoilStart;
#if (M_COIL_NCOILS % 8)
    extern UCHAR    ucMCoilBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_COIL_NCOILS/8+1];
#else
    extern UCHAR    ucMCoilBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_COIL_NCOILS/8];
#endif

//Master mode:InputRegister variables
extern USHORT   usMRegInStart;
extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];

//Master mode:HoldingRegister variables
extern USHORT   usMRegHoldStart;
extern USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];

//Master mode: ReportSlaveId variables
extern union SlaveID xMRepSlaveIdBuf[MB_MASTER_TOTAL_SLAVE_NUM];;


extern struct _coils Coils[MB_MASTER_TOTAL_SLAVE_NUM];
extern struct _hregs HRegs[MB_MASTER_TOTAL_SLAVE_NUM];
extern struct _iregs IRegs[MB_MASTER_TOTAL_SLAVE_NUM];
extern struct _dinputs DInputs[MB_MASTER_TOTAL_SLAVE_NUM];



#endif



/*****************************************************************************/
#endif
/* [] END OF FILE */
