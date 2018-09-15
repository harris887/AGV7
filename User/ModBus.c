#include "user_inc.h"
#include "string.h"
#include "stdlib.h"

#define HALL_COMM_ENUM            CH_HALL //CH_HALL, 4 
#define HALL_SENSOR_READ_ONCE_MS  100

//Ѳ�߹����еķֲ�
//u8 SelectDir = SELECT_DIR_LEFT;//0-��ָʾ��1-����ߣ�2-���ұ�
u16 MB_LINE_DIR_SELECT = BRANCH_TO_RIGHT; 


//�������������ն����ȫ�ֱ���
MODBUS_SAMPLE MODBUS_HallSensor = {
  .MachineState = 0,
  .read_success_num = 0,
  .write_success_num = 0,
};

u8 HallStatusFresh=0;
u8 HallValue[LINE_SENSOR_NUM];
//SENSOR_STATUS SENSOR_Status={0,0,0};
SENSOR_STATUS_NEW SENSOR_STATUS_New={
  .black_sensor_serial_flag=0,
  .Segment_Num=0,
};
//�������������Ͷ����ȫ�ֱ���
const u8 MODBUS_READ_SENSOR_DATA1[8]=
{0x01 ,0x04 ,0x00 ,0x00 ,0x00 ,0x08 ,0xF1 ,0xCC};


u16 HallSensor_Timeout = 3500;

u8 ON_LINE_Flag=0;
static u8 ON_LINE_Counter=0;
u8 MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR;


