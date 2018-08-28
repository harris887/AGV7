#include "user_inc.h"
#include "string.h"

#define MIN_MONITOR_DEV_ADDR  1
#define MAX_MONITOR_DEV_ADDR  MOTO_NUM
MONITOR_STATUS MONITOR_St[MOTO_NUM]={{0,0,0,0,0},{0,0,0,0,0}};
float COFF_001RPM_TO_MMS = 0.0633554;
//float COFF_MMS_TO_RPM = 0.1578396;
float COFF_MMS_TO_D1RPM = 1.578396;
float COFF_01RPM_TO_MMS = 0.633554;
float COFF_DISTANCE = 1.0;
MODBUS_SAMPLE MODBUS_Monitor = {
  .MachineState = 0,
  .read_success_num = 0,
  .write_success_num = 0,
};

void MONITOR_STATUS_Init(void)
{
  memset(MONITOR_St,0,sizeof(MONITOR_St));
}

void Analysis_Receive_From_Monitor(u8 data,MODBUS_SAMPLE* pMODBUS, MONITOR_STATUS* st)
{
    switch(pMODBUS->MachineState)//初始化 默认 为 00;
    {
    case 0:
      {
        if((data >= MIN_MONITOR_DEV_ADDR)&&(data <= MAX_MONITOR_DEV_ADDR))//从机地址
        {
          pMODBUS->MachineState = 0x01;
          pMODBUS->BufIndex = 0;
          pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        }
        else
        {
          pMODBUS->MachineState = 0x0B;//缓冲数据区域清零要处理，中间数据为01，误认为是要从机地址。
          //pMODBUS->MachineState = 0;
          pMODBUS->BufIndex = 0;
        }  
      }
      break;
      case 0x01:
      {	 
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        if(data == CMD_ModBus_ReadEx) 
        {
          pMODBUS->MachineState = 0x02; 
          pMODBUS->ModBus_CMD = data;
          pMODBUS->read_receive_timer = 0;
        }
        else if(data == CMD_ModBus_Write)
        {
          pMODBUS->MachineState = 0x04; 
          pMODBUS->ModBus_CMD = data;
          pMODBUS->read_receive_timer = 0;
        }
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
          pMODBUS->Read_Register_Num = pMODBUS->DataBuf[pMODBUS->BufIndex-1];
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
            u8 index = pMODBUS->DataBuf[0];
            pMODBUS->err_state = 0x00;//CRC校验正确 
            pMODBUS->read_success_num += 1;
            
            if((index>=MIN_MONITOR_DEV_ADDR)&&(index<=MAX_MONITOR_DEV_ADDR))
            {
              index -= MIN_MONITOR_DEV_ADDR;
              *((u16*)(&(st[index].real_rpm_reg))) = (pMODBUS->DataBuf[3]<<8) | pMODBUS->DataBuf[4];
              st[index].real_rpm = st[index].real_rpm_reg;
              //电机失能后,清零速度，后续和利时来改
              if(moto_enable_status[index]==0) st[index].real_rpm = 0;
              
              if(index & 0x1) st[index].real_rpm = -st[index].real_rpm;   //奇数电机速度反向
              st[index].real_mms = (s16)roundf((float)st[index].real_rpm * COFF_001RPM_TO_MMS);
              st[index].counter += 1;
            }
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
      case 0x04: //write 单个寄存器
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

void ROAD_RECORD_Task(void)
{
  static u32 record_counter[MOTO_NUM] = {0,0};
  static u32 timer_bk[MOTO_NUM] = {0,0};
  u8 i;
  u32 temp;
  for(i = 0; i < MOTO_NUM; i++)
  {
    if(record_counter[i] != MONITOR_St[i].counter)
    {
      record_counter[i] = MONITOR_St[i].counter;
      temp = NumOfSysTickInt - timer_bk[i];
      timer_bk[i] = NumOfSysTickInt;
      RoadLength[i] += (float)temp*(float)MONITOR_St[i].real_mms*0.001*COFF_DISTANCE;
    }
  }
}