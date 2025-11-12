#ifndef MODESTATEMECHANIC_INITTASK_H
#define MODESTATEMECHANIC_INITTASK_H

#ifdef __cplusplus
extern "C" {
#endif

// Includes
#include "stm32f4xx_hal.h"

#define MM_Test_INIT_TIMEROUTCOUNT   2u //20ms
#define MM_Test_DEINIT_TIMEROUTCOUNT 2u //20ms

typedef enum MM_InitStsType_
{
   MM_INIT_STS_DEINITIALIZED    = 0u,
   MM_INIT_STS_INITIALIZED      = 1u,
   MM_INIT_STS_INIT_PENDING     = 2u,
   MM_INIT_STS_INIT_FAILED      = 3u,
   MM_INIT_STS_DEINIT_PENDING   = 4u,
   MM_INIT_STS_DEINIT_FAILED    = 5u
} MM_InitStsType;

typedef enum MM_InitStsIndexes_{
    MM_INIT_TEST,
    /* Add new values before the last value */
    MM_INIT_NUM
}MM_InitStsIndexes;

typedef void (*InitFuncType)(void);

typedef struct InitCfgType_
{
   InitFuncType InitStart;
   InitFuncType InitPending;
   InitFuncType DeinitStart;
   InitFuncType DeInitPending;
} InitCfgType;

typedef enum MM_InitReqType_
{
   MM_INIT_REQ_INITIALIZE       = 0u,
   MM_INIT_REQ_DEINITIALIZE     = 1u
} MM_InitReqType;

extern MM_InitStsType MM_InitSts[MM_INIT_NUM];
extern MM_InitReqType MM_InitReq;

// Function prototypes
extern void MM_InitStart_Test(void);
extern void MM_InitPending_Test(void);
extern void MM_DeinitStart_Test(void);
extern void MM_DeInitPending_Test(void);

extern void MM_InitDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // MODESTATEMECHANIC_INITTASK_H