/**
 * @file MotorControl.c
 * @brief   Motor control implementation
 *
 * Location: /ASW/MotorControl/src/MotorControl.c
 */

#include "MotorControl.h"

/* ======= Configuration macros ======= */


/* ======= Private types ======= */


/* ======= Private variables ======= */
bldc_status_type bldc_status = {0};
int16_t pwm_dy = 0;
/* ======= Private function prototypes ======= */
static uint32_t Hallsensor(void);
static void m1_uhvl(void);
static void m1_uhwl(void);
static void m1_vhul(void);
static void m1_vhwl(void);
static void m1_whul(void);
static void m1_whvl(void);

/* ======= Public functions ======= */
void MotorControl_Init(void);
void MotorControl_Start(void);
void MotorControl_Stop(void);
void MotorControl_DeInit(void);
void MotorControl_Control(void);
void TIM1_Handler_IN_IRQ(void);

/**
 * @brief 
 * @param[in] 
 * @return 
 */
void MotorControl_Init(void)
{
    MotorControl_Stop();
    bldc_status.run_flag = STOP;
}

/**
 * @brief 
 */
void MotorControl_Start(void)
{
    SHUTDOWN_EN;
    HAL_GPIO_WritePin(M1_LOW_SIDE_U_PORT,M1_LOW_SIDE_U_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_V_PORT,M1_LOW_SIDE_V_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_W_PORT,M1_LOW_SIDE_W_PIN,GPIO_PIN_RESET);
    PWM_Start(&htim1);
}

/**
 * @brief 
 */
void MotorControl_Stop(void)
{
    SHUTDOWN_OFF;
    HAL_GPIO_WritePin(M1_LOW_SIDE_U_PORT,M1_LOW_SIDE_U_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_V_PORT,M1_LOW_SIDE_V_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_W_PORT,M1_LOW_SIDE_W_PIN,GPIO_PIN_RESET);
    PWM_Stop(&htim1);
}

void MotorControl_Control(void)
{
    uint8_t key = 0;
    
    key = key_scan(0);
    if(key == KEY0_PRES)                                /* 按下KEY0加速 */
    {
            pwm_dy += 500;
            if(pwm_dy >= MC_DEFAULT_PWM_MAX)
            {
                pwm_dy =  MC_DEFAULT_PWM_MAX;
            }
            if(pwm_dy > 0)
            {
                bldc_status.dir = CW;
                bldc_status.pwm_duty = pwm_dy;
            } 
            else 
            {
                bldc_status.dir = CCW;
                bldc_status.pwm_duty = -pwm_dy;
            }
            MotorControl_Start();
            bldc_status.run_flag = RUN;
    }
    else if(key == KEY1_PRES)                          /* 按下KEY1减速 */
    {
        pwm_dy -= 500;
        if(pwm_dy <= -MC_DEFAULT_PWM_MAX)
        {
            pwm_dy =  -MC_DEFAULT_PWM_MAX;
        }
        if(pwm_dy < 0)
        {
            bldc_status.dir = CCW;
            bldc_status.pwm_duty = -pwm_dy;
        } 
        else 
        {
            bldc_status.dir = CW;
            bldc_status.pwm_duty = pwm_dy;
        }
        MotorControl_Start();
        bldc_status.run_flag = RUN;
    }
    else if(key == KEY2_PRES)                          /* 按下KEY2关闭电机 */
    {
        MotorControl_Stop();
        bldc_status.run_flag = STOP;
        bldc_status.pwm_duty = 0;
        pwm_dy = 0;
    }
}

/**
 * @brief 
 */
void MotorControl_DeInit(void)
{

}

/*
 * @brief       获取霍尔传感器引脚状态
 * @param       无
 * @retval      霍尔传感器引脚状态
 */
static uint32_t Hallsensor(void)
{
    uint32_t state = 0;
    if(HAL_GPIO_ReadPin(HALL1_TIM_CH1_GPIO,HALL1_TIM_CH1_PIN) != GPIO_PIN_RESET)
    {
        state |= 0x01;
    }
    if(HAL_GPIO_ReadPin(HALL1_TIM_CH2_GPIO,HALL1_TIM_CH2_PIN) != GPIO_PIN_RESET)
    {
        state |= 0x02;
    }
    if(HAL_GPIO_ReadPin(HALL1_TIM_CH3_GPIO,HALL1_TIM_CH3_PIN) != GPIO_PIN_RESET)
    {
        state |= 0x04;
    }
    return   state; 
}


