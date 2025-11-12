/*
 * ModeStateMechanic.c
 * Description: Implementation of mode state mechanic functions.
 */
 
// Add your includes here
#include "ModeStateMechanic.h"
#include "ModeStateMechanic_InitTask.h"

MM_ModeType ModeMechanic_CurrentMode;
MM_InputDataType ModeMechanic_InputData;

void ModeStateMechanic_Init(void);
void ModeStateMechanic_Main(void);

static void ModeStateMechanic_Input(void);
static void ModeStateMechanic_Output(void);
static void ModeStateMechanic_ModeControl(void);
static void ModeStateMechanic_ModeTransition(MM_ModeType newMode);
static void ModeStateMechanic_InitMode(void);
static void ModeStateMechanic_StandbyMode(void);
static void ModeStateMechanic_TorqueCtrlMode(void);
static void ModeStateMechanic_SpeedCtrlMode(void);
static void ModeStateMechanic_PrpSleepMode(void);
static void ModeStateMechanic_SleepMode(void);
static void ModeStateMechanic_FailureMode(void);
static uint8_t ModeStateMechanic_GetFaultSignal(void);

// Add your function implementations below

void ModeStateMechanic_Init(void)
{
// Initialization code here
    ModeMechanic_CurrentMode = MM_INIT_MODE;
    MM_InitReq = MM_INIT_REQ_INITIALIZE;

    for(uint8_t i = 0; i < MM_INIT_NUM; i++)
    {
        MM_InitSts[i] = MM_INIT_STS_DEINITIALIZED;
    }

}

void ModeStateMechanic_Main(void)
{
    // Update state machine logic here
    ModeStateMechanic_Input();

    ModeStateMechanic_ModeControl();

    ModeStateMechanic_Output();

}

static void ModeStateMechanic_Input(void)
{
    ModeMechanic_InputData.WakeUpSignal = 1;
    ModeMechanic_InputData.FaultSignal = ModeStateMechanic_GetFaultSignal();
}

static void ModeStateMechanic_Output(void)
{

}

static void ModeStateMechanic_ModeControl(void)
{

    switch(ModeMechanic_CurrentMode)
    {
        case MM_INIT_MODE:
            ModeStateMechanic_InitMode();
            break;
        case MM_STANDBY_MODE:
            ModeStateMechanic_StandbyMode();
            break;
        case MM_TORQUECTRL_MODE:
            ModeStateMechanic_TorqueCtrlMode();
            break;
        case MM_SPEEDCTRL_MODE:
            ModeStateMechanic_SpeedCtrlMode();
            break;
        case MM_PRPSLEEP_MODE:
            ModeStateMechanic_PrpSleepMode();
            break;
        case MM_SLEEP_MODE:
            ModeStateMechanic_SleepMode();
            break;
        case MM_FAILURE_MODE:
            ModeStateMechanic_FailureMode();
            break;
        default:
            // Handle unexpected mode
            break;
    }
}

static void ModeStateMechanic_ModeTransition(MM_ModeType newMode)
{
    // Handle mode transition logic here
    ModeMechanic_CurrentMode = newMode;

}

static void ModeStateMechanic_InitMode(void)
{
    MM_InitDeinit();

    if(MM_InitSts[MM_INIT_TEST] == MM_INIT_STS_INITIALIZED)
    {
        ModeStateMechanic_ModeTransition(MM_STANDBY_MODE);
    }
    else if(MM_InitSts[MM_INIT_TEST] == MM_INIT_STS_INIT_FAILED)
    {
        ModeStateMechanic_ModeTransition(MM_FAILURE_MODE);
    }
    else
    {
        // Stay in INIT_MODE until initialization is complete
    }
}

static void ModeStateMechanic_StandbyMode(void)
{
    ModeStateMechanic_ModeTransition(MM_TORQUECTRL_MODE);
    ModeStateMechanic_ModeTransition(MM_SPEEDCTRL_MODE);
    ModeStateMechanic_ModeTransition(MM_PRPSLEEP_MODE);
    ModeStateMechanic_ModeTransition(MM_FAILURE_MODE);
}

static void ModeStateMechanic_TorqueCtrlMode(void)
{
    ModeStateMechanic_ModeTransition(MM_STANDBY_MODE);
    ModeStateMechanic_ModeTransition(MM_FAILURE_MODE);
    ModeStateMechanic_ModeTransition(MM_PRPSLEEP_MODE);
}

static void ModeStateMechanic_SpeedCtrlMode(void)
{
    ModeStateMechanic_ModeTransition(MM_STANDBY_MODE);
    ModeStateMechanic_ModeTransition(MM_FAILURE_MODE);
    ModeStateMechanic_ModeTransition(MM_PRPSLEEP_MODE);
}

static void ModeStateMechanic_PrpSleepMode(void)
{
    MM_InitReq = MM_INIT_REQ_DEINITIALIZE;
    MM_InitDeinit();

    if(MM_InitSts[MM_INIT_TEST] == MM_INIT_STS_DEINITIALIZED)
    {
        ModeStateMechanic_ModeTransition(MM_SLEEP_MODE);
    }
    else if(MM_InitSts[MM_INIT_TEST] == MM_INIT_STS_DEINIT_FAILED)
    {
        ModeStateMechanic_ModeTransition(MM_FAILURE_MODE);
    }
    else if(MM_InitSts[MM_INIT_TEST] == MM_INIT_STS_DEINIT_PENDING)
    {
      if(ModeMechanic_InputData.FaultSignal == 1)
      {
          ModeStateMechanic_ModeTransition(MM_FAILURE_MODE);
      }
      else if(ModeMechanic_InputData.WakeUpSignal == 1)
      {
          MM_InitReq = MM_INIT_REQ_INITIALIZE;
          ModeStateMechanic_ModeTransition(MM_INIT_MODE);
      }
      else
      {
          // Stay in PRPSLEEP_MODE until deinitialization is complete
      }
    }
    else
    {
        // Stay in PRPSLEEP_MODE until deinitialization is complete
    }

}

static void ModeStateMechanic_SleepMode(void)
{
    //go to sleep

}

static void ModeStateMechanic_FailureMode(void)
{
    ModeStateMechanic_ModeTransition(MM_STANDBY_MODE);
    ModeStateMechanic_ModeTransition(MM_PRPSLEEP_MODE);
}

static uint8_t ModeStateMechanic_GetFaultSignal(void)
{
    return 0u; // Simulate no fault
}