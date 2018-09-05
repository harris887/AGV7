#include "user_inc.h"
#include "string.h"

#define BMS_COMM_ENUM         4
u16 BMS_TimeOutCounter = DEFAULT_BMS_READ_START_DELAY;
#define BMS_RX_BUF_SIZE   256
u8 BMS_RX_Buf[BMS_RX_BUF_SIZE];
u8 BMS_BYTE_Buf[BMS_RX_BUF_SIZE >> 1];
u8 BMS_RX_BufIndex = 0;
u8 BMS_RX_Pro = 0;
u8 BMS_CURRENT_Cmd = 0;

static u8 CHARGE_MOS_Control = 0;
u32 GetBmsInforNum = 0;

const char BMS_INFOR_ACK[] = "~2500460090520001080C140C140C0C0C110C0E0C140C240C25040B970B990B980BA5000060B00B5A030FA000000FA0EC32\r";
const char BMS_WARN_ACK[] = "~25004600503800010800000000000000000400000000000000000006100000000000F30B\r";

PACK_ANALOG_INFOR PACK_ANALOG_Infor = 
{
  .Refresh = 0,
  .COMM_Num = 0,
};

PACK_WARN_INFOR PACK_WARN_Infor = 
{
  .Refresh = 0,
  .COMM_Num = 0,
};

u16 Get_BD_U16(u8** beam) 
{
  u16 temp;
  u8* c = *beam;
  temp = ((u16)c[0] << 8) | ((u16)c[1] << 0);
  *beam = c + 2;
  return temp;
}

u32 Get_BD_U32(u8** beam) 
{
  u32 temp;
  u8* c = *beam;
  temp = ((u32)c[0] << 24) | ((u32)c[1] << 16) | ((u32)c[2] << 8) | ((u32)c[3] << 0);
  *beam = c + 4;
  return temp;
}

void Byte2HexAscii(u8 value,char* str)
{
  const char* hex = "0123456789ABCDEF";
  str[0] = hex[value >> 4];
  str[1] = hex[value & 0xF];
}

u16 Byte2HexStr(u8* data, u8 len, u8* str)
{
  u8 i;
  const char* hex_str = "0123456789ABCDEF";
  for(i = 0; i < len; i++)
  {
    *str++ = hex_str[data[i] >> 4];
    *str++ = hex_str[data[i] & 0xF];
  }
  *str++ = '\0'; //'\0'
  return (len * 2) + 1;
}

u16 HexStr2Byte(u8* str, u8 len, u8* bytes)
{
  u8 i, H4, L4;
  for(i = 0; i < len; i += 2)
  {
    if((str[i] >= '0') && (str[i] <= '9')) H4 = str[i] - '0';
    else if((str[i] >= 'A') && (str[i] <= 'F')) H4 = str[i] - 'A' + 10;
    else if((str[i] >= 'a') && (str[i] <= 'f')) H4 = str[i] - 'a' + 10;
    else H4 = 0;
    if((str[i + 1] >= '0') && (str[i + 1] <= '9')) L4 = str[i + 1] - '0';
    else if((str[i + 1] >= 'A') && (str[i + 1] <= 'F')) L4 = str[i + 1] - 'A' + 10;
    else if((str[i + 1] >= 'a') && (str[i + 1] <= 'f')) L4 = str[i + 1] - 'a' + 10;
    else L4 = 0;
    
    bytes[i >> 1] = (H4 << 4) | L4;
  }
  return len >> 1;
}

u16 BMS_CheckSum(u8* data, u8 len)
{
  u16 tmp = 0, i;
  for(i = 0; i < len; i++)
  {
    tmp += data[i];
  }
  tmp = (~tmp) + 1;
  return tmp;
}

