#ifndef _DIDO_H_
#define _DIDO_H_



#define CMD_ModBus_Wire_Read         0x01
#define CMD_ModBus_Wire_ReadEx       0x02
#define CMD_ModBus_Wire_Write        0x05
#define CMD_ModBus_Wire_WriteMore    0x15

typedef struct
{
  u8 RelayStatus;
  u8 LightStatus;
  u8 RefreshFlag;
}DIDO_INPUT_STATUS;

typedef enum
{
  DO_LED_Red = 0,
  DO_LED_Yellow,
  DO_LED_Green,
  DO_Buzzer,
  DO_Fan_1,
  DO_Fan_2,
  DO_Reserve_1,
  DO_Reserve_2,
  DO_Num
}DIDO_OUT_INDEX;

typedef enum
{
  DI_TOUCH_HEAD = 0,
  DI_TOUCH_TAIL,
  DI_IM_STOP_1,
  DI_IM_STOP_2,
  DI_Reserve_1,
  DI_Reserve_2,
  DI_Reserve_3,
  DI_Reserve_4,  
  DI_Num
}DIDO_IN_INDEX;


extern u16 DIDO_COMM_Timeout;
extern u16 DIDO_READ_LIGHT_Timeout;
extern u16 DIDO_ENABLE_Timeout;
extern MODBUS_SAMPLE MODBUS_Dido;
extern DIDO_INPUT_STATUS DIDO_INPUT_Status;
extern u8 DIDO_RelayStatus;

void Analysis_Receive_From_Dido(u8 data,MODBUS_SAMPLE* pMODBUS, DIDO_INPUT_STATUS* st);
void Check_DIDO_TASK(void);
void SET_DIDO_Relay(u8 index,u8 status);

#endif