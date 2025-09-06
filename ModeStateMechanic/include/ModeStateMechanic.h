#ifndef MODESTATEMECHANIC_H
#define MODESTATEMECHANIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "GPIO.h"
#include "SystemClock.h"

// Add your type definitions here

// Add your function declarations here
extern void ModeStateMechanic_Init(void);
extern void ModeStateMechanic_Main(void);

#ifdef __cplusplus
}
#endif

#endif // MODESTATEMECHANIC_H