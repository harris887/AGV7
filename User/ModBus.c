#include "user_inc.h"
#include "string.h"
#include "stdlib.h"

#define HALL_SENSOR_PRINTF_DEBUG  0
#define HALL_COMM_ENUM            CH_HALL   

//本设备的寄存器
#define MOD_BUS_REG_START_ADDR  0x0000

//巡线过程中的分叉
u8 SelectDir=SELECT_DIR_LEFT;//0-无指示，1-走左边，2-走右边
u16 MB_LINE_DIR_SELECT=0;

//crc校验失败次数，调试使用
u32 crc_error_num=0;

//霍尔传感器接收端相关全局变量
struct  MODBUS  HallSensor_Modbus;
u8 HallSensorMachineState = 0;
u8 HallStatusFresh=0;
u8 HallValue[LINE_SENSOR_NUM];
//SENSOR_STATUS SENSOR_Status={0,0,0};
SENSOR_STATUS_NEW SENSOR_STATUS_New={
  //.black_sensor_num=0,
  .black_sensor_serial_flag=0,
  .Segment_Num=0,
};
//霍尔传感器发送端相关全局变量
const u8 MODBUS_READ_SENSOR_DATA1[8]=
{0x01 ,0x04 ,0x00 ,0x00 ,0x00 ,0x08 ,0xF1 ,0xCC};
#define MODBUS_TIME_OUT_MS  60
u8 modebus_hall_tx_pro=0;
u16 modebus_timeout=0;
u16 Modebus_tx_rx_change_delay=0;
u8 Modebus_read_cmd_tx_finish=0;
u8 ON_LINE_Flag=0;
static u8 ON_LINE_Counter=0;
u8 MODE_BUS_HALL_Addr=DEFAULT_MODE_BUS_HALL_ADDR;

/*******************************************************************
函数名称:void Analysis_Receive_From_ModeBusSlaveDev(u8 data)
函数功能:接收霍尔传感器 命令解析函数 （状态机）
*******************************************************************/
void Analysis_Receive_From_ModeBusSlaveDev(u8 data)
{
    static u8 index = 0;
    static u8 Receive_Data_From_HallSensor[256];
    static u8 read_receive_timer = 0;
    static u8 receive_CRC_H = 0;
    static u8 receive_CRC_L = 0;  
    switch(HallSensorMachineState)//初始化 默认 为 00;
    {
        case 0x00: 
        {
            if(data == MODE_BUS_HALL_Addr)//从机地址
            {
                HallSensorMachineState = 0x01;//从机地址可变，通过A8更改。
                index = 0;
                HallSensor_Modbus.Probe_Slave_Add = data;
                Receive_Data_From_HallSensor[index++] = data;
            }
            else
            {
                HallSensorMachineState = 0x0B;//缓冲数据区域清零要处理，中间数据为01，误认为是要从机地址。
                index = 0;
            }  
        }break;
	case 0x01:
        {	 
            Receive_Data_From_HallSensor[index++] = data;
            if(data == CMD_ModBus_Read) //执行读取单个或多个寄存器命令  0x04 
            {
                HallSensorMachineState = 0x02; 
                HallSensor_Modbus.ModBus_CMD = CMD_ModBus_Read;
                read_receive_timer = 0;
            }
            else
            { 
                HallSensorMachineState = 0x00;
            }
        }break;
	case 0x02: 
        {    
            //接收到读功能的字节数
            Receive_Data_From_HallSensor[index++] = data;
            read_receive_timer++;
            if( read_receive_timer == 2 )
            {
                
                HallSensor_Modbus.Read_Register_Num = Receive_Data_From_HallSensor[index-2]*256 + Receive_Data_From_HallSensor[index-1];
                read_receive_timer = 0;
                if(HallSensor_Modbus.Read_Register_Num==16)//仅支持一种读取数据的方式
                  HallSensorMachineState = 0x03;
                else
                  HallSensorMachineState = 0x00;
            } 
        }break;
	case 0x03: 
        {   
            Receive_Data_From_HallSensor[index++] = data;
            read_receive_timer++;
            if(read_receive_timer >= (HallSensor_Modbus.Read_Register_Num+2))
            {
              u16 cal_crc;
              cal_crc=ModBus_CRC16_Calculate(Receive_Data_From_HallSensor,HallSensor_Modbus.Read_Register_Num+4);
              
              receive_CRC_L = Receive_Data_From_HallSensor[index-2];
              receive_CRC_H = Receive_Data_From_HallSensor[index-1];
              if(((cal_crc>>8) == receive_CRC_H) 
                  && ((cal_crc&0xFF) == receive_CRC_L))
              {
                  HallSensor_Modbus.err_state = 0x00;//CRC校验正确 
                  //正确读到数据
                  memcpy(HallValue,&Receive_Data_From_HallSensor[4],16);
                  HallStatusFresh=1;
              }    
              else	  
              {
                  HallSensor_Modbus.err_state = 0x04;
                  crc_error_num+=1;
              }   
              index = 0;  
              read_receive_timer = 0;  
              HallSensorMachineState = 0x00;                
            }
        }
        break;
      default:
        {
          HallSensorMachineState=0;
        }
    }
}


