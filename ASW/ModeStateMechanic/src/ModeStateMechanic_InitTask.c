/**
 * @file ModeStateMechanic_InitTask.c
 * @brief ModeStateMechanic_InitTask module source file
 */
#include "ModeStateMechanic_InitTask.h"

MM_InitStsType MM_InitSts[MM_INIT_NUM];
MM_InitReqType MM_InitReq;

void MM_InitStart_Test(void);
void MM_InitPending_Test(void);
void MM_DeinitStart_Test(void);
void MM_DeInitPending_Test(void);

void MM_InitDeinit(void);

static uint8_t MM_Test_Init(void);
static uint8_t MM_Test_DeInit(void);

static const InitCfgType MM_InitCfg[MM_INIT_NUM] =
{
   { &MM_InitStart_Test,          &MM_InitPending_Test,    &MM_DeinitStart_Test,  &MM_DeInitPending_Test }
};
static uint16_t MM_Test_Init_TimeoutCtr = 0u;
static uint16_t MM_Test_DeInit_TimeoutCtr = 0u;

/* Public function implementations */
void MM_InitStart_Test(void)
{
   // Simulate initialization process
   MM_Test_Init_TimeoutCtr = MM_Test_INIT_TIMEROUTCOUNT;
   MM_InitSts[MM_INIT_TEST] = MM_INIT_STS_INIT_PENDING;
}
void MM_InitPending_Test(void)
{
   uint8_t initStatus = 0u;

   initStatus = MM_Test_Init();

    // Simulate checking initialization status
   if(1 == initStatus)
   {
      // Simulate successful initialization
      MM_InitSts[MM_INIT_TEST] = MM_INIT_STS_INITIALIZED;
   } 
   else if(0u == MM_Test_Init_TimeoutCtr)
   {
      MM_InitSts[MM_INIT_TEST] = MM_INIT_STS_INIT_FAILED;
   }
   else
   {
      MM_Test_Init_TimeoutCtr = MM_Test_Init_TimeoutCtr - 1u;
   }
   
}

void MM_DeinitStart_Test(void)
{
   // Simulate deinitialization process
   MM_Test_DeInit_TimeoutCtr = MM_Test_DEINIT_TIMEROUTCOUNT;
   MM_InitSts[MM_INIT_TEST] = MM_INIT_STS_DEINIT_PENDING;

}

void MM_DeInitPending_Test(void)
{
   uint8_t deinitStatus = 0u;
   deinitStatus = MM_Test_DeInit();
    // Simulate checking deinitialization status
   if(1 == deinitStatus)
   {
      // Simulate successful deinitialization
      MM_InitSts[MM_INIT_TEST] = MM_INIT_STS_DEINITIALIZED;
   } 
   else if(0u == MM_Test_DeInit_TimeoutCtr)
   {
      MM_InitSts[MM_INIT_TEST] = MM_INIT_STS_DEINIT_FAILED;
   }
   else
   {
      MM_Test_DeInit_TimeoutCtr = MM_Test_DeInit_TimeoutCtr - 1u;
   }
}

static uint8_t MM_Test_Init(void)
{
    return 1u; // Simulate successful initialization
}

static uint8_t MM_Test_DeInit(void)
{
    return 1u; // Simulate successful deinitialization
}

void MM_InitDeinit(void)
{
   uint32_t i = 0u;

   for(i = 0u; i < (uint32_t)MM_INIT_NUM; i++)
   {
      switch(MM_InitSts[i])
      {
         case MM_INIT_STS_DEINITIALIZED:
            {
              if(MM_InitReq == MM_INIT_REQ_INITIALIZE)
              {
                /* Start initializing the function */
                (*MM_InitCfg[i].InitStart)();
              }
              else
              {
               /* Nothing to do */
              }
              break;
            }
         case MM_INIT_STS_INIT_PENDING:
            {
               /* Wait until initialization is finished/failed */
              (*MM_InitCfg[i].InitPending)();
              break;
            }
         case MM_INIT_STS_INITIALIZED:
         case MM_INIT_STS_INIT_FAILED:
            {
              if(MM_InitReq == MM_INIT_REQ_DEINITIALIZE)
              {
                /* Start deinitializing the function */
                (*MM_InitCfg[i].DeinitStart)();
              }
              else
              {
               /* Nothing to do */
              }
              break;
            } 
         case MM_INIT_STS_DEINIT_PENDING:
            {
            /* Wait until deinitialization is finished */
              (*MM_InitCfg[i].DeInitPending)();
              break;
            }
         case MM_INIT_STS_DEINIT_FAILED:
            {
              if(MM_InitReq == MM_INIT_REQ_INITIALIZE)
              {
                /* Start initializing the function */
                (*MM_InitCfg[i].InitStart)();
              }
              else if(MM_InitReq == MM_INIT_REQ_DEINITIALIZE)
              {
                /* Start deinitializing the function */
                (*MM_InitCfg[i].DeinitStart)();
              }
              else
              {
               /* Nothing to do */
              }
              break;
            }
         default:
            /* Unknown status --- should not happen! */
            break;
      }
   }
}
