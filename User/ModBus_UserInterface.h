#ifndef _ModBus_2_h_
#define _ModBus_2_h_


#define DEFAULT_RFID_ONLINE_TIME_IN_MS  3000
#define U_TX_INDEX  2   //使用串口1发送

#define AUTO_FOLLOW_SPEED_CONTROL_MODE_ANALOG   0 //电位器
#define AUTO_FOLLOW_SPEED_CONTROL_MODE_DIGITAL  1 //MODBUS寄存器

typedef struct
{
  u16 M_CONTROL_MODE;
  u16 COMM_BD ;
  u16 SLAVE_ADDR;
  u16 AUTO_FOLLOW_ENABLE;
//--------------------------------
  //自动模式下速度控制模式的选择，0-车身电位器，1-MODBUS寄存器中的值
  u16 AUTO_FOLLOW_SPEED_CONTROL_MODE;
  u16 AUTO_FOLLOW_SPEED;//0~100
  u32 RFID_WAIT_TIME_IN_MS;//
  u16 RFID_ONLINE_TIME_IN_MS;

  
//--------------------------------  
  u16 EVEN_ODD_FILL;//保证是偶数，可能有，可能无
  u32 MOD_REG_MAGIC_WORD;
}MOD_BUS_REG;
#define MAGIC_WORD  0x1A2B3C4A
extern const MOD_BUS_REG DEFAULT_MOD_BUS_Reg;
extern MOD_BUS_REG MOD_BUS_Reg;
#define MOD_BUS_BD_LIST_LENGTH  9
extern const u32 MOD_BUS_BD_LIST[MOD_BUS_BD_LIST_LENGTH];
extern u16 RecoverFlash_Timeout;
u8 AckModBusReadReg(u16 reg_addr,u16 reg_num);
u8 AckModBusCrcError(u8 CMD_ModBus);
u8 AckModBusWriteOneReg(u16 reg_addr,u16 reg_value);
u8 AckModBusWriteMultiReg(u16 reg_addr,u16 reg_num,u8* pData);
u8 AckModBusFunctionError(u8 CMD_ModBus);
void GetFlashModBusData(MOD_BUS_REG* pMOD_BUS_Reg);
void MOD_BUS_REG_Backup(void);
void MOD_BUS_REG_MODIFY_Check(void);
extern FLASH_Status SaveFlashModBusData(MOD_BUS_REG* pMOD_BUS_Reg);

extern u16 M_Status;
extern u8 Send_Data_A8_array[256];

#define M_CONTROL_MODE_FOLLOW_LINE      0  // 巡线模式
#define M_CONTROL_MODE_SOFTWARE_STOP    1  // 空闲/急停模式
#define M_CONTROL_MODE_BACK_TO_ORIGN    2  // 返回起点 


#define M_STATUS_NOMAL  0
#define M_STATUS_STOP   1
#define M_STATUS_IM_STOP  2

#define M_CMD_STOP  0
#define M_CMD_FORWARD  1
#define M_CMD_BACKWARD  2
#define M_CMD_LEFT  3
#define M_CMD_RIGHT  4


typedef union
{
  u16 AsU16[2+1+2];
  struct
  {
    u16 M_Dir;
    u16 M_Speed;
    u16 M_FreshFlag;
    s16 LeftSpeed;
    s16 RightSpeed;
  }M_CONTROL_OPTION;
}U_M_CONTROL_OPTION;

extern u16 M_BAT_Precent;
extern u16 M_LightSensorStatus[6];
extern U_M_CONTROL_OPTION U_M_CONTROL_Op;
extern u8 machine_state;

void Analysis_Receive_From_A8(u8 data);
#endif