/********************************************************************************
函数名称:u16 ModBus_CRC16_Calculate(unsigned char *aStr , unsigned char alen)
函数功能:计算发送数据 CRC 校验功能
********************************************************************************/
u16 ModBus_CRC16_Calculate(u8 *aStr , u8 alen)
{
  u16 xda,xdapoly;
  u8 i,j,xdabit;
  xda = 0xFFFF;
  xdapoly = 0xA001;	// (X**16 + X**15 + X**2 + 1)
  for(i=0;i<alen;i++) 
  {
    xda ^= aStr[i];
    for(j=0;j<8;j++)
    {
      xdabit = (u8)(xda & 0x01);
      xda >>= 1;
      if( xdabit ) xda ^= xdapoly;
    }
  }    
  return xda;
}





void MODBUS_READ_SERSOR_BOARD_TASK(void)
{
  switch(modebus_hall_tx_pro)
  {
  case 0:
    {
      HALL_RS485_TX_ACTIVE();
      modebus_timeout=MODBUS_TIME_OUT_MS;
      modebus_hall_tx_pro++;
    }
    break;
  case 1:
    {
      //发送读取sensor的指令
      if(modebus_timeout<(MODBUS_TIME_OUT_MS-2))
      {
        u8 temp_buf[16];
        u16 cal_crc;
        Modebus_read_cmd_tx_finish=0;
        memcpy(temp_buf,MODBUS_READ_SENSOR_DATA1,sizeof(MODBUS_READ_SENSOR_DATA1));
        temp_buf[0]=MODE_BUS_HALL_Addr;
        cal_crc=ModBus_CRC16_Calculate(temp_buf,sizeof(MODBUS_READ_SENSOR_DATA1)-2);
        temp_buf[6]=cal_crc&0xFF;
        temp_buf[7]=cal_crc>>8;   
        
        //FillUartTxBufN((u8*)temp_buf,sizeof(MODBUS_READ_SENSOR_DATA1),HALL_COMM_ENUM);
        FillUartTxBuf_NEx((u8*)temp_buf, sizeof(MODBUS_READ_SENSOR_DATA1), HALL_COMM_ENUM);
        modebus_hall_tx_pro++;
      }
    }
    break;
  case 2:
    if(modebus_timeout!=0)
    {
      if(Modebus_read_cmd_tx_finish!=0)
      {
        Modebus_tx_rx_change_delay=3;//for 9600bps
        modebus_hall_tx_pro++;
      }
    }
    else
    {
      modebus_hall_tx_pro=0;
    }
    break;
  case 3:
    if(modebus_timeout!=0)
    {
      if(Modebus_tx_rx_change_delay==0)
      {
        HALL_RS485_RX_ACTIVE();
        modebus_hall_tx_pro++;
      }
    }
    else
    {
      modebus_hall_tx_pro=0;
    }    
    break;
  case 4:
    if(modebus_timeout!=0)
    {
      //这里应该收到数据，什么也不做
    }
    else
    {
      modebus_hall_tx_pro=0;
    }
    break;
  default:  
    {
      modebus_hall_tx_pro=0; 
    }
  }
  
  //检测结果归纳
  if(HallStatusFresh)
  {
    HallStatusFresh=0;
    CheckHallOnListNumNew(HallValue,LINE_SENSOR_NUM,&SENSOR_STATUS_New);
    
    if(SENSOR_STATUS_New.black_sensor_serial_flag==0)
    {
      //SetBeep(1,200,100);//发出警告
    }
    if(ON_LINE_Flag)
    {
      if(SENSOR_STATUS_New.Segment_Num==0)
      {
        ON_LINE_Counter+=1;
        if(ON_LINE_Counter>=3)
        {
          ON_LINE_Flag=0;
          ON_LINE_Counter=0;
#if (HALL_SENSOR_PRINTF_DEBUG)          
          printf("Off LINE!\n");
#endif
        }
      }
      else
      {
        ON_LINE_Counter=0;
      }
    }
    else
    {
      if(SENSOR_STATUS_New.Segment_Num!=0)
      {
        ON_LINE_Counter+=1;
        if(ON_LINE_Counter>=10)
        {
          ON_LINE_Flag=1;
          ON_LINE_Counter=0;
          
#if (HALL_SENSOR_PRINTF_DEBUG)
          printf("On LINE!\n");
#endif
        }
      }
      else
      {
        ON_LINE_Counter=0;
      }
    }
  }
}


