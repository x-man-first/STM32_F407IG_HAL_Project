#ifndef CAN_H
#define CAN_H

#include "stm32f4xx_hal.h"
#include "main.h"
#include "Can_Cfg.h"
#include "MotorControl.h"

#define CAN_RXFifo0  0
#define CAN_RXFifo1  1

typedef enum
{
    CAN_RX_Notok = 0,
    CAN_RX_Ok = 1
} CAN_RX_Status_Type;

typedef enum
{
    CAN_TX_Notok = 0,
    CAN_TX_Ok = 1
} CAN_TX_Status_Type;

typedef enum
{
    CAN_Notok = 0,
    CAN_Ok = 1
} CAN_Status_Type;

typedef struct
{
    uint8_t  ID;
    uint32_t Data[2];
    uint8_t  Fifo_Source;

}ReceiveData_Type;

extern CAN_HandleTypeDef hcan1;

extern void CAN1_Init(void);
extern void CAN1TX(void);
extern CAN_Status_Type Get_CAN_Status(void);

#endif // CAN_H