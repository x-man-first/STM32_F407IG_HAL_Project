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
Bldc_Control_Req_type Bldc_Control_Req = {0};
int16_t pwm_dy = 0;
/* ======= Private function prototypes ======= */
static void MotorControl_Start(void);
static void MotorControl_Stop(void);
static void m1_uhvl(void);
static void m1_uhwl(void);
static void m1_vhul(void);
static void m1_vhwl(void);
static void m1_whul(void);
static void m1_whvl(void);

/* ======= Public functions ======= */
uint8_t MotorControl_Init(void);
uint8_t MotorControl_DeInit(void);
void MotorControl_Control(void);
void Motor_Handler_IN_TIM1IRQ(void);

/**
 * @brief 
 * @param[in] 
 * @return 
 */
uint8_t MotorControl_Init(void)
{
    uint8_t MotorControl_Init_Status = 0;

    SHUTDOWN_OFF;
    HAL_GPIO_WritePin(M1_LOW_SIDE_U_PORT,M1_LOW_SIDE_U_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_V_PORT,M1_LOW_SIDE_V_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_W_PORT,M1_LOW_SIDE_W_PIN,GPIO_PIN_RESET);
    PWM_Stop(&htim1);

    MotorControl_Init_Status = 1;

    return MotorControl_Init_Status;
}

/**
 * @brief 
 */
static void MotorControl_Start(void)
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
static void MotorControl_Stop(void)
{
    SHUTDOWN_OFF;
    HAL_GPIO_WritePin(M1_LOW_SIDE_U_PORT,M1_LOW_SIDE_U_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_V_PORT,M1_LOW_SIDE_V_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_W_PORT,M1_LOW_SIDE_W_PIN,GPIO_PIN_RESET);
    PWM_Stop(&htim1);
}

void MotorControl_Control(void)
{
#if USE_KEY_CONTROL
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
#endif
#if USE_CAN_CONTROL
    uint16_t PWM_DutyCycle = 0;
    uint8_t  PWM_Changed = 0;
    uint8_t  dir = 0;
    uint8_t  dir_changed = 0;
    if(Bldc_Control_Req.start_stop == 0x01)                  /* 启动电机 */
    {
        PWM_DutyCycle = Bldc_Control_Req.pwm_duty;
        if(Bldc_Control_Req.CW_CCW == 0x00)          /* 正转 */
        {
            dir = CW;
        }
        if(Bldc_Control_Req.CW_CCW == 0x01)          /* 反转 */
        {
            dir = CCW;
        }

        if(PWM_DutyCycle != bldc_status.pwm_duty)
        {
            PWM_Changed = 1;
        }
        if(dir != bldc_status.dir)
        {
            dir_changed = 1;
        }

        if(PWM_Changed || dir_changed)
        {
            if(PWM_DutyCycle >= MC_DEFAULT_PWM_MAX)
            {
                PWM_DutyCycle =  MC_DEFAULT_PWM_MAX;
            }

            if(dir == CW)          /* 正转 */
            {
                bldc_status.dir = CW;
                bldc_status.pwm_duty = PWM_DutyCycle;
            }
            else if(dir == CCW)          /* 反转 */
            {
                bldc_status.dir = CCW;
                bldc_status.pwm_duty = PWM_DutyCycle;
            }

            MotorControl_Start();
            bldc_status.run_flag = RUN;
        }
    }
    else if(Bldc_Control_Req.start_stop == 0x00)             /* 停止电机 */
    {
        MotorControl_Stop();
        bldc_status.run_flag = STOP;
        bldc_status.pwm_duty = 0;
    }
#endif
}

/**
 * @brief 
 */
uint8_t MotorControl_DeInit(void)
{
    uint8_t MotorControl_DeInit_Status = 0;

    SHUTDOWN_OFF;
    HAL_GPIO_WritePin(M1_LOW_SIDE_U_PORT,M1_LOW_SIDE_U_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_V_PORT,M1_LOW_SIDE_V_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_LOW_SIDE_W_PORT,M1_LOW_SIDE_W_PIN,GPIO_PIN_RESET);
    PWM_Stop(&htim1);

    MotorControl_DeInit_Status = 1;

    return MotorControl_DeInit_Status;

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
 * @brief       TIM1中断回调
 * @param       无
 * @retval      无
 */
void Motor_Handler_IN_TIM1IRQ(void)
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
                bldc_status.step_last = bldc_status.step_sta;
                bldc_status.step_sta = Hallsensor();
            }
        }else 
        {
            MotorControl_Stop();
            bldc_status.run_flag = STOP;
        }
    
}