u8 CheckHallOnListNumNew(u8* hall_list,u8 total_num,SENSOR_STATUS_NEW* St)
{
  u8 i,num;
  u8 start_flag=0;
  u8 seg_index=0;
  for(i=0;i<total_num;i++)
  {
    if(hall_list[i]!=0)
    {
      num++;
      if(start_flag==0) 
      {
        start_flag=1;
        St->seg_list[seg_index].head_index=(i+1);
      }
    }
    else
    {
      if(start_flag!=0)
      {
        start_flag=0;
        St->seg_list[seg_index].tail_index=i;
        //线段长度滤波
        if((St->seg_list[seg_index].tail_index-St->seg_list[seg_index].head_index+1)>=2)
        {
          St->seg_list[seg_index].middle_index=
            St->seg_list[seg_index].tail_index+St->seg_list[seg_index].head_index;
          seg_index+=1;
          
          if(seg_index>=MAX_SEGMENT_NUM) break;
        }
      }
    }
  }
  //最后一次判决
  {
      if(start_flag!=0)
      {
        start_flag=0;
        St->seg_list[seg_index].tail_index=i;
        //线段长度滤波
        if((St->seg_list[seg_index].tail_index-St->seg_list[seg_index].head_index+1)>=2)
        {
          St->seg_list[seg_index].middle_index=
            St->seg_list[seg_index].tail_index+St->seg_list[seg_index].head_index;
          seg_index+=1;
          
          //if(seg_index>=MAX_SEGMENT_NUM) break;
        }
      }  
  }
  
  if(seg_index==1) St->black_sensor_serial_flag=1;
  else St->black_sensor_serial_flag=0;
  //St->black_sensor_num=num;
  St->Segment_Num=seg_index;
  return num;
}

