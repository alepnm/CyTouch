#ifndef	USER_APP
#define USER_APP
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb_m.h"
#include "mbutils.h"

/* -----------------------Slave Defines -------------------------------------*/
#define S_DISCRETE_INPUT_START        1
#define S_DISCRETE_INPUT_NDISCRETES   16
#define S_COIL_START                  1
#define S_COIL_NCOILS                 64
#define S_REG_INPUT_START             1
#define S_REG_INPUT_NREGS             32
#define S_REG_HOLDING_START           1
#define S_REG_HOLDING_NREGS           100

/* -----------------------Master Defines -------------------------------------*/
#define M_DISCRETE_INPUT_START        0
#define M_DISCRETE_INPUT_NDISCRETES   64
#define M_COIL_START                  0
#define M_COIL_NCOILS                 64
#define M_REG_INPUT_START             0
#define M_REG_INPUT_NREGS             16
#define M_REG_HOLDING_START           0
#define M_REG_HOLDING_NREGS           8
#define M_REP_SLV_ID_NREGS            64

    
#pragma pack(push,1) 
union SlaveID{
    uint8_t data[M_REP_SLV_ID_NREGS + 2];
    struct{
        uint8_t     ID;
        uint8_t     Status;
        UCHAR       Add[M_REP_SLV_ID_NREGS];
    };
};
#pragma pack(pop)



#define M_COILS_COUNT   16
#define M_HREGS_COUNT   5
#define M_IREGS_COUNT   10
#define M_DINPUTS_COUNT   16


typedef struct{
    uint16_t        Addr;
    uint16_t        Value;    
}datunit_t;


struct _coils{
    datunit_t   Register[M_COILS_COUNT];
    uint8_t     Tail;
};
struct _hregs{
    datunit_t   Register[M_HREGS_COUNT];
    uint8_t     Tail;
};
struct _iregs{
    datunit_t   Register[M_IREGS_COUNT];
    uint8_t     Tail;
};
struct _dinputs{
    datunit_t   Register[M_DINPUTS_COUNT];
    uint8_t     Tail;
};




void CleanSlaveIdBuffer( void );
    

#endif
