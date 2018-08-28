#include "user_inc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

extern u16 g_can_defbound;

#define DEFAULT_LASER_BEEP_TIMEOUT  1000
// 接收邮箱
CanRxMsg g_CAN_RxData[MAX_CAN_RX_INFOR_NUM];
u8 CAN_INFOR_IN_Index = 0;
u8 CAN_INFOR_OUT_Index = 0;
u32 CAN_RX_IntNum = 0;
u32 LaserTimeout = 0;
u16 LaserBeepTimeout = DEFAULT_LASER_BEEP_TIMEOUT;
u8 LaserSelect = 0; // LASER_NUM
u16 laser_width_cm = 70;
u16 laser_deep_cm = 100; // 200
u16 LASER_SENSOR_Flag = 0;

// CAN初始化函数
void CAN1_Init(u32 Param1, u32 Param2)
{
    GPIO_InitTypeDef       GPIO_InitStructure;
    CAN_InitTypeDef        CAN_InitStructure;
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;
    

    u8 Mode = CAN_Mode_Normal;
    u8 CAN_SJW;
    u8 CAN_BS1;
    u8 CAN_BS2;
    u16 CAN_Prescaler;
    if(Param2 == CAN_Mode_Normal)
    {
        Mode = CAN_Mode_Normal;
    }
    else if(Param2 == CAN_Mode_LoopBack)
    {
        Mode = CAN_Mode_LoopBack;
    }

    if(Param1 == CAN_BOUND_1000)
    {
        CAN_SJW = CAN_SJW_1tq;
        CAN_BS1 = CAN_BS1_9tq;
        CAN_BS2 = CAN_BS2_8tq;
        CAN_Prescaler = 2;
    }
    else if(Param1 == CAN_BOUND_500)
    {
        CAN_SJW = CAN_SJW_1tq;
        CAN_BS1 = CAN_BS1_5tq;
        CAN_BS2 = CAN_BS2_2tq;
        CAN_Prescaler = 9;
    }
    else if(Param1 == CAN_BOUND_250)
    {
        CAN_SJW = CAN_SJW_1tq;
        CAN_BS1 = CAN_BS1_4tq;
        CAN_BS2 = CAN_BS2_3tq;
        CAN_Prescaler = 18;
    }

    GPIO_PinRemapConfig(GPIO_Remap1_CAN, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //复用推挽
    GPIO_Init(GPIOB, &GPIO_InitStructure);            //初始化IO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;    //上拉输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);           //初始化IO

    //CAN单元设置
    CAN_InitStructure.CAN_TTCM = DISABLE;         //非时间触发通信模式
    CAN_InitStructure.CAN_ABOM = DISABLE;         //软件自动离线管理
    CAN_InitStructure.CAN_AWUM = DISABLE;         //睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
    CAN_InitStructure.CAN_NART = ENABLE;          //禁止报文自动传送
    CAN_InitStructure.CAN_RFLM = DISABLE;         //报文不锁定,新的覆盖旧的
    CAN_InitStructure.CAN_TXFP = DISABLE;         //优先级由报文标识符决定
    CAN_InitStructure.CAN_Mode = Mode;            //模式设置： mode:0,普通模式;1,回环模式;
    //设置波特率
    CAN_InitStructure.CAN_SJW = CAN_SJW;      //重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位  CAN_SJW_1tq     CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq
    CAN_InitStructure.CAN_BS1 = CAN_BS1;      //Tbs1=tbs1+1个时间单位CAN_BS1_1tq ~CAN_BS1_16tq
    CAN_InitStructure.CAN_BS2 = CAN_BS2;      //Tbs2=tbs2+1个时间单位CAN_BS2_1tq ~    CAN_BS2_8tq
    CAN_InitStructure.CAN_Prescaler = CAN_Prescaler;          //分频系数(Fdiv)为brp+1
    CAN_Init(&CAN_InitStructure);           //初始化CAN1

    CAN_FilterInitStructure.CAN_FilterNumber = 0;                        //过滤器0
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;      //屏蔽位模式
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;     //32位宽
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;                   //32位ID
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;               //32位MASK
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0; //过滤器0关联到FIFO0
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;               //激活过滤器0

    CAN_FilterInit(&CAN_FilterInitStructure);

    CAN_ITConfig(CAN_IT_FMP0,ENABLE);
}

/*************************************************
Function:       CAN1_SendMsg
Description:    CAN发送单包数据
Input:          data - 需要发送数据
                len - 数据长度
                id - CAN-ID
Output: none
Return: none
Others: none
*************************************************/
static void CAN1_SendMsg(u8 *data,u16 len,u32 id)
{
    // 传输邮箱
    u8 Tran_Mailbox;
    CanTxMsg CAN_TxData;
    u8 i = 0;
    u32 count = 0;

    CAN_TxData.StdId = id;                  // 设置标准标示符
    CAN_TxData.IDE   = CAN_ID_STD;     // 使用标准标识符
    CAN_TxData.RTR   = CAN_RTR_DATA;        // 消息类型为数据帧
    CAN_TxData.DLC   = len;

    for(i = 0; i < len; i++)
    {
        CAN_TxData.Data[i] = data[i];
    }

    Tran_Mailbox = CAN_Transmit(&CAN_TxData);
#if (1)
    while((CAN_TransmitStatus(Tran_Mailbox) != CANTXOK) && (count < 2000))
    {
        count++;
    }
    if(count >= 2000) //255
    {
        //CAN_DeInit(CAN1);
        //CAN1_Init(g_can_defbound, CAN_Mode_Normal);
        //Tran_Mailbox = CAN_Transmit(CAN1, &CAN_TxData);
        printf("error can\r\n");
    }    
#endif
}

/*************************************************
Function:       CAN1_Send
Description:    CAN发送数据
Input:          data - 需要发送数据
                len - 数据长度
                id - CAN-ID
Output: none
Return: none
Others: 可以发送数据长度8字节以上数据
*************************************************/
void CAN1_Send(u8 *data,u16 len,u32 id)
{
    u16 send_once_len;

    if(len <= 8)
    {
        send_once_len = len;
    }
    else
    {
        send_once_len = 8;
    }
    while(1)
    {
        CAN1_SendMsg(data, send_once_len, id);
        if(len == send_once_len)
        {
            break;
        }
        else
        {
            data += send_once_len;
            len -= send_once_len;
            if(len <= 8)
            {
               send_once_len = len;
            }
            else
            {
                send_once_len = 8;
            }
        }
    }
}



/*
// 消息队列全局变量
//extern Msg_Buf g_TaskMessage;

// CAN中断函数
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    //Msg_Data Msg;
    CAN_Receive(CAN_FIFO0, &g_CAN_RxData);
    //Msg.msg_type = MSG_TYPE_CE30_DATA;
    //Msg.msg_data = 0;
    //MSG_Set(&g_TaskMessage, &Msg);
}
*/



LASER_INFOR LASER_Infor[LASER_NUM];

void LASER_INFOR_Init(void)
{
  memset(LASER_Infor, 0, sizeof(LASER_Infor));
  LASER_Infor[0].data_id = 0x101;
  LASER_Infor[0].heart_id = 0x102;
  LASER_Infor[0].cmd_id = 0x103;
  
  LASER_Infor[1].data_id = 0x201;  
  LASER_Infor[1].heart_id = 0x202;
  LASER_Infor[1].cmd_id = 0x203;
}

void Laser_Task()
{
  static u8 pro = 0;
  static u8 LaserSelectTemp;
  u8 i;
  if(CAN_INFOR_IN_Index != CAN_INFOR_OUT_Index)
  {
    CanRxMsg* m = &g_CAN_RxData[CAN_INFOR_OUT_Index];
    for(i = 0; i < LASER_NUM; i++)
    {
      if(LASER_Infor[i].heart_id == m->StdId) 
      {
        LASER_Infor[i].heart_beat_num += 1;
        break;
      }
      if(LASER_Infor[i].data_id == m->StdId) 
      {
        LASER_Infor[i].distance = m->Data[0] | (m->Data[1] << 8);
        LASER_Infor[i].angle = *(s8*)(&m->Data[3]);  
        LASER_Infor[i].is_things = m->Data[7] & 0x1;
        LASER_Infor[i].data_infor_num += 1;
        break;
      }
    }
    CAN_INFOR_OUT_Index += 1;
    CAN_INFOR_OUT_Index &= CAN_RX_INFOR_NUM_MASK;
  }
  
  // live check
  if(LaserTimeout == 0)
  {
    LaserTimeout = 1000;
    for(i = 0; i < LASER_NUM; i++)
    {
      if(LASER_Infor[i].state & 0x1)
      {
        if(LASER_Infor[i].heart_beat_num == LASER_Infor[i].heart_beat_num_bk)
        {
          LASER_Infor[i].state = 0;
          printf("LASER %d OutSystem \n", i);
        }
        else
        {
          LASER_Infor[i].heart_beat_num_bk = LASER_Infor[i].heart_beat_num;
          if(LASER_Infor[i].state & 0x2)
          {
            if(LASER_Infor[i].data_infor_num == LASER_Infor[i].data_infor_num_bk)
            {
              LASER_Infor[i].state &= ~0x2;
              printf("LASER %d Idle \n", i);
            }
            else
            {
              LASER_Infor[i].data_infor_num_bk = LASER_Infor[i].data_infor_num;
            }
          }
          else
          {
            if(LASER_Infor[i].data_infor_num != LASER_Infor[i].data_infor_num_bk)
            {
              LASER_Infor[i].state |= 0x2;
              LASER_Infor[i].data_infor_num_bk = LASER_Infor[i].data_infor_num;
              printf("LASER %d Working \n", i);
            }
          }
        }
      }
      else
      {
        if(LASER_Infor[i].heart_beat_num != LASER_Infor[i].heart_beat_num_bk)
        {
          LASER_Infor[i].state = 1;
          LASER_Infor[i].heart_beat_num_bk = LASER_Infor[i].heart_beat_num;
          LASER_Infor[i].data_infor_num_bk = LASER_Infor[i].data_infor_num;
          printf("LASER %d InSystem \n", i);
        }
      }
    }
    
    // select laser
    switch(pro)
    {
    case 0:
      {
        LaserSelectTemp = LaserSelect;
        if(LaserSelectTemp < LASER_NUM)
        {
          if(((LASER_Infor[LaserSelectTemp].state & 0x2) == 0) && ((LASER_Infor[LaserSelectTemp].state & 0x1) != 0))
          {
            pro += 1;
          }
        }
        else
        {
          for(i = 0; i < LASER_NUM; i++)
          {
            if(LASER_Infor[i].state & 0x2)
            {
              printf("Set_LASER %d OFF \n", i);
              Set_LASER(LASER_Infor[i].cmd_id, 0, 0, 0);
              pro = 7;
            }
          }
        }
      }
      break;
    case 1:
      {
        for(i = 0; i < LASER_NUM; i++)
        {
          printf("Set_LASER %d OFF \n", i);
          Set_LASER(LASER_Infor[i].cmd_id, 0, 0, 0);
        }
        pro += 1;
      }
      break;
    case 2:
      {
        printf("Set_LASER %d ON \n", LaserSelectTemp);
        Set_LASER(LASER_Infor[LaserSelectTemp].cmd_id, 1, laser_width_cm, laser_deep_cm);
        pro += 1;
      }
      break;
    case 3:
    case 4:
    case 5:  
      {
        if((LASER_Infor[LaserSelectTemp].state & 0x2) != 0) pro = 0;
        else pro += 1;
      }
      break;
    case 6:
      {
        pro = 0;
      }
      break;
    case 7:
    case 8:
    case 9:
      {
        pro += 1;
      }
      break;
    case 10:
      {
        pro = 0;
      }
      break;      
    }
  }
  
#if (1)
  if(LaserBeepTimeout == 0)
  {
    LaserBeepTimeout = DEFAULT_LASER_BEEP_TIMEOUT;
    for(i = 0; i < LASER_NUM; i++)
    {
      if((LASER_Infor[i].state & 0x2) && (LASER_Infor[i].is_things))
      {
        //SetBeep(1, 100, 50);
        printf("Laser %d: angle = %d, distance = %d\n", i, LASER_Infor[i].angle, LASER_Infor[i].distance);
        LASER_SENSOR_Flag |= (1 << i); 
      }
      else
      {
        LASER_SENSOR_Flag &= ~(1 << i);
      }
    }
  }
#endif  
}

void Set_LASER(u32 id, u8 on_off, u8 width_cm, u16 deep_cm)
{
  //0xc1 0x00 0x23 0x28 0x00
  u8 data[8];
  memset(data, 0, 8);
  if(on_off & 1)
  {
    data[0] = 0xc1;
    data[2] = width_cm >> 1;
    data[3] = deep_cm / 10;       
    CAN1_Send(data, 5, id);
  }
  else
  {
    data[0] = 0xc0;
    CAN1_Send(data, 5, id);
  }
}