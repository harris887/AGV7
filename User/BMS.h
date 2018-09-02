#ifndef _bms_h_
#define _bms_h_


#define DEFAULT_BMS_READ_START_DELAY  2000
#define DEFAULT_BMS_COMM_CYCLE        1000


#define BMS_SOI         0x7E 
#define BMS_EOI         0x0D

#define DEFAULT_BMS_ADDR 0x01
#define DEFAULT_BMS_VER  0x25
#define DEFAULT_BAT_CID1 0x46


#define BMS_CID2_GET_PACK_ANALOG        0x42
#define BMS_CID2_GET_PACK_WARNING       0x44
#define BMS_CID2_GET_PACK_NUM           0x90
#define BMS_CID2_CONTROL                0x99
#define BMS_CID2_BAT_CHARGE_MOS_CONTROL 0x9A
#define BMS_CID2S_BAT_OUT_MOS_CONTROL   0x9B

#define BMS_CHARGE_MOS_CHANGE_FLAG      0x80
#define BMS_CHARGE_MOS_OPEN             0x00
#define BMS_CHARGE_MOS_CLOSE            0x01

#define BMS_PACK_ALL                    0xFF


typedef struct
{
  u8 INFOR_Flag;
  u8 PACK_Num;
  //------------------
  u8 CELL_Num;
  u16 CELL_Vol[16];
  u8 TEMP_Num;
  u16 TEMP_Value[16];
  u16 PACK_Current;
  u16 PACK_Vol;
  u16 PACK_Left;
  u8 USER_DefineNum;
  u16 PACK_TotalCap;
  u16 BAT_Cycle;
  u16 BAT_Cap;
  //------------------
  u8 Refresh;
  u32 COMM_Num;
}PACK_ANALOG_INFOR;

typedef struct
{
  u8 INFOR_Flag;
  u8 PACK_Num;
  //------------------
  u8 CELL_Num;
  u8 CELL_Vol[16];
  u8 TEMP_Num;
  u8 TEMP_Value[16];
  u8 PACK_CHARGE_Current;
  u8 PACK_Vol;
  u8 PACK_OUT_Current;
  u8 WARN_Status[9];
  //------------------
  u8 Refresh;
  u32 COMM_Num;
}PACK_WARN_INFOR;

extern u8 BMS_RX_Pro;
extern u8 BMS_CURRENT_Cmd;
extern u16 BMS_TimeOutCounter;
extern const char BMS_INFOR_ACK[];
extern const char BMS_WARN_ACK[];
extern PACK_ANALOG_INFOR PACK_ANALOG_Infor;
extern PACK_WARN_INFOR PACK_WARN_Infor;

extern void SendBmsCommand(u8 ADR, u8 CID2, u8 infor);
extern void Set_BMS_CHARGE_MOS(u8 status);
extern void Handle_BmsRx(u8 data);
extern void BMS_Task(void);
extern u16 Get_BD_U16(u8** beam); 
#endif