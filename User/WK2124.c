#include "user_inc.h"

#define DEFAULT_WK2124_INIT_DELAY 1000
#define DEFAULT_RD_WK2124_CYCLE   4
u16 WK2124_Timeout = DEFAULT_WK2124_INIT_DELAY;
u32 WK2124_BD_List[WK_CH_NUM] = {
  9600  , // CH_VOICE
  38400  , // CH_DIDO
  19200  , // HALL
  9600  , // RSV
};

void SPI2_MasterInit(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;  
  
  GPIO_InitStructure.GPIO_Pin = WK2124_CS_PIN ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(WK2124_CS_PORT, &GPIO_InitStructure);  
  
  /* ��������: SCK �� MOSI������������� */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13  | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* ��������:  MISO--��������  */ 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
 
  /* SPI�Ĵ�������---------------------- */
    /* SPI2 --------  ��������:
        - ˫��˫��ȫ˫��ͨ�� 
        - SPI���豸ģʽ
        - 8λ֡�ṹ
        - ʱ�����ո�
        - ���ݲ����ڵڶ�ʱ����
        - �ڲ�NSS�ź���SSIλ����
        - ������Ԥ��ƵֵΪ8
        - ���ݴ����MSBλ��ʼ
        - CRC����ʽΪ7
  */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;   
  SPI_InitStructure.SPI_CPHA =SPI_CPHA_1Edge;  
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;//SPI_BaudRatePrescaler_128;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI2, &SPI_InitStructure);

  /*----ʹ�� SPI2 ----------- */
  SPI_Cmd(SPI2, ENABLE);
  
  WK2124_CS_HIGH();
}


u8 Spi2_SendReceiveByte(u8 byte)
{
  /* �ȴ���һ�����ݷ�����*/
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

  /* �������� */
  SPI_I2S_SendData(SPI2, byte);

  /* �ȴ������������ */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

  /* ���ؽ��յ�����*/
  return SPI_I2S_ReceiveData(SPI2);
}


//==============================================================================

#define WK2XXX_PAGE1        1
#define WK2XXX_PAGE0        0

#define WK2XXX_STATUS_PE    1
#define WK2XXX_STATUS_FE    2
#define WK2XXX_STATUS_BRK   4
#define WK2XXX_STATUS_OE    8

u8 wk2xxx_read_reg(u8 port,u8 reg,u8 *dat,u8 len)
{
  WK2124_CS_LOW();
  Spi2_SendReceiveByte(0x40|(port<<4)|reg);   
  while(len--)
  {
    *dat++ = Spi2_SendReceiveByte(0);
  }
  WK2124_CS_HIGH();
  return 0;
}

u8 wk2xxx_write_reg(u8 port,u8 reg,u8 *dat,u8 len)
{
  WK2124_CS_LOW();
  Spi2_SendReceiveByte((port<<4)|reg);   
  while(len--)
  {
    Spi2_SendReceiveByte(*dat++);
  }
  WK2124_CS_HIGH();
  return 0;
}

u8 wk2xxx_read_fifo(u8 port,u8 *dat,u8 len)
{
  WK2124_CS_LOW();
  Spi2_SendReceiveByte(0xC0|(port<<4));   
  while(len--)
  {
    *dat++ = Spi2_SendReceiveByte(0);
  }
  WK2124_CS_HIGH();
  return 0;
}

u8 wk2xxx_write_fifo(u8 port,u8 *dat,u8 len)
{
  WK2124_CS_LOW();
  Spi2_SendReceiveByte(0x80|(port<<4));   
  while(len--)
  {
    Spi2_SendReceiveByte(*dat++);
  }
  WK2124_CS_HIGH();
  return 0;
}

