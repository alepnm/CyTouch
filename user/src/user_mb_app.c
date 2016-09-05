#include "math.h"
#include "user_mb_app.h"
#include "main.h"
#include "prv_codes.h"
#include "mbunit.h"

/*------------------------Slave mode use these variables----------------------*/

//Slave mode:DiscreteInputs variables
USHORT   usSDiscInStart = S_DISCRETE_INPUT_START;
#if (S_DISCRETE_INPUT_NDISCRETES % 8)
	UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8+1];
#else
	UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8];
#endif

//Slave mode:Coils variables
USHORT   usSCoilStart = S_COIL_START;
#if (S_COIL_NCOILS % 8)
	UCHAR    ucSCoilBuf[S_COIL_NCOILS/8+1];
#else
	UCHAR    ucSCoilBuf[S_COIL_NCOILS/8];
#endif

//Slave mode:InputRegister variables
USHORT   usSRegInStart = S_REG_INPUT_START;
USHORT   usSRegInBuf[S_REG_INPUT_NREGS];

//Slave mode:HoldingRegister variables
USHORT   usSRegHoldStart = S_REG_HOLDING_START;
USHORT   usSRegHoldBuf[S_REG_HOLDING_NREGS];

/*-----------------------Master mode use these variables----------------------*/
#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
   
//Master mode:DiscreteInputs variables
USHORT   usMDiscInStart = M_DISCRETE_INPUT_START;
#if (M_DISCRETE_INPUT_NDISCRETES % 8)
    UCHAR    ucMDiscInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_DISCRETE_INPUT_NDISCRETES/8+1];
#else
    UCHAR    ucMDiscInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_DISCRETE_INPUT_NDISCRETES/8];
#endif

//Master mode:Coils variables
USHORT   usMCoilStart = M_COIL_START;
#if (M_COIL_NCOILS % 8)
    UCHAR    ucMCoilBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_COIL_NCOILS/8+1];
#else
    UCHAR    ucMCoilBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_COIL_NCOILS/8];
#endif

//Master mode:InputRegister variables
USHORT   usMRegInStart = M_REG_INPUT_START;
USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];

//Master mode:HoldingRegister variables
USHORT   usMRegHoldStart = M_REG_HOLDING_START;
USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];

//Master mode: ReportSlaveId variables
union SlaveID xMRepSlaveIdBuf[MB_MASTER_TOTAL_SLAVE_NUM];

struct _coils Coils[MB_MASTER_TOTAL_SLAVE_NUM];
struct _hregs HRegs[MB_MASTER_TOTAL_SLAVE_NUM];
struct _iregs IRegs[MB_MASTER_TOTAL_SLAVE_NUM];
struct _dinputs DInputs[MB_MASTER_TOTAL_SLAVE_NUM];

struct _coils *pCoils = NULL;
struct _hregs *pHRegs = NULL;
struct _iregs *pIRegs = NULL;
struct _dinputs *pDInputs = NULL;


#endif


extern mbdev_t *prv;


static void CoilsProcOneReregister(UCHAR * pucRegBuffer, USHORT usAddress);
static void DInputsProcOneReregister(UCHAR * pucRegBuffer, USHORT usAddress);
static void HRegsProcOneReregister( UCHAR * pucRegBuffer, USHORT usAddress );
static void IRegsProcOneReregister(UCHAR * pucRegBuffer, USHORT usAddress);


