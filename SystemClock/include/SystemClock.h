#ifndef SYSTEMCLOCK_H
#define SYSTEMCLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "main.h"

/**
 * @brief 
 * @note   Configure the clock source and frequency as needed.
 */
void SystemClock_Init(void);

#ifdef __cplusplus
}
#endif

#endif // SYSTEMCLOCK_H