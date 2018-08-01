#include "user_inc.h"


void Spi1_MasterInit(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = WK2124_CS_PIN ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(WK2124_CS_PORT, &GPIO_InitStructure);  
  
  /* 配置引脚: SCK 和 MOSI，复用推挽输出 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5  | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* 配置引脚:  MISO--浮空输入  */ 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
 
  /* SPI寄存器配置---------------------- */
    /* SPI1 --------  配置如下:
        - 双向双线全双工通信 
        - SPI主设备模式
        - 8位帧结构
        - 时钟悬空高
        - 数据捕获于第二时钟沿
        - 内部NSS信号有SSI位控制
        - 波特率预分频值为8
        - 数据传输从MSB位开始
        - CRC多项式为7
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
  SPI_Init(SPI1, &SPI_InitStructure);

  /*----使能 SPI1 ----------- */
  SPI_Cmd(SPI1, ENABLE);
  
  WK2124_CS_HIGH();
}


u8 Spi1_SendReceiveByte(u8 byte)
{
  /* 等待上一个数据发送完*/
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  /* 发送数据 */
  SPI_I2S_SendData(SPI1, byte);

  /* 等待接收数据完成 */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  /* 返回接收的数据*/
  return SPI_I2S_ReceiveData(SPI1);
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
  Spi1_SendReceiveByte(0x40|(port<<4)|reg);   
  while(len--)
  {
    *dat++ = Spi1_SendReceiveByte(0);
  }
  WK2124_CS_HIGH();
  return 0;
}

u8 wk2xxx_write_reg(u8 port,u8 reg,u8 *dat,u8 len)
{
  WK2124_CS_LOW();
  Spi1_SendReceiveByte((port<<4)|reg);   
  while(len--)
  {
    Spi1_SendReceiveByte(*dat++);
  }
  WK2124_CS_HIGH();
  return 0;
}

u8 wk2xxx_read_fifo(u8 port,u8 *dat,u8 len)
{
  WK2124_CS_LOW();
  Spi1_SendReceiveByte(0xC0|(port<<4));   
  while(len--)
  {
    *dat++ = Spi1_SendReceiveByte(0);
  }
  WK2124_CS_HIGH();
  return 0;
}

u8 wk2xxx_write_fifo(u8 port,u8 *dat,u8 len)
{
  WK2124_CS_LOW();
  Spi1_SendReceiveByte(0x80|(port<<4));   
  while(len--)
  {
    Spi1_SendReceiveByte(*dat++);
  }
  WK2124_CS_HIGH();
  return 0;
}

void WK2124_Init(void)
{
  u8 data[16],i;
  //配置硬件
  Spi1_MasterInit();
  //配置寄存器
  data[0]=0x3F;//使能4个串口
  data[1]=0x00;//软件复位4个串口
  wk2xxx_write_reg(WK2XXX_GPORT,WK2XXX_GENA,data,2);
  data[0]=0x00;//禁止4个串口的中断
  wk2xxx_write_reg(WK2XXX_GPORT,WK2XXX_GIER,data,1);
  
  Delay_ms(100);
  //分别配置4个串口
  for(i=0;i<4;i++)
  {
    //--------- SPAGE0 ---------
    data[0]=0;
    wk2xxx_write_reg(i,WK2XXX_SPAGE,data,1);
    
    data[0]=0x03;//RX_EN,TX_EN,SLEEP_DISABLE
    data[1]=0x00;//STOP_1BIT,0校验,无校验位，普通模式，正常输出
    data[2]=0x0C;//0xC0;//接收不复位,发送不复位，使能接收FIFO,使能发送FIFO,
    data[3]=0x00;//禁止所有中断
    wk2xxx_write_reg(i,WK2XXX_SCR,data,4);
    //--------- SPAGE1 ---------
    data[0]=1;
    wk2xxx_write_reg(i,WK2XXX_SPAGE,data,1);    
    //11.0592Mhz晶振，115200bps-0x0005
    data[0]=0x00;//
    data[1]=0x05;//
    data[2]=0x00;
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
  static u32 NumOfSysTickIntBk;
  static u8 temp_buf[256];
  u8 data[8],i,j;
  
  if(NumOfSysTickInt==NumOfSysTickIntBk) 
  {
    return;
  }
  else
  {
    NumOfSysTickIntBk=NumOfSysTickInt;
  }
  
  //-----------------------------
  for(i=0;i<4;i++)
  {
     wk2xxx_read_reg(i,WK2XXX_RFCNT,data,1);
     if(data[0]!=0)
     {
        wk2xxx_read_fifo(i,temp_buf,data[0]);
        FillUartTxBufN(temp_buf,data[0],1);

        //解析串口收到的数据
        for(j=0;j<data[0];j++)
        {
          switch(i)
          {
          case 0:
            
            break;
          case 1:
            
            break;
          case 2:
            
            break;
          case 3:
            
            break;            
          }
        }
     }
  }
}

void WK2124_Test(void)
{
  u8 td[8];
  u8 data[32],i;
  static u32 NumOfSysTickIntBk;
  static u8 pro=0;
#if 0
  wk2xxx_read_reg(WK2XXX_GPORT,WK2XXX_GENA,data,2);
  wk2xxx_read_reg(WK2XXX_GPORT,WK2XXX_GIER,data+2,2);
  
  //--------- SPAGE0 ---------
  td[0]=0;
  wk2xxx_write_reg(0,WK2XXX_SPAGE,td,1);     
  wk2xxx_read_reg(0,WK2XXX_SPAGE,data+4,11);
  
  //--------- SPAGE1 ---------
  td[0]=1;
  wk2xxx_write_reg(0,WK2XXX_SPAGE,td,1);  
  wk2xxx_read_reg(0,WK2XXX_BAUD1,data+15,5);
   
  for(i=0;i<20;i++)
  {
    printf("0x%x ",data[i]);
  }
  printf("\n");
#else
  
  FillUartTxBuf_NEx("1234567890  ",12,0);
  wk2xxx_read_reg(0,WK2XXX_TFCNT,data,2);
  
  printf("T=%d,R=%d\n",data[0],data[1]);
  

#endif
}