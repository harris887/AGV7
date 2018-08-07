#include "user_inc.h"
#include "stdlib.h"



//���豸�ļĴ���
#define MOD_BUS_REG_START_ADDR  0x0000
#define DEFAULT_MODE_BUS_AGV_ADDR   0x0001
#define MOD_BUS_REG_NUM         ((sizeof(MOD_BUS_REG)>>1)-2)//0x000C

struct  MODBUS  A8_Modbus;

u8 machine_state = 0;
static u8 index = 0;
static u8 Receive_Data_From_A8[256];
u8 Send_Data_A8_array[256];
static u8 read_receive_timer = 0;
static u8 write_one_receive_timer = 0;
static u8 write_more_receive_timer = 0;

static u8 receive_CRC_H = 0;
static u8 receive_CRC_L = 0;
//u8 calculate_CRC_H = 0;
//u8 calculate_CRC_L = 0;

u8 Potentiometer_Position_aready_read_flag=0;

MOD_BUS_REG MOD_BUS_Reg;
MOD_BUS_REG MOD_BUS_Reg_Backup;
u8 MOD_BUS_REG_FreshFlag=0;
#define MOD_BUS_BD_LIST_LENGTH  9
const u32 MOD_BUS_BD_LIST[MOD_BUS_BD_LIST_LENGTH]=
{
  115200,//DEFAULT
  1200,2400,4800,9600,
  19200,38400,57600,115200,
}; 
const MOD_BUS_REG DEFAULT_MOD_BUS_Reg=
{
  M_CONTROL_MODE_FOLLOW_LINE,
  8,
  DEFAULT_MODE_BUS_AGV_ADDR,
  1,//�Զ�Ѳ��ʹ��
  //--------------------------------
  AUTO_FOLLOW_SPEED_CONTROL_MODE_ANALOG,//0-�����λ��,1-MODBUS�Ĵ���
  50,//30%
  DEFAULT_RFID_WAIT_TIME_IN_MS,
  DEFAULT_RFID_ONLINE_TIME_IN_MS,
  //--------------------------------
  0,

  MAGIC_WORD,
};

////////////////////////////////
u16 M_Status=M_STATUS_STOP;
u16 M_BAT_Precent=100;//�����ٷֱ�
u16 M_LightSensorStatus[6]={0,0,0,0,0,0};
U_M_CONTROL_OPTION U_M_CONTROL_Op={M_CMD_STOP,0,0,0,0};


