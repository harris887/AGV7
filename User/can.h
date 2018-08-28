#ifndef __CAN_H_
#define __CAN_H_
#include "stm32f10x_lib.h"


#define CAN_BOUND_1000          1000
#define CAN_BOUND_500           500
#define CAN_BOUND_250           250

#define MAX_CAN_RX_INFOR_NUM    16
#define CAN_RX_INFOR_NUM_MASK   (MAX_CAN_RX_INFOR_NUM - 1)
extern CanRxMsg g_CAN_RxData[MAX_CAN_RX_INFOR_NUM];
extern u8 CAN_INFOR_IN_Index;
extern u8 CAN_INFOR_OUT_Index;
extern u32 CAN_RX_IntNum;
extern u8 LaserSelect;
extern u16 LASER_SENSOR_Flag;

typedef struct
{
  u32 heart_id;
  u32 data_id;
  u32 cmd_id;
  u16 distance;
  s8  angle;
  u8 is_things;
  u32 heart_beat_num;
  u32 heart_beat_num_bk;
  u32 data_infor_num;
  u32 data_infor_num_bk;
  u32 state;
}LASER_INFOR;
#define LASER_NUM   2
extern LASER_INFOR LASER_Infor[LASER_NUM];
extern u32 LaserTimeout;
extern u16 LaserBeepTimeout;
/*************************************************
Function: CAN1_Init
Description: CAN初始化
Input: Param1 - bound
       Param2 - mode:CAN_Mode_Normal,普通模式;CAN_Mode_LoopBack,回环模式;
Output: none
Return: none
Others: 1m bound
*************************************************/
void CAN1_Init(u32 Param1, u32 Param2);

/*************************************************
Function:       CAN1_Send
Description:    CAN发送数据
Input:          data - 需要发送数据
                len - 数据长度
                id - CAN-ID
Output: none
Return: none
Others: none
*************************************************/
void CAN1_Send(u8 *data,u16 len,u32 id);

void LASER_INFOR_Init(void);
void Laser_Task();
void Set_LASER(u32 id, u8 on_off, u8 width_cm, u16 deep_cm);
#endif

