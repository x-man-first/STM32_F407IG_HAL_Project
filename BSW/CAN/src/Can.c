#include "Can.h"


CAN_HandleTypeDef hcan1;

static CAN_Status_Type CAN_Status = CAN_Ok;
//接收到的数据储存到下面的变量中
static ReceiveData_Type CANRX_Data = {0};
static CAN_TX_Status_Type CAN_TX_State = CAN_TX_Ok;
static CAN_RX_Status_Type CAN_RX_State = CAN_RX_Ok;


void CAN1_Init(void);
void CAN1TX(void);
CAN_Status_Type Get_CAN_Status(void); 

static void CAN_Filter_config(void);
static CAN_TX_Status_Type can_send_msg(uint32_t id, uint8_t *msg, uint8_t len);
static CAN_RX_Status_Type can_receive_msg(ReceiveData_Type *rx_data);
static void CAN_RX_Handle(void);

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
void CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.State = HAL_CAN_STATE_RESET;
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_7TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_6TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN CAN1_Init 2 */
  CAN_Filter_config();
  /* USER CODE END CAN1_Init 2 */

}

void CAN1TX(void)
{
    uint8_t tx_data[8] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};
    CAN_TX_State = can_send_msg(CANID_SYNC, tx_data, 8);
}


static void CAN_Filter_config(void)
{
    CAN_FilterTypeDef sFilterConfig;
 
    /* 配置CAN过滤器 */
    sFilterConfig.FilterBank = 0;                             /* 过滤器0 */
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;                      /* 32位ID */
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;                  /* 32位MASK */
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;    /* 过滤器0关联到FIFO0 */
    sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;       /* 激活滤波器0 */
    sFilterConfig.SlaveStartFilterBank = 14;
 
    /* 过滤器配置 */
    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }
	HAL_CAN_Start(&hcan1);//开启CAN
	HAL_CAN_ActivateNotification(&hcan1 ,CAN_IT_RX_FIFO0_MSG_PENDING);//开启CAN的中断
}

/**
 * @brief       CAN 发送一组数据
 * @note        发送格式固定为: 标准ID, 数据帧
 * @param       id      : 标准ID(11位)
 * @param       msg     : 数据指针
 * @param       len     : 数据长度
 * @retval      发送状态 0, 成功; 1, 失败;
 */
static CAN_TX_Status_Type can_send_msg(uint32_t id, uint8_t *msg, uint8_t len)
{
    uint16_t t = 0;
    uint32_t TxMailbox = CAN_TX_MAILBOX0;
    CAN_TxHeaderTypeDef can_tx_handle;
    CAN_TX_Status_Type CAN_TX_Status = CAN_TX_Ok;

    can_tx_handle.StdId = id;         /* 标准标识符 */
    can_tx_handle.ExtId = id;         /* 扩展标识符(29位) */
    can_tx_handle.IDE = CAN_ID_STD;   /* 使用标准帧 */
    can_tx_handle.RTR = CAN_RTR_DATA; /* 数据帧 */
    can_tx_handle.DLC = len;
 
    if (HAL_CAN_AddTxMessage(&hcan1 , &can_tx_handle, msg, &TxMailbox) != HAL_OK) /* 发送消息 */
    {
        CAN_TX_Status =  CAN_TX_Notok;
    }
    
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 3)   /* 等待发送完成,所有邮箱为空 */
    {
        t++;
        
        if (t > 0xFFF)
        {
            HAL_CAN_AbortTxRequest(&hcan1, TxMailbox);     /* 超时，直接中止邮箱的发送请求 */
            CAN_TX_Status =  CAN_TX_Notok;
        }
    }
    
    return CAN_TX_Status;
}

/**
 * @brief       CAN 接收数据查询
 * @note        接收数据格式固定为: 标准ID, 数据帧
 * @param       id      : 要查询的 标准ID(11位)
 * @param       buf     : 数据缓存区
 * @retval      接收结果
 * @arg         0   , 无数据被接收到;
 * @arg         其他, 接收的数据长度
 */
static CAN_RX_Status_Type can_receive_msg(ReceiveData_Type *rx_data)
{
    CAN_RxHeaderTypeDef can_rx_handle;
    CAN_RX_Status_Type CAN_RX_Status = CAN_RX_Ok;

    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 1)
    {
        rx_data->Fifo_Source = CAN_RXFifo0;
    }
    else if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO1) == 1)
    {
        rx_data->Fifo_Source = CAN_RXFifo1;
    }
    else
    {
        CAN_RX_Status =  CAN_RX_Notok;
    }
 
    if (HAL_CAN_GetRxMessage(&hcan1, rx_data->Fifo_Source, &can_rx_handle, (uint8_t *)rx_data->Data) != HAL_OK)  /* 读取数据 */
    {
        CAN_RX_Status = CAN_RX_Notok;
    }
    rx_data->ID = can_rx_handle.StdId;
 
    return CAN_RX_Status;

}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  if (hcan == (&hcan1))
  {
    CAN_RX_State = can_receive_msg(&CANRX_Data);
  }
  CAN_RX_Handle();
}

CAN_Status_Type Get_CAN_Status(void)
{
    if(CAN_TX_State == CAN_TX_Notok || CAN_RX_State == CAN_RX_Notok)
    {
        CAN_Status = CAN_Notok;
    }
    else
    {
        CAN_Status = CAN_Ok;
    }
    
    return CAN_Status;
}

static void CAN_RX_Handle(void)
{
    if(CANRX_Data.ID == CANID_VCU_CMD)
    {
        Bldc_Control_Req.start_stop = (CANRX_Data.Data[0]&0x00000001);
        Bldc_Control_Req.CW_CCW = ((CANRX_Data.Data[0]&0x00000002)>>1);
        Bldc_Control_Req.pwm_duty = (uint16_t)((CANRX_Data.Data[0]&0x00FFFF00)>>8);
    }
}