/*******************************************************************
��������:void Analysis_Receive_From_A8(u8 data)
��������:������λ�� ����������� ��״̬����
*******************************************************************/
void Analysis_Receive_From_A8(u8 data)
{
    switch(machine_state)//��ʼ�� Ĭ�� Ϊ 00;
    {
        case 0x00: 
        {
            if(data == MOD_BUS_Reg.SLAVE_ADDR)//�ӻ���ַ
            {
                machine_state = 0x01;//�ӻ���ַ�ɱ䣬ͨ��A8���ġ�
                index = 0;
                A8_Modbus.Probe_Slave_Add = data;
                Receive_Data_From_A8[index++] = data;
            }
            else
            {
                machine_state = 0x0B;//����������������Ҫ�����м�����Ϊ01������Ϊ��Ҫ�ӻ���ַ��
                index = 0;
            }  
        }break;
	case 0x01:
        {	 
            Receive_Data_From_A8[index++] = data;
            if(data == CMD_ModBus_Read) //ִ�ж�ȡ���������Ĵ�������  0x04 
            {
                machine_state = 0x02; 
                A8_Modbus.ModBus_CMD = CMD_ModBus_Read;
                read_receive_timer = 0;
            }
            else if(data == CMD_ModBus_Write)//ִ��д�����Ĵ�������   0x06
            {
                machine_state = 0x03;
                A8_Modbus.ModBus_CMD = CMD_ModBus_Write;
                write_one_receive_timer = 0;
            }
            else if(data == CMD_ModBus_WriteMore)//ִ��д����Ĵ�������    0x16
            {
                machine_state = 0x04;
                A8_Modbus.ModBus_CMD = CMD_ModBus_WriteMore;
                write_more_receive_timer = 0;
            }    
            else
            { 
                machine_state = 0x0A;
                A8_Modbus.ModBus_CMD = data;
                A8_Modbus.err_state = 0x01;  // ���������
            }
        }break;
	case 0x02: 
        {    
            //���յ������ܵĵ�ַ���ֽ���
            Receive_Data_From_A8[index++] = data;
            read_receive_timer++;
            if( read_receive_timer == 4 )
            {
                machine_state = 0x06;
                A8_Modbus.Read_Register_Add = Receive_Data_From_A8[index-4]*256 + Receive_Data_From_A8[index-3];
                A8_Modbus.Read_Register_Num = Receive_Data_From_A8[index-2]*256 + Receive_Data_From_A8[index-1];
                read_receive_timer = 0;
            } 
        }break;
	case 0x03: 
        {   //���յ�д���ܵ�ַ������(�����ֽ�)
            Receive_Data_From_A8[index++] = data;
            write_one_receive_timer++;
            if( write_one_receive_timer == 4 )
            {
                machine_state = 0x07;
                A8_Modbus.Write_Register_Add = Receive_Data_From_A8[index-4]*256 + Receive_Data_From_A8[index-3];
                A8_Modbus.Write_Register_Data_One = Receive_Data_From_A8[index-2]*256 + Receive_Data_From_A8[index-1];
                write_one_receive_timer = 0;
            }	 	 
        }break;
	case 0x04: 
        {   //���յ�д���ܵ�ַ�����ݸ���(����ֽ�)
            Receive_Data_From_A8[index++] = data;
            write_more_receive_timer++;
            if( write_more_receive_timer == 4 )
            {
                machine_state = 0x08;
                write_more_receive_timer = 0;
                A8_Modbus.Write_Register_Add = Receive_Data_From_A8[index-4]*256 + Receive_Data_From_A8[index-3];
                A8_Modbus.Write_Register_Num = (Receive_Data_From_A8[index-2]*256 + Receive_Data_From_A8[index-1])*2;
            }			      	 
        }break;
        case 0x06: 
        {   //��������һ�������Ĵ����ж�
            Receive_Data_From_A8[index++] = data;	
            read_receive_timer++;
            if(read_receive_timer == 2)
            {			
              u16 cal_crc;
                //������յ������� CRC�����
                cal_crc=ModBus_CRC16_Calculate(Receive_Data_From_A8,6);
                //�жϽ��յ���CRC�����������Ƿ���ͬ��
                receive_CRC_L = Receive_Data_From_A8[6];
                receive_CRC_H = Receive_Data_From_A8[7];
                if(((cal_crc>>8) == receive_CRC_H) 
                   && ((cal_crc&0xff) == receive_CRC_L) )
                {
                    A8_Modbus.err_state = 0x00;//CRCУ����ȷ 
                    if(Receive_Data_From_A8[3]<ULTRA_SONIC_REG_ADDR_OFFSET)
                    {
                      AckModBusReadReg(A8_Modbus.Read_Register_Add,A8_Modbus.Read_Register_Num);
                    }
                    else
                    {
                      //Transit_UltarSonic(Receive_Data_From_A8,8);
                    }
                }
                else	  
                {
                    A8_Modbus.err_state = 0x04;
                    AckModBusCrcError(CMD_ModBus_Read);
                }   
                index = 0;  
                read_receive_timer = 0;  
                machine_state = 0x00;
            }
        }break;
	case 0x07: 
        {   //д����  ��һ�Ĵ���
            Receive_Data_From_A8[index++] = data;	
        	write_one_receive_timer++;
            if(write_one_receive_timer == 2)
            {
              u16 cal_crc;
                //������յ������� CRC�����
                cal_crc=ModBus_CRC16_Calculate(Receive_Data_From_A8,6);
            	//�жϽ��յ���CRC�����������Ƿ���ͬ��
                receive_CRC_L = Receive_Data_From_A8[6];
                receive_CRC_H = Receive_Data_From_A8[7];
                if(((cal_crc>>8) == receive_CRC_H) 
                   && ((cal_crc&0xff) == receive_CRC_L) )
                {
                    A8_Modbus.err_state = 0x00;//CRCУ����ȷ 		
                    if(Receive_Data_From_A8[3]<ULTRA_SONIC_REG_ADDR_OFFSET)
                    {
                      AckModBusWriteOneReg(A8_Modbus.Write_Register_Add,A8_Modbus.Write_Register_Data_One);
                    }
                    else
                    {
                      //Transit_UltarSonic(Receive_Data_From_A8,8);
                    }
                }
                else	  
                {
                    A8_Modbus.err_state = 0x04;//CRCУ�����    
                    AckModBusCrcError(CMD_ModBus_Write);
                }

                machine_state = 0x00;
                index = 0;  
                write_one_receive_timer = 0;  					
            }
        }break;
        case 0x08: 
        {   //д����������Ĵ���   
            Receive_Data_From_A8[index++] = data;
            A8_Modbus.Write_Register_Num--;					   
            if(A8_Modbus.Write_Register_Num == 0)
            {
                machine_state = 0x09;
                write_more_receive_timer = 2;					   
            }   
        }break;
        case 0x09: 
        {    
            Receive_Data_From_A8[index++] = data;
            write_more_receive_timer--;					   
            if(write_more_receive_timer == 0)
            {
              u16 cal_crc;
                //������յ������� CRC�����
                cal_crc=ModBus_CRC16_Calculate(Receive_Data_From_A8,((Receive_Data_From_A8[4]*256 + Receive_Data_From_A8[5])*2 + 6));
                //�жϽ��յ���CRC�����������Ƿ���ͬ��
                receive_CRC_L = Receive_Data_From_A8[index-2];
                receive_CRC_H = Receive_Data_From_A8[index-1];	                   
                if(((cal_crc>>8) == receive_CRC_H) && 
                   ((cal_crc&0xff) == receive_CRC_L) )
                {
                    A8_Modbus.err_state = 0x00;//CRCУ����ȷ 	
                    if(Receive_Data_From_A8[3]<ULTRA_SONIC_REG_ADDR_OFFSET)
                    {
                      A8_Modbus.Write_Register_Num = (Receive_Data_From_A8[4]*256 + Receive_Data_From_A8[5]);
                      AckModBusWriteMultiReg(A8_Modbus.Write_Register_Add,A8_Modbus.Write_Register_Num,&Receive_Data_From_A8[6]);
                    }
                    else
                    {
                      //Transit_UltarSonic(Receive_Data_From_A8,index);
                    }                    
                }
                else	  
                {
                    A8_Modbus.err_state = 0x04;  
                    AckModBusCrcError(CMD_ModBus_WriteMore);
                }
                machine_state = 0x00;
                index = 0;  
                write_more_receive_timer = 0;				   
            }   
        }break;
        case 0x0A:
        {
            AckModBusFunctionError(A8_Modbus.ModBus_CMD);
            machine_state = 0x00;
        }break;
        case 0x0B:
        {//�����һ���ֽڲ��ǵ�ַ��������ȴ�����������������ͬ��
          //����ʲô���������ȴ���ʱ����״̬���Զ��ָ�
        }break;
    }
}