// 仅适用于INFOR为1字节的命令
// -> 7E 32 35 30 31 34 36 34 32 45 30 30 32 30 31 46 44 33 30 0D
// <- 7E 32 35 30 30 34 36 30 30 39 30 35 32 30 30 30 31 30 38 30 43 31 34 30 43 31 34 30 43 30 43 30 43 31 31 30 43 30 45 30 43 31 34 30 43 32 34 30 43 32 35 30 34 30 42 39 37 30 42 39 39 30 42 39 38 30 42 41 35 30 30 30 30 36 30 42 30 30 42 35 41 30 33 30 46 41 30 30 30 30 30 30 46 41 30 45 43 33 32 0D
// <- ~2500460090520001080C140C140C0C0C110C0E0C140C240C25040B970B990B980BA5000060B00B5A030FA000000FA0EC32

// -> 7E 32 35 30 31 34 36 34 34 45 30 30 32 30 31 46 44 32 45 0D
// <- ~25004600503800010800000000000000000400000000000000000006100000000000F30B
static char buf[32];
void SendBmsCommand(u8 ADR, u8 CID2, u8 infor)
{
  char index = 0;
  u16 check_sum = 0;
  buf[index++] = BMS_SOI;
  //------------------------------//
  Byte2HexAscii(DEFAULT_BMS_VER, buf + index);
  index += 2;
  Byte2HexAscii(ADR, buf + index);
  index += 2;  
  Byte2HexAscii(DEFAULT_BAT_CID1, buf + index);
  index += 2;   
  Byte2HexAscii(CID2, buf + index);
  index += 2;    
  Byte2HexAscii(0xE0, buf + index);
  index += 2;   
  Byte2HexAscii(0x02, buf + index);
  index += 2;   
  Byte2HexAscii(infor, buf + index);
  index += 2;   
  
  check_sum = BMS_CheckSum(buf + 1, index - 1);
  
  Byte2HexAscii(check_sum >> 8, buf + index);
  index += 2;   
  Byte2HexAscii(check_sum& 0xFF, buf + index);
  index += 2;   
  //------------------------------//
  buf[index++] = BMS_EOI;
  
  FillUartTxBufN((u8*)buf, index, BMS_COMM_ENUM);
}

void BMS_Task(void)
{
  static u8 Read_index = 0;
  if(BMS_TimeOutCounter == 0)
  {
    BMS_TimeOutCounter = DEFAULT_BMS_COMM_CYCLE;
    if(CHARGE_MOS_Control & BMS_CHARGE_MOS_CHANGE_FLAG)
    {
      CHARGE_MOS_Control &= (~BMS_CHARGE_MOS_CHANGE_FLAG);
      //SendBmsCommand(DEFAULT_BMS_ADDR, BMS_CID2_BAT_CHARGE_MOS_CONTROL, CHARGE_MOS_Control);
    }
    else
    {
      if(Read_index & 0x1)
      {
        SendBmsCommand(DEFAULT_BMS_ADDR, BMS_CID2_GET_PACK_ANALOG, 0x01); // BMS_PACK_ALL
        BMS_CURRENT_Cmd = BMS_CID2_GET_PACK_ANALOG;
      }
      else
      {
        SendBmsCommand(DEFAULT_BMS_ADDR, BMS_CID2_GET_PACK_WARNING, 0x01);
        BMS_CURRENT_Cmd = BMS_CID2_GET_PACK_WARNING;
      }
      Read_index += 1;
      GetBmsInforNum += 1;
    }
  }
}

void Set_BMS_CHARGE_MOS(u8 status)
{
  //CHARGE_MOS_Control = BMS_CHARGE_MOS_CHANGE_FLAG | status;
}