void WK2124_Init(void)
{
  u8 data[16],i;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin = RS485_3_DIR_PIN_RE;		  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(RS485_3_DIR_R_PORT, &GPIO_InitStructure); 
  
  GPIO_InitStructure.GPIO_Pin = RS485_3_DIR_PIN_DE;		  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(RS485_3_DIR_D_PORT, &GPIO_InitStructure);   
  RS485_3_RX_Active(); 
  
  GPIO_InitStructure.GPIO_Pin = RS485_4_DIR_PIN_RE;		  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(RS485_4_DIR_R_PORT, &GPIO_InitStructure); 
  
  GPIO_InitStructure.GPIO_Pin = RS485_4_DIR_PIN_DE;		  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(RS485_4_DIR_D_PORT, &GPIO_InitStructure);   
  RS485_4_RX_Active();   
  
  //����Ӳ��
  SPI2_MasterInit();
  //���üĴ���
  data[0]=0x3F;//ʹ��4������
  data[1]=0x00;//�����λ4������
  wk2xxx_write_reg(WK2XXX_GPORT,WK2XXX_GENA,data,2);
  data[0]=0x00;//��ֹ4�����ڵ��ж�
  wk2xxx_write_reg(WK2XXX_GPORT,WK2XXX_GIER,data,1);
  
  Delay_ms(100);
  //�ֱ�����4������
  for(i=0;i<4;i++)
  {
    //--------- SPAGE0 ---------
    data[0]=0;
    wk2xxx_write_reg(i,WK2XXX_SPAGE,data,1);
    
    data[0]=0x03;//RX_EN,TX_EN,SLEEP_DISABLE
    data[1]=0x00;//STOP_1BIT,0У��,��У��λ����ͨģʽ���������
    data[2]=0x0C;//0xC0;//���ղ���λ,���Ͳ���λ��ʹ�ܽ���FIFO,ʹ�ܷ���FIFO,
    data[3]=0x00;//��ֹ�����ж�
    wk2xxx_write_reg(i,WK2XXX_SCR,data,4);
    //--------- SPAGE1 ---------
    data[0]=1;
    wk2xxx_write_reg(i,WK2XXX_SPAGE,data,1);    
    //11.0592Mhz����115200bps-0x0005 9600bps-71
    // 11059200
    if(WK2124_BD_List[i] == 115200)
    {
      data[0] = 0;  data[1] = 5;  data[2] = 0;
    }
    else if(WK2124_BD_List[i] == 57600)
    {
      data[0] = 0;  data[1] = 11;  data[2] = 0;
    }
    else if(WK2124_BD_List[i] == 38400)
    {
      data[0] = 0;  data[1] = 17;  data[2] = 0;
    }    
    else if(WK2124_BD_List[i] == 19200)
    {
      data[0] = 0;  data[1] = 35;  data[2] = 0;
    }    
    else if(WK2124_BD_List[i] == 9600)
    {
      data[0] = 0;  data[1] = 71;  data[2] = 0;
    }     
    else
    {
      data[0] = 0;  data[1] = 71;  data[2] = 0;
    }
    wk2xxx_write_reg(i,WK2XXX_BAUD1,data,3);
    
    //--------- SPAGE0 ---------
    data[0]=0;
    wk2xxx_write_reg(i,WK2XXX_SPAGE,data,1);    
  }
}

void FillUartTxBuf_NEx(u8* pData,u8 num,u8 U_0_3)
{
  wk2xxx_write_fifo(U_0_3,pData,num);  
}


void WK2124_TransTask(void)
{
  static u8 temp_buf[256];
  static u8 rx_time[WK_CH_NUM] = {0, 0, 0, 0};
  u8 data[8],i,j;
  

  //---- RX ----
  if(WK2124_Timeout == 0)
  {
    WK2124_Timeout = DEFAULT_RD_WK2124_CYCLE;
    for(i=0;i<4;i++)
    {
       wk2xxx_read_reg(i,WK2XXX_RFCNT,data,1);
       if(rx_time[i] > 0) rx_time[i] -= 1;
       if(data[0]!=0)
       {
          wk2xxx_read_fifo(i,temp_buf,data[0]);
          
          switch(i)
          {
          case CH_DIDO:
            {
              rx_time[CH_DIDO] = 5;
              for(j = 0; j < data[0]; j++)
              {
                Analysis_Receive_From_Dido(temp_buf[j], &MODBUS_Dido, &DIDO_INPUT_Status);
              }
            }
            break;
          case CH_VOICE:
            {
            
            }
            break;
          /*  */
          case CH_HALL:
            {
              rx_time[CH_HALL] = 3;
              for(j = 0; j < data[0]; j++)
              {              
                Analysis_Receive_From_HallSensor(temp_buf[j], &MODBUS_HallSensor);
              }
            }
            break;
          
          case CH_BMS:
            {
              
            }
            break;
          }
       }
    }
    
    // ��ʱ����
    if(rx_time[CH_DIDO] == 0)
    {
      if(MODBUS_Dido.MachineState)
      {
        MODBUS_Dido.MachineState = 0;
      }
    }
    /*   */
    if(rx_time[CH_HALL] == 0)
    {
      if(MODBUS_HallSensor.MachineState) 
      {
        MODBUS_HallSensor.MachineState = 0;
      }    
    }
    
  }
}

