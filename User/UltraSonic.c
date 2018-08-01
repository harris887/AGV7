#include "user_inc.h"
#include <string.h>

#define US_COMM_ENUM      2   //USART2

u32 us_time=0;
u32 us_suc_time=0;

u16 UltraSonicCheckTimer=DEFAULT_ULTRA_SONIC_CHECK_TIME_IN_MS;
u16 UltraSonicDistance[ULTRA_SONIC_SENSOR_NUM]=
{
  DEFAULT_ULTRA_SONIC_DISTANCE,DEFAULT_ULTRA_SONIC_DISTANCE,
  DEFAULT_ULTRA_SONIC_DISTANCE,DEFAULT_ULTRA_SONIC_DISTANCE
};
u16 US_Timeout=DEFAULT_ULTRA_SONIC_NO_DATA_TIME_OUT;

u8 UltraSonicMachineState = 0;
struct  MODBUS  UltraSonic_Modbus;
const u8 MODBUS_READ_ULTRA_SONIC[8]={0x01 ,0x04 ,0x00 ,0x05 ,0x00 ,0x04 ,0xE1 ,0xC8};

void Analysis_Receive_From_UltraSonic(u8 data)
{
    static u8 index = 0;
    static u8 Receive_Data_From_UltraSonic[256];
    static u8 read_receive_timer = 0;
    static u8 receive_CRC_H = 0;
    static u8 receive_CRC_L = 0;  
    switch(UltraSonicMachineState)//初始化 默认 为 00;
    {
        case 0x00: 
        {
            if(data == DEFAULT_MODE_BUS_ULTRA_SONIC_ADDR)//从机地址
            {
                UltraSonicMachineState = 0x01;//从机地址可变，通过A8更改。
                index = 0;
                UltraSonic_Modbus.Probe_Slave_Add = data;
                Receive_Data_From_UltraSonic[index++] = data;
            }
            else
            {
                UltraSonicMachineState = 0x0B;//缓冲数据区域清零要处理，中间数据为01，误认为是要从机地址。
                index = 0;
            }  
        }break;
	case 0x01:
        {	 
            Receive_Data_From_UltraSonic[index++] = data;
            if(data == CMD_ModBus_Read) //执行读取单个或多个寄存器命令  0x04 
            {
                UltraSonicMachineState = 0x02; 
                UltraSonic_Modbus.ModBus_CMD = CMD_ModBus_Read;
                read_receive_timer = 0;
            }
            else if((data == CMD_ModBus_Write) || (data == CMD_ModBus_WriteMore))
            {
                UltraSonicMachineState = 0x04; 
                UltraSonic_Modbus.ModBus_CMD = data;
                read_receive_timer = 0;
            }
            else
            { 
                UltraSonicMachineState = 0x0B;
            }
        }break;
	case 0x02: //read part 00
        {    
            //接收到读功能的字节数
            Receive_Data_From_UltraSonic[index++] = data;
            read_receive_timer++;
            if( read_receive_timer == 2 )
            {
                UltraSonic_Modbus.Read_Register_Num 
                  = Receive_Data_From_UltraSonic[index-2]*256 + Receive_Data_From_UltraSonic[index-1];
                if(UltraSonic_Modbus.Read_Register_Num<=8)//长度限定
                {
                  UltraSonicMachineState = 0x03;
                }
                else
                {
                  UltraSonicMachineState = 0x00;
                }
                read_receive_timer = 0;
            } 
        }break;
	case 0x03: //read part 01
        {   
            Receive_Data_From_UltraSonic[index++] = data;
            read_receive_timer++;
            if(read_receive_timer >= (UltraSonic_Modbus.Read_Register_Num+2))
            {
              u16 cal_crc;
              cal_crc=ModBus_CRC16_Calculate(Receive_Data_From_UltraSonic,
                                             UltraSonic_Modbus.Read_Register_Num+4);
              
              receive_CRC_L = Receive_Data_From_UltraSonic[index-2];
              receive_CRC_H = Receive_Data_From_UltraSonic[index-1];
              if(((cal_crc>>8) == receive_CRC_H) 
                  && ((cal_crc&0xFF) == receive_CRC_L))
              {
                  UltraSonic_Modbus.err_state = 0x00;//CRC校验正确 
                  //正确读到数据
                  UltraSonicDistance[0]=(Receive_Data_From_UltraSonic[4]<<8)|Receive_Data_From_UltraSonic[5];
                  UltraSonicDistance[1]=(Receive_Data_From_UltraSonic[6]<<8)|Receive_Data_From_UltraSonic[7];
                  UltraSonicDistance[2]=(Receive_Data_From_UltraSonic[8]<<8)|Receive_Data_From_UltraSonic[9];
                  UltraSonicDistance[3]=(Receive_Data_From_UltraSonic[10]<<8)|Receive_Data_From_UltraSonic[11];
                  US_Timeout=DEFAULT_ULTRA_SONIC_NO_DATA_TIME_OUT; 
                  us_suc_time += 1;
              }    
              else	  
              {
                  //AckModBusCrcError(UltraSonic_Modbus.ModBus_CMD);
                  UltraSonic_Modbus.err_state = 0x04;
                  //crc_error_num+=1;
              }   
              index = 0;  
              read_receive_timer = 0;  
              UltraSonicMachineState = 0x00;                
            }
        }
        break;
      case 0x04: //write
        {
            Receive_Data_From_UltraSonic[index++] = data;
            read_receive_timer++;
            if( read_receive_timer == 3 )
            {
              u16 cal_crc;
              cal_crc=ModBus_CRC16_Calculate(Receive_Data_From_UltraSonic,3);
              
              receive_CRC_L = Receive_Data_From_UltraSonic[index-2];
              receive_CRC_H = Receive_Data_From_UltraSonic[index-1];
              if(((cal_crc>>8) == receive_CRC_H) 
                  && ((cal_crc&0xFF) == receive_CRC_L))
              {
                //AckWriteCmdToMaster(UltraSonic_Modbus.ModBus_CMD,
                //                    Receive_Data_From_UltraSonic[2]);
              }    
              else	  
              {
                //AckModBusCrcError(UltraSonic_Modbus.ModBus_CMD);
                UltraSonic_Modbus.err_state = 0x04;
                //crc_error_num+=1;
              }   
              index = 0;  
              read_receive_timer = 0;  
              UltraSonicMachineState = 0x00;   
            }
        }break;
      case 0xb:break;  
      default:
        {
          UltraSonicMachineState=0;
        }
    }
}

