#include "user_inc.h"
#include "string.h"

#define BMS_PRINTF_DEBUG      0

/*
const u8* READ_PROTECT_PARAM = ":000100000E09~";
const u8 BMS_READ_STATUS_ALL[6] = {0x06 ,0x01 ,0x10 ,0x00 ,0x00 ,0x17};
#define BMS_TX_BUF_SIZE   32

u8 BMS_TX_Buf[BMS_TX_BUF_SIZE];
u8 BMS_RX_Buf[BMS_RX_BUF_SIZE];
u8 BMS_RX_BufIndex = 0;
BMS_STATUS BMS_St = {0,0,0,0,0,0,0,0,0,0};
*/

#define BMS_COMM_ENUM      4
u16 BMS_TimeOutCounter = DEFAULT_BMS_READ_START_DELAY;
#define BMS_RX_BUF_SIZE   256
u8 BMS_RX_Buf[BMS_RX_BUF_SIZE];
u8 BMS_RX_BufIndex = 0;
u8 BMS_RX_Pro = 0;

static u8 CHARGE_MOS_Control = BMS_CHARGE_MOS_CHANGE_FLAG | BMS_CHARGE_MOS_CLOSE;
u32 GetBmsInforNum = 0;
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
void SendBmsCommand(u8 ADR, u8 CID2, u8 infor)
{
  char buf[32], index = 0;
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
      SendBmsCommand(DEFAULT_BMS_ADDR, BMS_CID2_BAT_CHARGE_MOS_CONTROL, CHARGE_MOS_Control);
    }
    else
    {
      if(Read_index & 0x1)
      {
        SendBmsCommand(DEFAULT_BMS_ADDR, BMS_CID2_GET_PACK_ANALOG, BMS_PACK_ALL);
      }
      else
      {
        SendBmsCommand(DEFAULT_BMS_ADDR, BMS_CID2_GET_PACK_WARNING, BMS_PACK_ALL);
      }
      Read_index += 1;
      GetBmsInforNum += 1;
    }
  }
}

void Set_BMS_CHARGE_MOS(u8 status)
{
  CHARGE_MOS_Control = BMS_CHARGE_MOS_CHANGE_FLAG | status;
}

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
          u8 check_sum_str[8];
          u16 check_sum = BMS_CheckSum(BMS_RX_Buf + 1, BMS_RX_BufIndex - 6);
          check_sum = (check_sum >> 8) | (check_sum << 8);
          Byte2HexStr((u8*)&check_sum, sizeof(u16), check_sum_str);
          if(0 == memcmp(check_sum_str, BMS_RX_Buf + BMS_RX_BufIndex - 1 - (sizeof(u16) * 2), sizeof(u16) * 2))
          {
            // 解析数据
            
          }
        }
        BMS_RX_Pro = 0;
      }
    }
    break;
  }
}

#if (0)
void Byte2HexAscii(u8 value,char* str)
{
  const char* hex="0123456789ABCDEF";
  str[0] = hex[value>>4];
  str[1] = hex[value&15];
}

u8 BMS_Crc(u8* data, u16 len)
{
  u16 i;
  u8 tmp=0;
  for(i=1;i<(len-3);i++)
  {
    tmp += data[i];
  }
  return tmp^0xFF;
}



u8 NewBMSFrame(u8 cmd, u8* buf)
{
  u16 len;
  u8 crc;
  BMS_FRAME* pF = (BMS_FRAME*)buf;
  pF->SOI = BMS_SOI;
  Byte2HexAscii(DEFAULT_BMS_ADDR, pF->Addr);
  Byte2HexAscii(cmd, pF->Cmd);
  Byte2HexAscii(DEFAULT_BMS_VAR, pF->Ver);
  len = sizeof(BMS_FRAME);
  
  switch(cmd)
  {
  case CMD_GET_PROTECT_PARAM:
    {
      buf[len-1] = BMS_EOI;
      Byte2HexAscii(len>>8, pF->Len);
      Byte2HexAscii(len&0xFF, pF->Len+2);     
      crc = BMS_Crc(buf, len);
      Byte2HexAscii(crc, buf+len-3); 
    }
    break;
  }
  return len;
}

u8 BMS_ReadStatus(void)
{
  u8 len = sizeof(BMS_READ_STATUS_ALL);
  FillUartTxBuf_NEx((u8*)BMS_READ_STATUS_ALL, len, CH_BMS);
  BMS_St.tx_num += 1;
  return len;
}

void Handle_BmsRx(u8* data, u8 len)
{
  if((len+BMS_RX_BufIndex)<=BMS_RX_BUF_SIZE)
  {
    memcpy(BMS_RX_Buf+BMS_RX_BufIndex, data, len);
    BMS_RX_BufIndex += len;
    if(BMS_RX_BufIndex == sizeof(BMS_STATUS_FRAME))
    {
      u8 valid=0;
      BMS_STATUS_FRAME* pBMS_St = (BMS_STATUS_FRAME*)BMS_RX_Buf;
      if(pBMS_St->check_sum == BMS_CheckSum(BMS_RX_Buf, BMS_RX_BufIndex-1))
        valid += 1;
      if(pBMS_St->func_code == BMS_FUNC_CODE_STATUS)
        valid += 1;
      if(valid == 2)
      {
        BMS_St.ack_num += 1;
        BMS_St.voltage_mv = (pBMS_St->voltage_h<<8) | pBMS_St->voltage_l;
        BMS_St.curr_ma = (pBMS_St->curr_h<<8) | pBMS_St->curr_l;
        BMS_St.out_ele_num = (pBMS_St->out_ele_num_h<<8) | pBMS_St->out_ele_num_l;
        BMS_St.in_ele_num = (pBMS_St->in_ele_num_h<<8) | pBMS_St->in_ele_num_l;
        BMS_St.status = (pBMS_St->status_h<<8) | pBMS_St->status_l;
        BMS_St.temprature = pBMS_St->temprature;
        BMS_St.capacity = pBMS_St->capacity;
      }
      BMS_RX_BufIndex -= sizeof(BMS_STATUS_FRAME);
    }
  }
}

u16 Byte2HexStr(u8* data, u8 len, u8* str)
{
  u8 i;
  const char* hex_str = "0123456789ABCDEF";
  for(i=0; i<len; i++)
  {
    *str++ = hex_str[data[i]>>4];
    *str++ = hex_str[data[i]&15];
  }
  *str++ = '\n'; //'\0'
  return len*2+1;
}

void Flush_BmsRx(void)
{
#if (BMS_PRINTF_DEBUG)
  static u8 buf[256];  
  if(USART_BYTE == 'M')
  {
    if(BMS_RX_BufIndex != 0)
    {
      u16 strlen = Byte2HexStr(BMS_RX_Buf, BMS_RX_BufIndex, buf);
      FillUartTxBufN(buf, strlen, 1);
    }
  }
#endif
  BMS_RX_BufIndex = 0;
}

u8 BMS_ResetCheck(void)
{
  if(BMS_St.tx_num >= (BMS_St.ack_num+DEFAULT_BMS_RESET_LOSE_NUM))
  {
    BMS_St.tx_num = BMS_St.ack_num;
    BMS_St.reset_num += 1;
    return 1;
  }
  return 0;
}
#endif