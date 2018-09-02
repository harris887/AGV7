#include "user_inc.h"
#include "string.h"

#define DIDO_UART_PORT                    CH_DIDO
#define DIDO_MODULE_BORCAST_ADDR          254
#define DEFAULT_DIDO_COMM_TIME_OUT        12
#define DEFAULT_DIDO_READ_LIGHT_TIME_OUT  100
#define DEFAULT_DIDO_ENABLE_TIME_OUT      1000


u16 DIDO_COMM_Timeout = DEFAULT_DIDO_ENABLE_TIME_OUT;
u16 DIDO_READ_LIGHT_Timeout = DEFAULT_DIDO_READ_LIGHT_TIME_OUT;

u8 DIDO_RelayStatus=0;
u8 DIDO_RelayRefresh=0xFF;//0xFF

DIDO_INPUT_STATUS DIDO_INPUT_Status = {
  .RelayStatus=0,
  .LightStatus=0,
};

MODBUS_SAMPLE MODBUS_Dido = {
  .MachineState=0,
  .read_success_num=0,
};

const u8 DEFAULT_DIDO_RELAY_SET[8]={0xFE,CMD_ModBus_Wire_Write,0x00,0x00,0xFF,0x00,0x98,0x35};
const u8 DEFAULT_DIDO_RELAY_GET[8]={0xFE,CMD_ModBus_Wire_Read ,0x00,0x00,0x00,0x08,0x29,0xC3};
const u8 DEFAULT_DIDO_LIGHT_GET[8]={0xFE,CMD_ModBus_Wire_ReadEx ,0x00,0x00,0x00,0x08,0x6d,0xC3};

void Analysis_Receive_From_Dido(u8 data,MODBUS_SAMPLE* pMODBUS, DIDO_INPUT_STATUS* st)
{
    switch(pMODBUS->MachineState)//初始化 默认 为 00;
    {
    case 0:
      {
        if(data == DIDO_MODULE_BORCAST_ADDR)//从机地址
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
        if((data == CMD_ModBus_Wire_Read) || (data == CMD_ModBus_Wire_ReadEx)) //执行读取单个或多个寄存器命令  0x01 
        {
          pMODBUS->MachineState = 0x02; 
          pMODBUS->ModBus_CMD = data;
          pMODBUS->read_receive_timer = 0;
        }
        else if((data == CMD_ModBus_Wire_Write) || (data == CMD_ModBus_Wire_WriteMore))
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
            pMODBUS->err_state = 0x00;//CRC校验正确 
            pMODBUS->read_success_num += 1;

            st->RefreshFlag = 1;
            if(pMODBUS->ModBus_CMD == CMD_ModBus_Wire_Read)
            {
              st->RelayStatus = pMODBUS->DataBuf[3];
            }
            else
            {
              st->LightStatus  = pMODBUS->DataBuf[3];
              BUTTON_IM_STOP_Flag &= ~3;
              BUTTON_IM_STOP_Flag |= (st->LightStatus & ((1 << DI_IM_STOP_1) | (1 << DI_IM_STOP_2))) >> DI_IM_STOP_1;
              TOUCH_SENSOR_Flag = (st->LightStatus & ((1 << DI_TOUCH_HEAD) | (1 << DI_TOUCH_TAIL))) >> DI_TOUCH_TAIL; 
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

void Check_DIDO_TASK(void)
{
  // 通讯
  if(DIDO_COMM_Timeout==0)
  {
    /**/
    //输入光耦查询
    //if(0)
    if(DIDO_READ_LIGHT_Timeout==0)
    {
      DIDO_COMM_Timeout = DEFAULT_DIDO_COMM_TIME_OUT;
      DIDO_READ_LIGHT_Timeout = DEFAULT_DIDO_READ_LIGHT_TIME_OUT;
      FillUartTxBuf_NEx((u8*)DEFAULT_DIDO_LIGHT_GET,sizeof(DEFAULT_DIDO_LIGHT_GET),DIDO_UART_PORT);
      return;
    }
    
    //数字输出
    if(DIDO_RelayRefresh)
    {
      u8 i;
      for(i=0;i<8;i++)
      {
        if(DIDO_RelayRefresh&(1<<i))
        {
          u8 t_buf[16];
          u16 cal_crc;
          
          DIDO_COMM_Timeout = DEFAULT_DIDO_COMM_TIME_OUT;
          
          DIDO_RelayRefresh &= ~(1<<i);
          memcpy(t_buf,DEFAULT_DIDO_RELAY_SET,sizeof(DEFAULT_DIDO_RELAY_SET));
          t_buf[3] = i;
          if(DIDO_RelayStatus&(1<<i)) t_buf[4] = 0xFF;
          else t_buf[4] = 0x00;
          cal_crc = ModBus_CRC16_Calculate(t_buf,sizeof(DEFAULT_DIDO_RELAY_SET)-2);
          t_buf[sizeof(DEFAULT_DIDO_RELAY_SET)-2] = cal_crc&0xFF;
          t_buf[sizeof(DEFAULT_DIDO_RELAY_SET)-1] = cal_crc>>8;
          FillUartTxBuf_NEx(t_buf, sizeof(DEFAULT_DIDO_RELAY_SET), DIDO_UART_PORT);
          break;
        }
      }
    }
  }
  
  /*
  //---- 输入打印测试 ----
  if(1)
  {
    static u8 st=0;
    u8 i;
    if(st != DIDO_INPUT_Status.LightStatus)
    {
      for(i=0;i<8;i++)
      {
        if((st&(1<<i))!=(DIDO_INPUT_Status.LightStatus&(1<<i)))
        {
          printf("In_%d = %d\n",i,(st&(1<<i))?0:1);
        }
      }
      st = DIDO_INPUT_Status.LightStatus;
    }
  }
  */
}


void SET_DIDO_Relay(u8 index,u8 status)
{
  if(index < DO_Num)
  {
    DIDO_RelayRefresh |= (1 << index);
    if(status)  DIDO_RelayStatus |= (1 << index);
    else DIDO_RelayStatus &= ~(1 << index);
  }
}