void Check_UltraSonic_TASK(void)
{
  static u16 status_time[4]={0,0,0,0};
  if(UltraSonicCheckTimer==0)
  {
    UltraSonicCheckTimer = DEFAULT_ULTRA_SONIC_CHECK_TIME_IN_MS;
    FillUartTxBufN((u8*)MODBUS_READ_ULTRA_SONIC,sizeof(MODBUS_READ_ULTRA_SONIC),US_COMM_ENUM);
    us_time+=1;
    if(US_Timeout==0)
    {
      UltraSonicDistance[0]=DEFAULT_ULTRA_SONIC_DISTANCE;   
      UltraSonicDistance[1]=DEFAULT_ULTRA_SONIC_DISTANCE;   
      UltraSonicDistance[2]=DEFAULT_ULTRA_SONIC_DISTANCE;   
      UltraSonicDistance[3]=DEFAULT_ULTRA_SONIC_DISTANCE; 
    }
    
    if(1)
    {
      u8 i;
      for(i=0;i<ULTRA_SONIC_SENSOR_NUM;i++)
      {
        if(UltraSonicDistance[i] <= DEFAULT_ULTRA_SONIC_CLOSE_DISTANCE)
        {
          if(status_time[i]<=3) // 超声滤波，防止误动作
          {
            status_time[i]+=1;
          }
          else
          {
            TOUCH_SENSOR_Flag |= (1<<(i+2));
          }
          //M_LightSensorStatus[i+2]=1;
        }
        else
        {
          status_time[i] = 0;
          TOUCH_SENSOR_Flag &= ~(1<<(i+2));
          //M_LightSensorStatus[i+2]=0;
        }
      }
    }
  }
}