HALL_DEBUG HALL_D_Rt = {0, 0};
HALL_DEBUG HALL_D_Bk = {0, 0};
u8 GetSensorMiddleIndex_Debug(SENSOR_STATUS_NEW* st);
/*******************************************************************
��������:void Analysis_Receive_From_ModeBusSlaveDev(u8 data)
��������:���ջ��������� ����������� ��״̬����
*******************************************************************/
void Analysis_Receive_From_HallSensor(u8 data, MODBUS_SAMPLE* pMODBUS)
{
    switch(pMODBUS->MachineState)//��ʼ�� Ĭ�� Ϊ 00;
    {
    case 0x00:
      {
        if(data >= MODE_BUS_HALL_Addr)//�ӻ���ַ
        {
          pMODBUS->MachineState = 0x01;
          pMODBUS->BufIndex = 0;
          pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        }
        else
        {
          pMODBUS->MachineState = 0x0B;//����������������Ҫ�����м�����Ϊ01������Ϊ��Ҫ�ӻ���ַ��
          pMODBUS->BufIndex = 0;
        }  
      }
      break;
    case 0x01:
      {
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        if(data == CMD_ModBus_Read) 
        {
          pMODBUS->MachineState = 0x02; 
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
        //���յ������ܵ��ֽ���
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        pMODBUS->read_receive_timer++;
        if(pMODBUS->read_receive_timer == 2 )
        {
          pMODBUS->Read_Register_Num = pMODBUS->DataBuf[pMODBUS->BufIndex-2]*256 + pMODBUS->DataBuf[pMODBUS->BufIndex-1];
          if(pMODBUS->Read_Register_Num == 16) //��֧��һ�ֶ�ȡ���ݵķ�ʽ
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
        if(pMODBUS->read_receive_timer >= (pMODBUS->Read_Register_Num + 2))
        {
          u16 cal_crc;
          cal_crc=ModBus_CRC16_Calculate(pMODBUS->DataBuf,pMODBUS->Read_Register_Num+4);
              
          pMODBUS->receive_CRC_L = pMODBUS->DataBuf[pMODBUS->BufIndex-2];
          pMODBUS->receive_CRC_H = pMODBUS->DataBuf[pMODBUS->BufIndex-1];
          if(((cal_crc>>8) == pMODBUS->receive_CRC_H) && ((cal_crc&0xFF) == pMODBUS->receive_CRC_L))
          {
            pMODBUS->err_state = 0x00;//CRCУ����ȷ 
            pMODBUS->read_success_num += 1;
            
            //��ȷ��������
            memcpy(HallValue,pMODBUS->DataBuf + 4,16);
            HallStatusFresh=1;
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

/********************************************************************************
��������:u16 ModBus_CRC16_Calculate(unsigned char *aStr , unsigned char alen)
��������:���㷢������ CRC У�鹦��
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



void MODBUS_READ_HALL_SERSOR_TASK(void)
{
  static u8 modebus_hall_tx_pro=0;
  static u16 ms_waste;
  switch(modebus_hall_tx_pro)
  {
  case 0:
    {
      if(HallSensor_Timeout == 0)
      {
        HALL_RS485_TX_ACTIVE();
        HallSensor_Timeout = 2;
        ms_waste = 2;
        modebus_hall_tx_pro++;
      }
    }
    break;
  case 1:
    {
      //���Ͷ�ȡsensor��ָ��
      if(HallSensor_Timeout == 0)
      {
        u8 temp_buf[16];
        u16 cal_crc;
        u16 bits = sizeof(MODBUS_READ_SENSOR_DATA1) * 10;

        memcpy(temp_buf,MODBUS_READ_SENSOR_DATA1,sizeof(MODBUS_READ_SENSOR_DATA1));
        temp_buf[0]=MODE_BUS_HALL_Addr;
        cal_crc=ModBus_CRC16_Calculate(temp_buf,sizeof(MODBUS_READ_SENSOR_DATA1)-2);
        temp_buf[6]=cal_crc&0xFF;
        temp_buf[7]=cal_crc>>8;   
        
        FillUartTxBuf_NEx((u8*)temp_buf, sizeof(MODBUS_READ_SENSOR_DATA1), HALL_COMM_ENUM);
        HallSensor_Timeout = (bits / 19) + 3; // 2
        ms_waste += (bits / 19) + 3;
        modebus_hall_tx_pro++;
      }
    }
    break;
  case 2:
    if(HallSensor_Timeout == 0)
    {
      HALL_RS485_RX_ACTIVE();
      HallSensor_Timeout = HALL_SENSOR_READ_ONCE_MS - ms_waste;
      modebus_hall_tx_pro = 0;
    }
    break;
  default:  
    {
      modebus_hall_tx_pro=0; 
    }
  }
  
  //���������
  if(HallStatusFresh)
  {
    HallStatusFresh=0;
    CheckHallOnListNumNew(HallValue,LINE_SENSOR_NUM,&SENSOR_STATUS_New);
#if (1)
    HALL_D_Rt.sensor_buf[HALL_D_Rt.index++] = GetSensorMiddleIndex_Debug(&SENSOR_STATUS_New);  
#endif
    
    if(SENSOR_STATUS_New.black_sensor_serial_flag==0)
    {
      //SetBeep(1,200,100);//��������
    }
    if(ON_LINE_Flag)
    {
      if(SENSOR_STATUS_New.Segment_Num==0)
      {
        ON_LINE_Counter+=1;
        if(ON_LINE_Counter>=5) // 3 -> 5 
        {
          ON_LINE_Flag=0;
          ON_LINE_Counter=0;
#if (1)
          memcpy((void*)&HALL_D_Bk, (void*)&HALL_D_Rt, sizeof(HALL_DEBUG));       
#endif
          if(LOG_Level <= LEVEL_INFO) printf("Off LINE!\n");
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
          
          if(LOG_Level <= LEVEL_INFO) printf("On LINE!\n");
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
  u8 i, num = 0;
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
        //�߶γ����˲�
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
  //���һ���о�
  {
      if(start_flag!=0)
      {
        start_flag=0;
        St->seg_list[seg_index].tail_index=i;
        //�߶γ����˲�
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
  St->Segment_Num=seg_index;
  return num;
}


/*�������ֲַ�ļ�����*/
u8 GetSensorMiddleIndex(SENSOR_STATUS_NEW* st)
{
  static u8 last_index = WONDER_MID_SENSOR_INDEX; //
  u8 temp = WONDER_MID_SENSOR_INDEX;
  if(st->Segment_Num == 0)
  {
    temp = last_index;
  }
  else if(st->Segment_Num == 1)
  {
    temp = st->seg_list[0].middle_index;
    last_index = temp;
  }
  else if(st->Segment_Num >= 2)
  {
    if(MB_LINE_DIR_SELECT == BRANCH_TO_LEFT) //left
    {
      temp=st->seg_list[1].middle_index;
    }
    else // right
    {
      temp=st->seg_list[0].middle_index;
    }
    last_index = temp;
  }
  return temp;
}

u8 GetSensorMiddleIndex_Debug(SENSOR_STATUS_NEW* st)
{
  static u8 last_index = WONDER_MID_SENSOR_INDEX; //
  u8 temp = WONDER_MID_SENSOR_INDEX;
  if(st->Segment_Num == 0)
  {
    temp = last_index;
  }
  else if(st->Segment_Num == 1)
  {
    temp = st->seg_list[0].middle_index;
    last_index = temp;
  }
  else if(st->Segment_Num >= 2)
  {
    if(MB_LINE_DIR_SELECT == BRANCH_TO_LEFT) //left
    {
      temp=st->seg_list[1].middle_index;
    }
    else // right
    {
      temp=st->seg_list[0].middle_index;
    }
    last_index = temp;
  }
  return temp;
}

void HALL_DEBUG_Print(void)
{
  u16 i;
  u8 index = HALL_D_Bk.index;
  printf("------------------\n");
  for(i = 0; i < 256; i++)
  {
    if((i & 15) == 0) printf("\n");
    printf("%d ", HALL_D_Bk.sensor_buf[index++]);
  }
  printf("\n------------------\n");
}