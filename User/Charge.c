#include "user_inc.h"

#define CHARGE_PRINTF_DEBUG         1
#define DEFAULT_CHARGE_MODULE_ADDR  0x01
#define CHARGE_COMM                 1

const u8 CMD_CHARGE_START[8] = {0x01, 0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,0xCD ,0xCA};  // 启动
const u8 CMD_CHARGE_STOP[8] = {0x01, 0x05 ,0x00 ,0x00 ,0x00 ,0x01 ,0x0C ,0x0A};   // 停止
const u8 CMD_CHARGE_ANALOG[8] = {0x01, 0x03, 0x00, 0x64, 0x00, 0x0A, 0x84, 0x12}; //{0x01, 0x01, 0x00, 0x64, 0x00, 0x06, 0xFD, 0xD7}; // 

// <- 01 05 00 00 00 00 CD CA 
// <- 01 05 00 00 00 01 0C 0A 
// <- 01 01 06 06 01 00 00 00 00 9D 0A 
// <- 01 03 0B 00 00 00 00 00 00 00 00 06 01 E2 2A 
// <- 01 03 0B 0A 5A 02 4E 00 2D 00 05 13 10 1D EF 

MODBUS_SAMPLE MODBUS_Charge = {
  .MachineState = 0,
  .read_success_num = 0,
  .write_success_num = 0,
};
u32 CHARGE_COMM_Timout = 3500;
static u8 SET_CHARGE_OnOff = 0;
u8 CHARGE_FULL_Flag = 0;
CHARGE_STATUS CHARGE_St = 
{
  .Refresh = 0,
  .RxNum = 0,
};

void Analysis_Receive_From_Charge(u8 data,MODBUS_SAMPLE* pMODBUS)
{
    switch(pMODBUS->MachineState)//初始化 默认 为 00;
    {
    case 0:
      {
        if(data == DEFAULT_CHARGE_MODULE_ADDR)//从机地址
        {
          pMODBUS->MachineState = 0x01;
          pMODBUS->BufIndex = 0;
          pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        }
        else
        {
          pMODBUS->MachineState = 0x0B;//缓冲数据区域清零要处理，中间数据为01，误认为是要从机地址。
          pMODBUS->BufIndex = 0;
        }  
      }
      break;
      case 0x01:
      {	 
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        if(data == CMD_ModBus_ReadEx) //执行读取单个或多个寄存器命令  0x01 
        {
          pMODBUS->MachineState = 0x02; 
          pMODBUS->ModBus_CMD = data;
          pMODBUS->read_receive_timer = 0;
        }
        //else if(data == CMD_ModBus_Wire_Write)
        //{
        //  pMODBUS->MachineState = 0x04; 
        //  pMODBUS->ModBus_CMD = data;
        //  pMODBUS->read_receive_timer = 0;
        //}
        else
        { 
          pMODBUS->MachineState = 0x0B;
          pMODBUS->BufIndex = 0;
        }
      }
      break;
      case 0x02: //read part 00
      {    
        //接收到读功能的字节数
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        pMODBUS->read_receive_timer++;
        if(pMODBUS->read_receive_timer == 1 )
        {
          pMODBUS->Read_Register_Num = pMODBUS->DataBuf[pMODBUS->BufIndex-1] - 1; // charge fix spec
          if(pMODBUS->Read_Register_Num <= 16)//长度限定
          {
            pMODBUS->MachineState = 0x03;
          }
          else
          {
            pMODBUS->MachineState = 0x00;
          }
          pMODBUS->read_receive_timer = 0;
        } 
      }
      break;
      case 0x03: //read part 01
      {   
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        pMODBUS->read_receive_timer++;
        if(pMODBUS->read_receive_timer >= (pMODBUS->Read_Register_Num+2))
        {
          u16 cal_crc;
          cal_crc=ModBus_CRC16_Calculate(pMODBUS->DataBuf,pMODBUS->Read_Register_Num+3);
              
          pMODBUS->receive_CRC_L = pMODBUS->DataBuf[pMODBUS->BufIndex-2];
          pMODBUS->receive_CRC_H = pMODBUS->DataBuf[pMODBUS->BufIndex-1];
          if(((cal_crc>>8) == pMODBUS->receive_CRC_H) && ((cal_crc&0xFF) == pMODBUS->receive_CRC_L))
          {
            u8* ptr = pMODBUS->DataBuf + 3;
            pMODBUS->err_state = 0x00;//CRC校验正确 
            pMODBUS->read_success_num += 1;
            //--------------//
            CHARGE_St.Voltage = Get_BD_U16(&ptr);
            CHARGE_St.Current = Get_BD_U16(&ptr);
            CHARGE_St.Cap = Get_BD_U16(&ptr);
            CHARGE_St.Time = Get_BD_U16(&ptr);
            
            CHARGE_St.Refresh = 1;
            CHARGE_St.RxNum += 1;
            //--------------//
          }    
          else	  
          {
             pMODBUS->err_state = 0x04;
          }   
           pMODBUS->BufIndex = 0;  
           pMODBUS->read_receive_timer = 0;  
           pMODBUS->MachineState = 0x00;                
          }
        }
        break;
        case 0x04: //write
          {
            pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
            pMODBUS->read_receive_timer++;
            if( pMODBUS->read_receive_timer == 6 )
            {
              u16 cal_crc;
              cal_crc=ModBus_CRC16_Calculate(pMODBUS->DataBuf,6);
              
              pMODBUS->receive_CRC_L = pMODBUS->DataBuf[pMODBUS->BufIndex-2];
              pMODBUS->receive_CRC_H = pMODBUS->DataBuf[pMODBUS->BufIndex-1];
              if(((cal_crc>>8) == pMODBUS->receive_CRC_H) && ((cal_crc&0xFF) == pMODBUS->receive_CRC_L))
              {
                pMODBUS->err_state = 0x00;//CRC校验正确 
                pMODBUS->write_success_num += 1;
                
              }    
              else	  
              {
                pMODBUS->err_state = 0x04;
              }   
              pMODBUS->BufIndex = 0;  
              pMODBUS->read_receive_timer = 0;  
              pMODBUS->MachineState = 0;   
            }
          }
        break;
      case 0xb:
        {
        
        }
        break;  
      default:
        {
          pMODBUS->MachineState=0;
        }
    }
}