/************************ Modbus Callback Functions **************************/
eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ){
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT *        pusRegInputBuf;
    UCHAR           REG_INPUT_START;
    USHORT          REG_INPUT_NREGS;
    UCHAR           usRegInStart;
    
    //Determine the master or slave
    if (xMBMasterGetCBRunInMasterMode())
    {
        mbdev_t *sl = GetSlave(ucMBMasterGetDestAddress());
        
        pIRegs = sl->ShortBuffers.InputRegisters;

        sl->Status.ConnectionStateCounter = 0;                
        sl->Status.IsAlive = true;
        pusRegInputBuf = sl->Buffers.InputRegisters;        
        
    	REG_INPUT_START = M_REG_INPUT_START;
    	REG_INPUT_NREGS = M_REG_INPUT_NREGS;
    	usRegInStart = usMRegInStart;
    }
    else
    {
    	pusRegInputBuf = usSRegInBuf;
    	REG_INPUT_START = S_REG_INPUT_START;
    	REG_INPUT_NREGS = S_REG_INPUT_NREGS;
    	usRegInStart = usSRegInStart;
    }

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        uint16_t iRegIndex = ( uint16_t )( usAddress - usRegInStart );        
        
        /* jai registras vienas, rasom jo reiksme i registru masyva IRegs[x] */
        if( usNRegs == 1u ) IRegsProcOneReregister(pucRegBuffer, usAddress);        
        
        while( usNRegs > 0 )
        {
            //Determine the master or slave
            if (xMBMasterGetCBRunInMasterMode())
            {
            	pusRegInputBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegInputBuf[iRegIndex] |= *pucRegBuffer++;
            }
            else
            {
				*pucRegBuffer++ = ( unsigned char )( pusRegInputBuf[iRegIndex] >> 8 );
				*pucRegBuffer++ = ( unsigned char )( pusRegInputBuf[iRegIndex] & 0xFF );
            }
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode ){
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT *        pusRegHoldingBuf;
    UCHAR           REG_HOLDING_START;
    USHORT          REG_HOLDING_NREGS;
    UCHAR           usRegHoldStart;
    
    //Determine the master or slave
    if (xMBMasterGetCBRunInMasterMode())
    {
        mbdev_t *sl = GetSlave(ucMBMasterGetDestAddress());
        
        pHRegs = sl->ShortBuffers.HoldingRegisters;

        sl->Status.ConnectionStateCounter = 0;                
        sl->Status.IsAlive = true;
        pusRegHoldingBuf = sl->Buffers.HoldingRegisters;        
        
    	REG_HOLDING_START = M_REG_HOLDING_START;
    	REG_HOLDING_NREGS = M_REG_HOLDING_NREGS;
    	usRegHoldStart = usMRegHoldStart;
    	//If mode is read,the master will write the received data to buffer.
    	eMode = MB_REG_WRITE;
    }
    else
    {
    	pusRegHoldingBuf = usSRegHoldBuf;
    	REG_HOLDING_START = S_REG_HOLDING_START;
    	REG_HOLDING_NREGS = S_REG_HOLDING_NREGS;
    	usRegHoldStart = usSRegHoldStart;
    }

//    if( ( usAddress >= REG_HOLDING_START ) &&
//        ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )    
    if( usAddress >= REG_HOLDING_START )
    {
        uint16_t iRegIndex = ( uint16_t )( usAddress - usRegHoldStart );
        
        switch ( eMode )
        {
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
				*pucRegBuffer++ = ( unsigned char )( pusRegHoldingBuf[iRegIndex] >> 8 );
				*pucRegBuffer++ = ( unsigned char )( pusRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE: 
            
            /* jai registras vienas, rasom jo reiksme i registru masyva HRegs[x] */
            if( usNRegs == 1u ) HRegsProcOneReregister( pucRegBuffer, usAddress );       
            
            /* kelis nuskaitytus registrus perkeliam i pagrindini masyva */
            while( usNRegs > 0 )
            {
                pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                
                // gal isvalom ir priemimo buferi iskarto?..
                
                iRegIndex++;
                usNRegs--;
            }
            break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode ){
    eMBErrorCode    eStatus = MB_ENOERR;
	UCHAR *         pucCoilBuf;
    UCHAR           COIL_START;
    USHORT          COIL_NCOILS;
    UCHAR           usCoilStart;
    
    uint16_t iNReg =  usNCoils / 8 + 1;
    
    //Determine the master or slave
    if (xMBMasterGetCBRunInMasterMode())
    {
        mbdev_t *sl = GetSlave(ucMBMasterGetDestAddress());

        sl->Status.ConnectionStateCounter = 0;                
        sl->Status.IsAlive = true;
        pucCoilBuf = sl->Buffers.Coils;
        
    	COIL_START = M_COIL_START;
    	COIL_NCOILS = M_COIL_NCOILS;
    	usCoilStart = usMCoilStart;
    	//If mode is read,the master will write the received data to buffer.
    	eMode = MB_REG_WRITE;
    }
    else
    {
    	pucCoilBuf = ucSCoilBuf;
    	COIL_START = S_COIL_START;
    	COIL_NCOILS = S_COIL_NCOILS;
    	usCoilStart = usSCoilStart;
    }

    if( ( usAddress >= COIL_START ) &&
        ( usAddress + usNCoils <= COIL_START + COIL_NCOILS ) )
    {
        uint16_t iRegIndex    = ( uint16_t )( usAddress - usCoilStart ) / 8 ;
		uint16_t iRegBitIndex = ( uint16_t )( usAddress - usCoilStart ) % 8 ;
        
        switch ( eMode )
        {
        /* Pass current coil values to the protocol stack. */
        case MB_REG_READ:
            while( iNReg > 0 )
            {
				*pucRegBuffer++ = xMBUtilGetBits(&pucCoilBuf[iRegIndex++] , iRegBitIndex , 8);
                iNReg --;
            }
			pucRegBuffer --;
			usNCoils = usNCoils % 8;
			*pucRegBuffer = *pucRegBuffer <<(8 - usNCoils);
			*pucRegBuffer = *pucRegBuffer >>(8 - usNCoils);
            break;

            /* Update current coil values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
            while(iNReg > 1)
            {
				xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, 8, *pucRegBuffer++);
                iNReg--;
            }
            
			usNCoils = usNCoils % 8;
            
			if (usNCoils != 0)
			{
				xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, usNCoils,
						*pucRegBuffer++);
			}
			break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete ){
    eMBErrorCode    eStatus = MB_ENOERR;
	UCHAR *         pucDiscreteInputBuf;
    UCHAR           DISCRETE_INPUT_START;
    USHORT          DISCRETE_INPUT_NDISCRETES;
    UCHAR           usDiscreteInputStart;
	
    uint16_t iNReg =  usNDiscrete / 8 + 1;
 
    //Determine the master or slave
    if (xMBMasterGetCBRunInMasterMode())
    {
        mbdev_t *sl = GetSlave(ucMBMasterGetDestAddress());

        sl->Status.ConnectionStateCounter = 0;                
        sl->Status.IsAlive = true;
        pucDiscreteInputBuf = sl->Buffers.DiscreteInputs;       
        
    	DISCRETE_INPUT_START = M_DISCRETE_INPUT_START;
    	DISCRETE_INPUT_NDISCRETES = M_DISCRETE_INPUT_NDISCRETES;
    	usDiscreteInputStart = usMDiscInStart;
    }
    else
    {
    	pucDiscreteInputBuf = ucSDiscInBuf;
    	DISCRETE_INPUT_START = S_DISCRETE_INPUT_START;
    	DISCRETE_INPUT_NDISCRETES = S_DISCRETE_INPUT_NDISCRETES;
    	usDiscreteInputStart = usSDiscInStart;
    }

    if( ( usAddress >= DISCRETE_INPUT_START )
        && ( usAddress + usNDiscrete <= DISCRETE_INPUT_START + DISCRETE_INPUT_NDISCRETES ) )
    {
        uint16_t iRegIndex    = ( uint16_t )( usAddress - usDiscreteInputStart ) / 8 ;
		uint16_t iRegBitIndex = ( uint16_t )( usAddress - usDiscreteInputStart ) % 8 ;

	    //Determine the master or slave
	    if (xMBMasterGetCBRunInMasterMode())
	    {
			/* Update current coil values with new values from the
			 * protocol stack. */
			while(iNReg > 1)
			{
				xMBUtilSetBits(&pucDiscreteInputBuf[iRegIndex++] , iRegBitIndex  , 8 , *pucRegBuffer++);
				iNReg--;
			}
			usNDiscrete = usNDiscrete % 8;
			if (usNDiscrete != 0)
			{
				xMBUtilSetBits(&pucDiscreteInputBuf[iRegIndex++], iRegBitIndex,
						usNDiscrete, *pucRegBuffer++);
			}
	    }
	    else
	    {
			while( iNReg > 0 )
			{
				*pucRegBuffer++ = xMBUtilGetBits(&pucDiscreteInputBuf[iRegIndex++] , iRegBitIndex , 8);
				iNReg --;
			}
            
			pucRegBuffer --;
			usNDiscrete = usNDiscrete % 8;
			*pucRegBuffer = *pucRegBuffer <<(8 - usNDiscrete);
			*pucRegBuffer = *pucRegBuffer >>(8 - usNDiscrete);
	    }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    
    return eStatus;
}

eMBErrorCode
eMBReportSlaveIdCB( UCHAR * pucRegBuffer, UCHAR ucNBytes ){
    eMBErrorCode    eStatus = MB_ENOERR;
    UCHAR *         pusRegRepSlaveIdBuf;
    
    if ( xMBMasterGetCBRunInMasterMode() )
    {
        prv->Status.ConnectionStateCounter = 0;                
        prv->Status.IsAlive = true;
        
        prv->Buffers.SlaveId->ID = *pucRegBuffer++;
        prv->Buffers.SlaveId->Status = *pucRegBuffer++;
        
        pusRegRepSlaveIdBuf = prv->Buffers.SlaveId->Add;
        
        ucNBytes -= 2;

        while( ucNBytes > 0 )
        {
            *(pusRegRepSlaveIdBuf++) = *(pucRegBuffer++);
            ucNBytes--;
        }
        
        CheckPrvType(prv);
    }

    return eStatus;
}



static void HRegsProcOneReregister(UCHAR * pucRegBuffer, USHORT usAddress){
    
    uint8_t i = 0; 
    
    /* jieskom masyve registra su usAddress. */
    while(i < pHRegs->Tail){
        if((pHRegs->Register + i)->Addr == usAddress){               
            (pHRegs->Register + i)->Value = (*pucRegBuffer << 8) | (*(pucRegBuffer + 1));
            break;
        }        
        i++;
    };
    
    /* Neradom tokio registro. Irasom i masyva nauja elementa */ 
    if(i == pHRegs->Tail){
        (pHRegs->Register + i)->Addr = usAddress;
        (pHRegs->Register + i)->Value = (*pucRegBuffer << 8) | (*(pucRegBuffer + 1));
        if(pHRegs->Tail < M_HREGS_COUNT) pHRegs->Tail++; 
    }    
}
static void CoilsProcOneReregister(UCHAR * pucRegBuffer, USHORT usAddress){
}
static void IRegsProcOneReregister(UCHAR * pucRegBuffer, USHORT usAddress){
            
    uint8_t i = 0; 
    
    /* jieskom masyve registra su usAddress. */
    while(i < pIRegs->Tail){
        if((pIRegs->Register + i)->Addr == usAddress){               
            (pIRegs->Register + i)->Value = (*pucRegBuffer << 8) | (*(pucRegBuffer + 1));
            break;
        }        
        i++;
    };
    
    /* Neradom tokio registro. Irasom i masyva nauja elementa */ 
    if(i == pIRegs->Tail){
        (pIRegs->Register + i)->Addr = usAddress;
        (pIRegs->Register + i)->Value = (*pucRegBuffer << 8) | (*(pucRegBuffer + 1));
        if(pIRegs->Tail < M_IREGS_COUNT) pIRegs->Tail++; 
    } 
}
static void DInputsProcOneReregister(UCHAR * pucRegBuffer, USHORT usAddress){
}


void CleanSlaveIdBuffer(){
    
    uint8_t i = 0;
    
    do{
        prv->Buffers.SlaveId->data[i++] = 0x00;
    }while( i < M_REP_SLV_ID_NREGS + 2 );
}