#if 0
u8 CheckHallOnListNum(u8* hall_list,u8 total_num,SENSOR_STATUS* St)
{
  u8 i,num;
  u8 last_black_sensor=0;
  u8 serial_num=0;
  num=0;
  for(i=0;i<total_num;i++)
  {
    if(hall_list[i]!=0) 
    {
      St->black_sensor_index_list[num]=i+1;
      if(serial_num==0) 
      {
        last_black_sensor=i+1;
        serial_num++;
      }
      else 
      {
        if(last_black_sensor==i) serial_num++;
        last_black_sensor=i+1;
      }
      num++;
    }
  }
  if(num==0) 
  {
    St->black_sensor_serial_flag=0;
  }
  else if(num==serial_num) 
  {
    St->black_sensor_serial_flag=1;
    St->Middle_Index=St->black_sensor_index_list[0]+St->black_sensor_index_list[num-1];
  }
  else
  {
    St->black_sensor_serial_flag=0;
  }
  St->black_sensor_num=num;
  return num;
}
#endif



#define LINE_WIDTH_FILTER_LENGTH  64
#define DEFAULT_LINE_WIDTH  4
u32 LineWidthBuf=64*DEFAULT_LINE_WIDTH;
/*磁条出现分叉的检测策略*/
u8 GetSensorMiddleIndex(SENSOR_STATUS_NEW* st)
{
  static u8 LastMiddleIndex=WONDER_MID_SENSOR_INDEX;
  if(st->Segment_Num==0)
  {
    return WONDER_MID_SENSOR_INDEX;
  }
  else if(st->Segment_Num==1)
  {
    //u8 line_width=st->seg_list[0].tail_index-st->seg_list[0].head_index+1;
    //u8 line_width_filter;
    //LineWidthBuf-=(LineWidthBuf/LINE_WIDTH_FILTER_LENGTH);
    //LineWidthBuf+=line_width;
    //line_width_filter=(LineWidthBuf/LINE_WIDTH_FILTER_LENGTH);
    
    
    //分叉过后，清除方向选择标志
    if(SelectDir&SELECT_DIR_START_FLAG)
    {
      SelectDir+=SELECT_DIR_SINGLE_LINE_TIMES_UP_STEP;
      if((SelectDir&SELECT_DIR_SINGLE_LINE_TIMES_MASK)>=20)
      {
        //SelectDir=SELECT_DIR_NULL;
        //SelectDir=MB_LINE_DIR_SELECT?SELECT_DIR_LEFT:SELECT_DIR_RIGHT;
        SelectDir=MB_LINE_DIR_SELECT?SELECT_DIR_RIGHT:SELECT_DIR_LEFT;
      }
    }
    //if(line_width<(line_width_filter+3))
    //{
      LastMiddleIndex=st->seg_list[0].middle_index;
      return st->seg_list[0].middle_index;
    //}
    //else
    //{
    //  return LastMiddleIndex;
    //}
  }
  else if(st->Segment_Num>=2)
  {
    u8 temp;
    switch(SelectDir&SELECT_DIR_MASK)
    {
    case SELECT_DIR_NULL:
      {
        if(abs(((s32)LastMiddleIndex)-((s32)st->seg_list[0].middle_index))
            >= abs(((s32)LastMiddleIndex)-((s32)st->seg_list[1].middle_index)))
        {
          temp=st->seg_list[1].middle_index;
        }
        else
        {
          temp=st->seg_list[0].middle_index;
        }
      }
      break;
    case SELECT_DIR_LEFT:
      {
        if((SelectDir&SELECT_DIR_START_FLAG)==0) SelectDir|=SELECT_DIR_START_FLAG;
        temp=st->seg_list[1].middle_index;
      }
      break;
    case SELECT_DIR_RIGHT:
      {
        if((SelectDir&SELECT_DIR_START_FLAG)==0) SelectDir|=SELECT_DIR_START_FLAG;
        temp=st->seg_list[0].middle_index;
      }
      break;
    }
    LastMiddleIndex=temp;
    return temp;
  }
  return WONDER_MID_SENSOR_INDEX;
}