/*��ȡ�Ĵ���*/
u8 AckModBusReadReg(u16 reg_addr,u16 reg_num)
{
  u16 index=0;
  u16 loop;
  if(((reg_addr+reg_num)<=(MOD_BUS_REG_START_ADDR+MOD_BUS_REG_NUM))&&(reg_num<=MOD_BUS_REG_NUM))
  {
    u16* pBuf=&MOD_BUS_Reg.M_CONTROL_MODE;
    u16 cal_crc;
    pBuf=&pBuf[reg_addr];
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=pBuf[loop]>>8;//MSB
      Send_Data_A8_array[index++]=pBuf[loop]&0xFF;//LSB
    }
    cal_crc = ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }
  else if((reg_addr==0x10)&&(reg_num==6))
  {//��ȡRFID
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    if((RFID_COMEIN_Flag&(2+4))==2)
    {
      for(loop=0;loop<reg_num;loop++)
      {
        Send_Data_A8_array[index++]=RFID_BLOCK_Data[loop<<1];
        Send_Data_A8_array[index++]=RFID_BLOCK_Data[(loop<<1)+1];
      }
      //RFID_COMEIN_Flag|=4;
    }
    else
    {
      for(loop=0;loop<reg_num;loop++)
      {
        Send_Data_A8_array[index++]=0xFF;
        Send_Data_A8_array[index++]=0xFF;
      }      
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }
  else if((reg_addr==0x20)&&(reg_num==8))
  {//��ȡ����������
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=HallValue[loop<<1];
      Send_Data_A8_array[index++]=HallValue[(loop<<1)+1];
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }
  else if((reg_addr==0x21)&&(reg_num==1))
  {//��ȡ����״̬
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    if(BUTTON_IM_STOP_Flag) 
    {
      M_Status=M_STATUS_IM_STOP; 
    }
    else
    {
      //if((LEFT_MOTO_TIM_ENUM->CCR1==ZERO_SPEED_PWM_COUNTER)
      //  &&(RIGHT_MOTO_TIM_ENUM->CCR2==ZERO_SPEED_PWM_COUNTER))
      if(0)
      {
        M_Status=M_STATUS_STOP;
      }
      else
      {
        M_Status=M_STATUS_NOMAL;
      }
    }
    
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=M_Status>>8;
      Send_Data_A8_array[index++]=M_Status&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }
  else if((reg_addr==0x22)&&(reg_num==1))
  {//��ȡ�������
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=M_BAT_Precent>>8;
      Send_Data_A8_array[index++]=M_BAT_Precent&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }
  else if((reg_addr==0x23)&&(reg_num==6))
  {//��ȡ���´�����
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=M_LightSensorStatus[loop]>>8;
      Send_Data_A8_array[index++]=M_LightSensorStatus[loop]&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }    
  else if((reg_addr==0x31)&&(reg_num==2))
  {//��ȡ�����������
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=U_M_CONTROL_Op.AsU16[loop]>>8;
      Send_Data_A8_array[index++]=U_M_CONTROL_Op.AsU16[loop]&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }
  else if((reg_addr==0x33)&&(reg_num==4))
  {
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=0;
      Send_Data_A8_array[index++]=(U_M_CONTROL_Op.M_CONTROL_OPTION.LeftSpeed<0)?1:0;
      Send_Data_A8_array[index++]=0;
      Send_Data_A8_array[index++]=abs(U_M_CONTROL_Op.M_CONTROL_OPTION.LeftSpeed);
      Send_Data_A8_array[index++]=0;
      Send_Data_A8_array[index++]=(U_M_CONTROL_Op.M_CONTROL_OPTION.RightSpeed<0)?1:0;
      Send_Data_A8_array[index++]=0;
      Send_Data_A8_array[index++]=abs(U_M_CONTROL_Op.M_CONTROL_OPTION.RightSpeed);      
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;    
  }
  else if((reg_addr==0x38)&&(reg_num==4))
  {
    u16 cal_crc;
    u32 L,R;
    L=*(u32*)(&LeftRoadLength);
    R=*(u32*)(&RightRoadLength);
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=(L>>24)&0xFF;
      Send_Data_A8_array[index++]=(L>>16)&0xFF;
      Send_Data_A8_array[index++]=(L>>8)&0xFF;
      Send_Data_A8_array[index++]=(L>>0)&0xFF;;
      Send_Data_A8_array[index++]=(R>>24)&0xFF;
      Send_Data_A8_array[index++]=(R>>16)&0xFF;
      Send_Data_A8_array[index++]=(R>>8)&0xFF;
      Send_Data_A8_array[index++]=(R>>0)&0xFF;      
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;   
  }
  else if((reg_addr==0x3c)&&(reg_num==4))
  {
    //λ�ƿ�������Ķ�ȡ
    //��ȡ����λ�ƿ���
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    //for(loop=0;loop<reg_num;loop++)
    {
      //u32 displacement=DISPLACEMENT_MOVEMENT_OPTION_List.buf[DISPLACEMENT_MOVEMENT_OPTION_List.In_index&LIST_LENGTH_MASK].value;
      Send_Data_A8_array[index++]=0;
      //Send_Data_A8_array[index++]=
      //  DISPLACEMENT_MOVEMENT_OPTION_List.buf[DISPLACEMENT_MOVEMENT_OPTION_List.In_index&LIST_LENGTH_MASK].dir&0xFF;
      Send_Data_A8_array[index++]=0;
      Send_Data_A8_array[index++]=0;
      //Send_Data_A8_array[index++]=(displacement>>24)&0XFF;
      //Send_Data_A8_array[index++]=(displacement>>16)&0XFF;
      //Send_Data_A8_array[index++]=(displacement>>8)&0XFF;
      //Send_Data_A8_array[index++]=(displacement>>0)&0XFF;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;    
  }
  else if((reg_addr==0x4B)&&(reg_num==1))
  {
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=MOD_BUS_Reg.AUTO_FOLLOW_SPEED_CONTROL_MODE>>8;
      Send_Data_A8_array[index++]=MOD_BUS_Reg.AUTO_FOLLOW_SPEED_CONTROL_MODE&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;    
  }
  else if((reg_addr==0x4C)&&(reg_num==1))
  {
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=MOD_BUS_Reg.AUTO_FOLLOW_SPEED>>8;
      Send_Data_A8_array[index++]=MOD_BUS_Reg.AUTO_FOLLOW_SPEED&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;    
  }  
  else if((reg_addr==0x4D)&&(reg_num==4))
  {
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=0;
      Send_Data_A8_array[index++]=0;
      Send_Data_A8_array[index++]=0;
      Send_Data_A8_array[index++]=0;
      Send_Data_A8_array[index++]=(MOD_BUS_Reg.RFID_WAIT_TIME_IN_MS>>24)&0xFF;
      Send_Data_A8_array[index++]=(MOD_BUS_Reg.RFID_WAIT_TIME_IN_MS>>16)&0xFF;
      Send_Data_A8_array[index++]=(MOD_BUS_Reg.RFID_WAIT_TIME_IN_MS>>8)&0xFF;
      Send_Data_A8_array[index++]=(MOD_BUS_Reg.RFID_WAIT_TIME_IN_MS>>0)&0xFF;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;    
  }   
  else if((reg_addr==0x51)&&(reg_num==1))
  {//��ȡRFID���߳�ʱʱ��
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=MOD_BUS_Reg.RFID_ONLINE_TIME_IN_MS>>8;
      Send_Data_A8_array[index++]=MOD_BUS_Reg.RFID_ONLINE_TIME_IN_MS&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }
  else if((reg_addr==0x52)&&(reg_num==1))
  {//��ȡRFID����״̬
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=RFID_ONLINE_Flag>>8;
      Send_Data_A8_array[index++]=RFID_ONLINE_Flag&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  } 
  else if((reg_addr==0x53)&&(reg_num==1))
  {//��ȡ�ֲ�
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=MB_LINE_DIR_SELECT>>8;
      Send_Data_A8_array[index++]=MB_LINE_DIR_SELECT&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }     
  else if((reg_addr==0x55)&&(reg_num==1))
  {//��ȡ�̵���״̬
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=Relay_status>>8;
      Send_Data_A8_array[index++]=Relay_status&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }    
  else if((reg_addr==0x56)&&(reg_num==1))
  {//��ȡ ��尴�� ״̬,20160801
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=UPLOAD_BUTTON_STATUS>>8;
      Send_Data_A8_array[index++]=UPLOAD_BUTTON_STATUS&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }   
  else if((reg_addr==0x5a)&&(reg_num==3))
  {
    //��ȡǰPID����
    u16 cal_crc;
    s16 PID_List[3];
    PID_List[0]=(Pid.Kp*1000);
    PID_List[1]=(Pid.Ki*1000);
    PID_List[2]=(Pid.Kd*1000);
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=(PID_List[loop]>>8)&0xFF;
      Send_Data_A8_array[index++]=PID_List[loop]&0xFF;      
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1; 
  }  
  else if((reg_addr==0x5F)&&(reg_num==1))
  {//��ȡ Ѳ��ʱת�����ʱ�ٶ�
    u16 cal_crc;
    u16 value = Get_FollowLineTempBaseSpeed();
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=(reg_num<<1)>>8;//byte length ,MSB
    Send_Data_A8_array[index++]=(reg_num<<1)&0xFF;//byte length ,LSB
    
    //for(loop=0;loop<reg_num;loop++)
    {
      Send_Data_A8_array[index++]=value>>8;
      Send_Data_A8_array[index++]=value&0xff;
    }
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
    return 1;
  }
  else
  {//���ݴ��󡢳�����Χ illegal_data;Return-Code=0x03
    u16 cal_crc;
    Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
    Send_Data_A8_array[index++]=CMD_ModBus_Read;
    Send_Data_A8_array[index++]=illegal_register;
    cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
    Send_Data_A8_array[index++]=cal_crc&0xFF;
    Send_Data_A8_array[index++]=cal_crc>>8;
    FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
  }
  return 0;
}

u8 AckModBusCrcError(u8 CMD_ModBus)
{
  u16 index=0;
  u16 cal_crc;
  Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
  Send_Data_A8_array[index++]=CMD_ModBus;
  Send_Data_A8_array[index++]=crc_err;
  cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
  Send_Data_A8_array[index++]=cal_crc&0xFF;
  Send_Data_A8_array[index++]=cal_crc>>8;
  FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
  return 0;  
}

u8 AckModBusFunctionError(u8 CMD_ModBus)
{
  u16 index=0;
  u16 cal_crc;
  Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
  Send_Data_A8_array[index++]=CMD_ModBus;
  Send_Data_A8_array[index++]=illegal_function;
  cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
  Send_Data_A8_array[index++]=cal_crc&0xff;
  Send_Data_A8_array[index++]=cal_crc>>8;
  FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
  return 0;  
}

u8 AckModBusWriteOneReg(u16 reg_addr,u16 reg_value)
{
  u16 index=0;
  u8 return_code=return_OK;
  u16 cal_crc;
  switch(reg_addr)
  {
  case 0x0003://�����Զ�Ѳ�ߵ�ʹ��
    if(reg_value<=1)
    {
      MOD_BUS_Reg.AUTO_FOLLOW_ENABLE=reg_value;
      MOD_BUS_REG_FreshFlag=1;
      return_code=return_OK;        
    }
    else
    {
      return_code=illegal_data;
    }
    break;
  case 0x0000://���ÿ���ģʽ
    //if((reg_value>=M_CONTROL_MODE_FOLLOW_LINE)&&(reg_value<=M_CONTROL_MODE_FOLLOW_COMMAND_IN_DISP))
    if(reg_value<=M_CONTROL_MODE_FOLLOW_COMMAND_IN_DISP)
    {
      MOD_BUS_Reg.M_CONTROL_MODE=reg_value;
      MOD_BUS_REG_FreshFlag=1;
      return_code=return_OK;    
    }
    else
    {
      return_code=illegal_data;
    }    
    break;
  case 0x0002://���ôӻ���ַ
    MOD_BUS_Reg.SLAVE_ADDR=reg_value;
    MOD_BUS_REG_FreshFlag=1;
    return_code=return_OK;
    break;
  case 0x0001://���ò�����
    if((reg_value>=1)&&(reg_value<=8))
    {
      MOD_BUS_Reg.COMM_BD=reg_value;
      MOD_BUS_REG_FreshFlag=1;
      return_code=return_OK;
    }
    else
    {
      return_code=illegal_data;
    }
    break;
  case 0x004B:
    {
      if(reg_value<=1)
      {
        if(MOD_BUS_Reg.AUTO_FOLLOW_SPEED_CONTROL_MODE!=reg_value)
        {
          MOD_BUS_Reg.AUTO_FOLLOW_SPEED_CONTROL_MODE=reg_value;
          //MOD_BUS_REG_FreshFlag=1;
        }
        return_code=return_OK;
      }
      else
      {
        return_code=illegal_data;
      }      
    }
    break;
  case 0x004c:
    {
      if(reg_value<=100)
      {
        if(MOD_BUS_Reg.AUTO_FOLLOW_SPEED!=reg_value)
        {
          MOD_BUS_Reg.AUTO_FOLLOW_SPEED=reg_value;
          MOD_BUS_REG_FreshFlag=1;
        }
        return_code=return_OK;
      }
      else
      {
        return_code=illegal_data;
      }         
    }
    break;
  case 0x0051:
    {
      if(MOD_BUS_Reg.RFID_ONLINE_TIME_IN_MS!=reg_value)
      {
        MOD_BUS_Reg.RFID_ONLINE_TIME_IN_MS=reg_value;
        MOD_BUS_REG_FreshFlag=1;
      }
      return_code=return_OK;      
    }
    break; 
  case 0x0053:
    {//�ֲ淽��ѡ�񣬲��洢
      if(reg_value<=1)
      {
        MB_LINE_DIR_SELECT=reg_value;
        //Harris 20160608
        if((SelectDir&SELECT_DIR_START_FLAG)==0)
        {
          SelectDir=MB_LINE_DIR_SELECT?SELECT_DIR_RIGHT:SELECT_DIR_LEFT;
        }
        return_code=return_OK;
      }
      else
      {
        return_code=illegal_data;
      }         
    }
    break;    
  case 0x0055:
    {//�ֲ淽��ѡ�񣬲��洢
      if(reg_value<=3)
      {
        Relay_status=reg_value;
        if(Relay_status&2)
        {
          SetRelay(0,RELAY_ON);
        }
        else
        {
          SetRelay(0,RELAY_OFF);
        }
        
        if(Relay_status&1)
        {
          SetRelay(1,RELAY_ON);
        }
        else
        {
          SetRelay(1,RELAY_OFF); 
        }
        if(Relay_status&4)
        {
          SetRelay(2,RELAY_ON);
        }
        else
        {
          SetRelay(2,RELAY_OFF); 
        }           
        
        return_code=return_OK;
      }
      else
      {
        return_code=illegal_data;
      }         
    }
    break;
  case 0x5a:
    {//PID.Kp
      if(reg_value<32767)
      {
        Pid.Kp=((float)reg_value)/1000.0f;
        return_code=return_OK;
      }
      else
      {
        return_code=illegal_data;
      }    
    }
    break;     
  case 0x5b:
    {//PID.Ki
      if(reg_value<32767)
      {
        Pid.Ki=((float)reg_value)/1000.0f;
        return_code=return_OK;
      }
      else
      {
        return_code=illegal_data;
      }    
    }
    break;   
  case 0x5c:
    {//PID.Kd
      if(reg_value<32767)
      {
        Pid.Kd=((float)reg_value)/1000.0f;
        return_code=return_OK;
      }
      else
      {
        return_code=illegal_data;
      }    
    }
    break;   
  case 0x5F:
    {//������ʱѲ���ٶ�
      if(reg_value==0)
      {
        Clear_FollowLineTempBaseSpeed();
        return_code=return_OK;
      }
      else if(reg_value<=500)
      {
        Set_FollowLineTempBaseSpeed(reg_value);
        return_code=return_OK;
      }
      else
      {
        return_code=illegal_data;
      }    
    }   
    break;  
  default:
    return_code=illegal_register;
  }
  
  //�ظ��û�
  Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
  Send_Data_A8_array[index++]=CMD_ModBus_Write;
  Send_Data_A8_array[index++]=return_code;
  cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
  Send_Data_A8_array[index++]=cal_crc&0xff;
  Send_Data_A8_array[index++]=cal_crc>>8;
  FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
  return 0;    
}

u8 AckModBusWriteMultiReg(u16 reg_addr,u16 reg_num,u8* pData)
{
  u16 index=0;
  u8 return_code=return_OK;  
  u16 cal_crc;
  switch(reg_addr)
  {
  case 0x0031://���Ƶ��
    if(reg_num==2)
    {  
      u16 dir,speed;
      dir=(((u16)pData[0])<<8)|(((u16)pData[1])<<0);
      speed=(((u16)pData[2])<<8)|(((u16)pData[3])<<0);
      
      if((dir<=M_CMD_RIGHT)&&(speed<=100))
      {
        U_M_CONTROL_Op.M_CONTROL_OPTION.M_Dir=dir;
        U_M_CONTROL_Op.M_CONTROL_OPTION.M_Speed=speed;
        U_M_CONTROL_Op.M_CONTROL_OPTION.M_FreshFlag=1;
        switch(dir)
        {
        case M_CMD_STOP:
          U_M_CONTROL_Op.M_CONTROL_OPTION.LeftSpeed=0;
          U_M_CONTROL_Op.M_CONTROL_OPTION.RightSpeed=0;
          break;
        case M_CMD_FORWARD:
          U_M_CONTROL_Op.M_CONTROL_OPTION.LeftSpeed=speed;
          U_M_CONTROL_Op.M_CONTROL_OPTION.RightSpeed=speed;
          break;
        case M_CMD_BACKWARD:
          U_M_CONTROL_Op.M_CONTROL_OPTION.LeftSpeed=-speed;
          U_M_CONTROL_Op.M_CONTROL_OPTION.RightSpeed=-speed;
          break;
        case M_CMD_LEFT:
          U_M_CONTROL_Op.M_CONTROL_OPTION.LeftSpeed=0;
          U_M_CONTROL_Op.M_CONTROL_OPTION.RightSpeed=speed;
          break;
        case M_CMD_RIGHT:
          U_M_CONTROL_Op.M_CONTROL_OPTION.LeftSpeed=speed;
          U_M_CONTROL_Op.M_CONTROL_OPTION.RightSpeed=0;
          break;          
        }
        //MOD_BUS_REG_FreshFlag=1;
        return_code=return_OK; 
      }
      else return_code=illegal_data;
    }
    else 
    {
      return_code=illegal_data;
    }
    break;
  case 0x33://���Ƶ����ײ�
    if(reg_num==4)
    {
      u16 L_dir,R_dir,L_Speed,R_Speed;
      L_dir=(((u16)pData[0])<<8)|(((u16)pData[1])<<0);
      L_Speed=(((u16)pData[2])<<8)|(((u16)pData[3])<<0);
      R_dir=(((u16)pData[4])<<8)|(((u16)pData[5])<<0);
      R_Speed=(((u16)pData[6])<<8)|(((u16)pData[7])<<0);
      if((L_dir<=1)&&(R_dir<=1)&&(L_Speed<=100)&&(L_Speed<=100))
      {
        U_M_CONTROL_Op.M_CONTROL_OPTION.M_FreshFlag=1;
        if(L_dir)
          U_M_CONTROL_Op.M_CONTROL_OPTION.LeftSpeed=-L_Speed;
        else
          U_M_CONTROL_Op.M_CONTROL_OPTION.LeftSpeed=L_Speed;
        if(R_dir)
          U_M_CONTROL_Op.M_CONTROL_OPTION.RightSpeed=-R_Speed;
        else        
          U_M_CONTROL_Op.M_CONTROL_OPTION.RightSpeed=R_Speed;
      }
      else return_code=illegal_data;
    }
    else return_code=illegal_data;
    break;
  case 0x38:
    {
      if(reg_num==4)
      {
        u32 L,R;
        L=(((u32)pData[0])<<24)|(((u32)pData[1])<<16)|(((u32)pData[2])<<8)|(((u32)pData[3])<<0);
        R=(((u32)pData[4])<<24)|(((u32)pData[5])<<16)|(((u32)pData[6])<<8)|(((u32)pData[7])<<0);
        LeftRoadLength=*(s32*)(&L);
        RightRoadLength=*(s32*)(&R);
      }
      else return_code=illegal_data;   
    }
    break;
  case 0x3c:
    {//λ�ƿ�������
      if(reg_num==4)
      {
        u32 dir,displacement;
        dir=(((u32)pData[0])<<8)|(((u32)pData[1])<<0);
        displacement=(((u32)pData[4])<<24)|(((u32)pData[5])<<16)|(((u32)pData[6])<<8)|(((u32)pData[7])<<0);
        
        if((dir<=1)&&(MOD_BUS_Reg.M_CONTROL_MODE==M_CONTROL_MODE_FOLLOW_COMMAND_IN_DISP))
        {
          //MOVEMENT_OPTION* pM=
          //  &DISPLACEMENT_MOVEMENT_OPTION_List.buf[DISPLACEMENT_MOVEMENT_OPTION_List.In_index&LIST_LENGTH_MASK];
          //pM->dir=dir;
          //pM->value=displacement;
          //DISPLACEMENT_MOVEMENT_OPTION_List.In_index+=1;

          return_code=return_OK; 
        }
        else return_code=illegal_data;
      }
      else return_code=illegal_data;  
    }
    break;
  case 0x4d:
    {
      if(reg_num==4)
      {
        u32 temp;
        if(pData[0]|pData[1]|pData[2]|pData[3])
        {
          temp=0xFFFFFFFF;
        }
        else
        {
          temp=(((u32)pData[4])<<24)|(((u32)pData[5])<<16)|(((u32)pData[6])<<8)|(((u32)pData[7])<<0);
        }
        if(temp!=MOD_BUS_Reg.RFID_WAIT_TIME_IN_MS)
        {
          MOD_BUS_Reg.RFID_WAIT_TIME_IN_MS=temp;
          MOD_BUS_REG_FreshFlag=1;
        }
      }
      else return_code=illegal_data;    
    }
    break;    
  default:
    return_code=illegal_register;
  }
  
  //�ظ��û�
  Send_Data_A8_array[index++]=MOD_BUS_Reg.SLAVE_ADDR;
  Send_Data_A8_array[index++]=CMD_ModBus_WriteMore;
  Send_Data_A8_array[index++]=return_code;
  cal_crc=ModBus_CRC16_Calculate(Send_Data_A8_array , index);
  Send_Data_A8_array[index++]=cal_crc&0xff;
  Send_Data_A8_array[index++]=cal_crc>>8;
  FillUartTxBufN(Send_Data_A8_array,index,U_TX_INDEX);
  return 0;      
}

void MOD_BUS_REG_Backup(void)
{
  memcopy((void*)&MOD_BUS_Reg,(void*)&MOD_BUS_Reg_Backup,sizeof(MOD_BUS_REG));  
}

#define MIN_FLASH_RECOVER_TIMER 1000  
u8 RecoverFlashFlag=0;
u16 RecoverFlash_Timeout=0;
void MOD_BUS_REG_MODIFY_Check(void)
{
  if(MOD_BUS_REG_FreshFlag==1)
  {
    MOD_BUS_REG_FreshFlag=0;
    if(memcompare((void*)&MOD_BUS_Reg,(void*)&MOD_BUS_Reg_Backup,sizeof(MOD_BUS_REG))
       !=sizeof(MOD_BUS_REG))
    {
      //�޸Ĳ�����
      if(MOD_BUS_Reg.COMM_BD!=MOD_BUS_Reg_Backup.COMM_BD)
      {
        //����MODBUS ������
        if(MOD_BUS_Reg.COMM_BD<MOD_BUS_BD_LIST_LENGTH)
        {
          //Usart4_Init_op(MOD_BUS_BD_LIST[MOD_BUS_Reg.COMM_BD]);
          Usart2_Init_op(MOD_BUS_BD_LIST[MOD_BUS_Reg.COMM_BD]);
          //RS485_SLAVE_TX_2_RX_Delay=RS485_SLAVE_TX_2_RX_DELAY_List[MOD_BUS_Reg.COMM_BD];
          //printf("MOD BUS SPEED PARA %ld,N,8,1\r\n",MOD_BUS_BD_LIST[MOD_BUS_Reg.COMM_BD]);
        }
      }
      //��������Ӧ�ĺ�����ȥ��ȡ���ı�
      //���±���
      MOD_BUS_REG_Backup();

      RecoverFlash_Timeout=MIN_FLASH_RECOVER_TIMER;
      RecoverFlashFlag=1;

    }
  }
  if(RecoverFlashFlag)
  {
    if(RecoverFlash_Timeout==0)
    {
      RecoverFlashFlag=0;
#if(MODBUS_PARA_REFLUSH_FLASH_ENABLE)      
      SaveFlashModBusData((void*)&MOD_BUS_Reg);
      printf("RECOVER MOD BUS Flash!\r\n");
#else
      printf("DEBUG_NO_REFRESH!\r\n");
#endif      
    }
  }
}

