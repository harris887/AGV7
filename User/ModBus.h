#ifndef __ModBus_H_
#define __ModBus_H_
#include "init.h"

#define DEFAULT_MODE_BUS_HALL_ADDR    1
#define BACKWARD_MODE_BUS_HALL_ADDR   2
// 擦除一个page   2K
// 写之前要对page 进行擦除	  例如： 一页大小为2K，如果存储10个数据单位，先擦一遍数据区域，然后分别写入10个数据单位，
//                                   如果不擦除数据，不能保证数据正确。
#define return_OK                 0x00  // 一帧数据 OK 
#define illegal_function         0x01  // 功能码错误
#define illegal_register         0x02  // 寄存器地址错误
#define illegal_data              0x03  // 数据错误
#define crc_err                   0x04  //CRC 校验错误
#define switch_err                0x05  //写数据时开关没开
#define switch_value_err          0x06  //写开关值错误

#define CMD_ModBus_Read         0x04
#define CMD_ModBus_Write        0x06
#define CMD_ModBus_WriteMore    0x16

#define CMD_ModBus_ReadEx       0x03
#define CMD_ModBus_Read         0x04
#define CMD_ModBus_Write        0x06
#define CMD_ModBus_WriteMore    0x16

#define MOTO_CONTROL_MODE_SPEED 0
#define MOTO_CONTROL_MODE_RUN_PULSE 1



extern u16 modebus_timeout;
extern u8 Modebus_read_cmd_tx_finish;
extern u16 Modebus_tx_rx_change_delay;




#define LINE_SENSOR_NUM 16
#define WONDER_MID_SENSOR_INDEX 17//(8.5*2)为了整数预算，乘以2
#define MAX_SENSOR_VALUE        32
#define MIN_SENSOR_VALUE        2

extern u8 HallStatusFresh;
extern u8 HallValue[LINE_SENSOR_NUM];
typedef struct
{
  u8 black_sensor_num;
  u8 black_sensor_serial_flag;
  u8 Middle_Index;
  u8 black_sensor_index_list[LINE_SENSOR_NUM];
}SENSOR_STATUS;

typedef struct
{
  u8 head_index;
  u8 tail_index;
  u16 middle_index;
}SEGMENT_OPTION;
#define MAX_SEGMENT_NUM 8
typedef struct
{
  //u8 black_sensor_num;
  u8 black_sensor_serial_flag;
  u8 Segment_Num;
  SEGMENT_OPTION seg_list[MAX_SEGMENT_NUM];
}SENSOR_STATUS_NEW;

//extern SENSOR_STATUS SENSOR_Status;
extern SENSOR_STATUS_NEW SENSOR_STATUS_New;
//通信数据格式定义
struct MODBUS
{
   u8  ModBus_CMD;
   u8  Probe_Slave_Add;
   u16 Read_Register_Add;
   u16 Write_Register_Add;
   u16 Read_Register_Num;
   u16 Read_index;
   u8  Write_Register_Data[120];
   u16 Write_Register_Num;
   u16 Write_Register_Data_One;
   //u8  A8_Buffer_1[256];
   //u8  A8_Buffer_2[256];
   //u8  A8_Buffer_3[256];
   u8  Probe_state;
   u8  err_state;//返回错误代码。
};

typedef struct 
{
  u8 MachineState;
  u8 BufIndex;
  u8 read_receive_timer;
  u8 receive_CRC_H;
  u8 receive_CRC_L;
  u8 rev0;
  
  u8 ModBus_CMD;
  u8 err_state;
  u32 read_success_num;
  u32 write_success_num;
  u16 Read_Register_Num;
  u8 DataBuf[256];
}MODBUS_SAMPLE;

extern struct  MODBUS  A8_Modbus;
extern u8 Receive_Data_From_A8[256];
extern u8 HallSensorMachineState;

extern u8 calculate_CRC_H;
extern u8 calculate_CRC_L;
extern u8 Send_Data_A8_array[256];

extern u8 ON_LINE_Flag;

//ModBus 通信格式
//将从控制器 A8接收到的数据，进入状态机，进行数据检索。
extern void Analysis_Receive_From_ModeBusSlaveDev(unsigned char data);
extern u16 ModBus_CRC16_Calculate(unsigned char *aStr , unsigned char alen);






void MODBUS_READ_SERSOR_BOARD_TASK(void);
u8 CheckHallOnListNum(u8* hall_list,u8 total_num,SENSOR_STATUS* St);
u8 CheckHallOnListNumNew(u8* hall_list,u8 total_num,SENSOR_STATUS_NEW* St);
u8 GetSensorMiddleIndex(SENSOR_STATUS_NEW* st);

/////////////分叉////////////////////////////
#define SELECT_DIR_NULL   0
#define SELECT_DIR_LEFT   1
#define SELECT_DIR_RIGHT  2

#define SELECT_DIR_MASK   0x03
#define SELECT_DIR_START_FLAG 0x80
#define SELECT_DIR_SINGLE_LINE_TIMES_MASK 0x7c
#define SELECT_DIR_SINGLE_LINE_TIMES_UP_STEP 0x4

extern u8 SelectDir;
extern u16 MB_LINE_DIR_SELECT;
extern u8 MODE_BUS_HALL_Addr;
#endif