// 处理接收到的数据
void Handle_BmsRx(u8 data)
{
  switch(BMS_RX_Pro)
  {
  case 0:
    {
      if(data == BMS_SOI)
      {
        BMS_RX_Pro += 1;
        BMS_RX_BufIndex = 0;
        BMS_RX_Buf[BMS_RX_BufIndex++] = data;
      }
    }
    break;
  case 1:
    {
      BMS_RX_Buf[BMS_RX_BufIndex++] = data;
      if(data == BMS_EOI)
      {
        if(BMS_RX_BufIndex >= 14)
        {
          u8 infor_bytes;
          u16 check_sum = BMS_CheckSum(BMS_RX_Buf + 1, BMS_RX_BufIndex - 6);
          infor_bytes = HexStr2Byte(BMS_RX_Buf + 1, BMS_RX_BufIndex - 2, BMS_BYTE_Buf);
          
          if((0 == BMS_BYTE_Buf[3]) && 
             (check_sum == ((BMS_BYTE_Buf[infor_bytes - 2] << 8) | BMS_BYTE_Buf[infor_bytes - 1]) ))
          {
            // 解析数据
            if(BMS_CURRENT_Cmd == BMS_CID2_GET_PACK_ANALOG)
            {
              u8* ptr = BMS_BYTE_Buf + 6, i;
              PACK_ANALOG_Infor.INFOR_Flag = *ptr++;
              PACK_ANALOG_Infor.PACK_Num = *ptr++;
              PACK_ANALOG_Infor.CELL_Num = *ptr++;
              if((PACK_ANALOG_Infor.CELL_Num > 0) && (PACK_ANALOG_Infor.CELL_Num <= 16))
              {
                for(i = 0; i < PACK_ANALOG_Infor.CELL_Num; i++)
                {
                  PACK_ANALOG_Infor.CELL_Vol[i] = Get_BD_U16(&ptr); 
                }
              }
              PACK_ANALOG_Infor.TEMP_Num = *ptr++;
              if((PACK_ANALOG_Infor.TEMP_Num > 0) && (PACK_ANALOG_Infor.TEMP_Num <= 16))
              {
                for(i = 0; i < PACK_ANALOG_Infor.TEMP_Num; i++)
                {
                  PACK_ANALOG_Infor.TEMP_Value[i] = Get_BD_U16(&ptr); 
                }
              }
              PACK_ANALOG_Infor.PACK_Current = Get_BD_U16(&ptr); 
              PACK_ANALOG_Infor.PACK_Vol = Get_BD_U16(&ptr); 
              PACK_ANALOG_Infor.PACK_Left = Get_BD_U16(&ptr); 
              PACK_ANALOG_Infor.USER_DefineNum = *ptr++;
              PACK_ANALOG_Infor.PACK_TotalCap = Get_BD_U16(&ptr); 
              PACK_ANALOG_Infor.BAT_Cycle = Get_BD_U16(&ptr); 
              PACK_ANALOG_Infor.BAT_Cap = Get_BD_U16(&ptr);  
              
              PACK_ANALOG_Infor.Refresh = 1;
              PACK_ANALOG_Infor.COMM_Num += 1;
            }
            else if(BMS_CURRENT_Cmd == BMS_CID2_GET_PACK_WARNING)
            {
              u8* ptr = BMS_BYTE_Buf + 6, i;
              PACK_WARN_Infor.INFOR_Flag = *ptr++;
              PACK_WARN_Infor.PACK_Num = *ptr++;
              PACK_WARN_Infor.CELL_Num = *ptr++;
              if((PACK_WARN_Infor.CELL_Num > 0) && (PACK_WARN_Infor.CELL_Num <= 16))
              {
                memcpy(PACK_WARN_Infor.CELL_Vol, ptr, PACK_WARN_Infor.CELL_Num); 
                ptr += PACK_WARN_Infor.CELL_Num;                
              }
              PACK_WARN_Infor.TEMP_Num = *ptr++;
              if((PACK_WARN_Infor.TEMP_Num > 0) && (PACK_WARN_Infor.TEMP_Num <= 16))
              {
                memcpy(PACK_WARN_Infor.TEMP_Value, ptr, PACK_WARN_Infor.TEMP_Num); 
                ptr += PACK_WARN_Infor.TEMP_Num;
              }
              PACK_WARN_Infor.PACK_CHARGE_Current = *ptr++;
              PACK_WARN_Infor.PACK_Vol = *ptr++; 
              PACK_WARN_Infor.PACK_OUT_Current = *ptr++; 
              memcpy(PACK_WARN_Infor.WARN_Status, ptr, 9); 
              
              PACK_WARN_Infor.Refresh = 1;
              PACK_WARN_Infor.COMM_Num += 1;
            }
          }
        }
        BMS_RX_Pro = 0;
      }
    }
    break;
  }
}
