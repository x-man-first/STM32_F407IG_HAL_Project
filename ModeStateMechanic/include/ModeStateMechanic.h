#ifndef MODESTATEMECHANIC_H
#define MODESTATEMECHANIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"


// Add your type definitions here
typedef enum MM_ModeType_{
    MM_INIT_MODE         = 0u,
    MM_STANDBY_MODE      = 1u,
    MM_TORQUECTRL_MODE   = 2u,
    MM_SPEEDCTRL_MODE    = 3u,
    MM_PRPSLEEP_MODE     = 4u,
    MM_SLEEP_MODE        = 5u,
    MM_FAILURE_MODE      = 6u
} MM_ModeType;

typedef struct MM_InputDataType_{
    uint8_t WakeUpSignal;
    uint8_t FaultSignal;

}MM_InputDataType;

// Add your function declarations here
extern void ModeStateMechanic_Init(void);
extern void ModeStateMechanic_Main(void);

#ifdef __cplusplus
}
#endif

#endif // MODESTATEMECHANIC_H