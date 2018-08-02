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
  DIDO_LED_Blue = 0,
  DIDO_LED_Green,
  DIDO_LED_Red,
  DIDO_LED_Eye,
  DIDO_Fan,
  DIDO_Buzzer,
  DIDO_Breath,
  DIDO_Reserve
}DIDO_OUT_INDEX;

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