void SET_Charge(u8 on_off)
{
  if(on_off)
  {
    SET_CHARGE_OnOff = 0x81;
  }
  else
  {
    SET_CHARGE_OnOff = 0x80;
  }
}

void CHARGE_Task(void)
{
  if((CHARGE_COMM_Timout == 0) && (AGV_RUN_Pro == AGV_STATUS_CHARGE))
  {
    CHARGE_COMM_Timout = 1000;
    
    if(SET_CHARGE_OnOff & 0x80)
    {
      if(SET_CHARGE_OnOff & 0x01)
      {
        FillUartTxBufN((u8*)CMD_CHARGE_START, sizeof(CMD_CHARGE_START), CHARGE_COMM);
      }
      else
      {
        FillUartTxBufN((u8*)CMD_CHARGE_STOP, sizeof(CMD_CHARGE_STOP), CHARGE_COMM);
      }
      SET_CHARGE_OnOff = 0;
    }
    else //if 
    {
      FillUartTxBufN((u8*)CMD_CHARGE_ANALOG, sizeof(CMD_CHARGE_ANALOG), CHARGE_COMM);
    }
  }
  
  if(CHARGE_St.Refresh)
  {
    static u32 counter = 0;
    CHARGE_St.Refresh = 0;
    if(AGV_RUN_Pro == AGV_STATUS_CHARGE)
    {
      if(CHARGE_St.Current <= MIN_CHARGE_CURRENT_IN_0D01A)
      {
        if(counter < 120)
        {
          counter += 1;
        }
        else
        {
          CHARGE_FULL_Flag = 1;
#if(CHARGE_PRINTF_DEBUG)
          printf("-- CHARGE_FULL --\n");
#endif
        }
      }
      else
      {
        counter = 0;
      }
    }
    else
    {
      CHARGE_FULL_Flag = 0;
      counter = 0;
    }
  }
}