/* 上下桥臂的导通情况，共6种，也称为6步换向 */
static void m1_uhvl(void)
{
    /* 上桥臂 */
    PWM_SetDutyCycle(TIM_CHANNEL_1, bldc_status.pwm_duty);
    PWM_SetDutyCycle(TIM_CHANNEL_2, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_3, 0);
    /* 下桥臂 */
    HAL_GPIO_WritePin(M1_LOW_SIDE_U_PORT,M1_LOW_SIDE_U_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_V_PORT,M1_LOW_SIDE_V_PIN,GPIO_PIN_SET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_W_PORT,M1_LOW_SIDE_W_PIN,GPIO_PIN_RESET);
}
static void m1_uhwl(void)
{
    /* 上桥臂 */
    PWM_SetDutyCycle(TIM_CHANNEL_1, bldc_status.pwm_duty);
    PWM_SetDutyCycle(TIM_CHANNEL_2, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_3, 0);
    /* 下桥臂 */
    HAL_GPIO_WritePin(M1_LOW_SIDE_U_PORT,M1_LOW_SIDE_U_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_V_PORT,M1_LOW_SIDE_V_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_W_PORT,M1_LOW_SIDE_W_PIN,GPIO_PIN_SET);
}
static void m1_vhul(void)
{
    /* 上桥臂 */
    PWM_SetDutyCycle(TIM_CHANNEL_1, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_2, bldc_status.pwm_duty);
    PWM_SetDutyCycle(TIM_CHANNEL_3, 0);
    /* 下桥臂 */
    HAL_GPIO_WritePin(M1_LOW_SIDE_U_PORT,M1_LOW_SIDE_U_PIN,GPIO_PIN_SET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_V_PORT,M1_LOW_SIDE_V_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_W_PORT,M1_LOW_SIDE_W_PIN,GPIO_PIN_RESET);
}
static void m1_vhwl(void)
{
    /* 上桥臂 */
    PWM_SetDutyCycle(TIM_CHANNEL_1, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_2, bldc_status.pwm_duty);
    PWM_SetDutyCycle(TIM_CHANNEL_3, 0);
    /* 下桥臂 */
    HAL_GPIO_WritePin(M1_LOW_SIDE_U_PORT,M1_LOW_SIDE_U_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_V_PORT,M1_LOW_SIDE_V_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_W_PORT,M1_LOW_SIDE_W_PIN,GPIO_PIN_SET);
}
static void m1_whul(void)
{
    /* 上桥臂 */
    PWM_SetDutyCycle(TIM_CHANNEL_1, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_2, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_3, bldc_status.pwm_duty);
    /* 下桥臂 */
    HAL_GPIO_WritePin(M1_LOW_SIDE_U_PORT,M1_LOW_SIDE_U_PIN,GPIO_PIN_SET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_V_PORT,M1_LOW_SIDE_V_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_W_PORT,M1_LOW_SIDE_W_PIN,GPIO_PIN_RESET);
}
static void m1_whvl(void)
{
    /* 上桥臂 */
    PWM_SetDutyCycle(TIM_CHANNEL_1, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_2, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_3, bldc_status.pwm_duty);
    /* 下桥臂 */
    HAL_GPIO_WritePin(M1_LOW_SIDE_U_PORT,M1_LOW_SIDE_U_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_V_PORT,M1_LOW_SIDE_V_PIN,GPIO_PIN_SET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_W_PORT,M1_LOW_SIDE_W_PIN,GPIO_PIN_RESET);
}

/**
 * @brief       定时器中断回调
 * @param       无
 * @retval      无
 */
void TIM1_Handler_IN_IRQ(void)
{
        if(bldc_status.run_flag == RUN)
        {
            bldc_status.step_sta = Hallsensor();
            if(bldc_status.step_sta >= 1 && bldc_status.step_sta <= 6)
            {
                if(bldc_status.dir == CW)
                {
                    switch(bldc_status.step_sta)
                    {
                        case 0x01:  m1_uhwl(); break;
                        case 0x02:  m1_vhul(); break;
                        case 0x03:  m1_vhwl(); break;
                        case 0x04:  m1_whvl(); break;
                        case 0x05:  m1_uhvl(); break;
                        case 0x06:  m1_whul(); break;
                        default : break;
                    }
                }
                else if(bldc_status.dir == CCW)
                {
                    switch(bldc_status.step_sta)
                    {
                        case 0x01:  m1_whul(); break;
                        case 0x02:  m1_uhvl(); break;
                        case 0x03:  m1_whvl(); break;
                        case 0x04:  m1_vhwl(); break;
                        case 0x05:  m1_vhul(); break;
                        case 0x06:  m1_uhwl(); break;
                        default : break;
                    }
                }
            }
        }else 
        {
            MotorControl_Stop();
            bldc_status.run_flag = STOP;
        }
    
}

