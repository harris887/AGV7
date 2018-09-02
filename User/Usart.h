/****************************************************************
Copyright(C),2007-2010,AlexCybot (Beijing) Tech.Co.,Ltd.
All Rights Reserved
文 件 名： Peer_Usart.h
模块描述： 串口驱动程序
作    者： HARRIS
版    本： V1.2    
创建日期：         
全局变量：        
调用模块：        
其    他：         
修改历史：
 1、修改日期：
    修 改 人： HARRIS
    具体描述：
****************************************************************/

#ifndef _Usart_h
#define _Usart_h

/******************************************************************************/
/*预处理部分*/
#include "stm32f10x_lib.h"
#include "Protocol.h"
/*使用Printf*/
#define  PRINTF_EN

#ifdef   PRINTF_EN
  #include "stdio.h"
#endif 

#define DEBUG_USART_ENUM  2
#define DEBUG_USART       USART2  //UART5 USART1

#define false 0
#define true 1
/******************************************************************************/
/*--------------------宏定义--------------------*/


/******************************************************************************/
/*--------------------全局变量外包括--------------------*/
extern u8 USART_Byte[5];
                

           
/*上一个接受或者发送的字符*/
#define USART_BYTE USART_Byte[DEBUG_USART_ENUM - 1]


/******************************************************************************/
/*接口函数声明*/

/*****************************************************************
函 数 名：    Usart1_Init
功能描述：    芯片串口1的初始化
作    者：    HARRIS
创建日期：    2009-9-29     
参 数 表：    无
返 回 值：    无              
全局变量：    无
模块支持：    GPIO固件
*****************************************************************/
extern void Usart1_Init(void);

/*****************************************************************
函 数 名：    Usart3_Init
功能描述：    芯片串口3的初始化
作    者：    HARRIS
创建日期：    2009-9-29     
参 数 表：    无
返 回 值：    无              
全局变量：    无
模块支持：    GPIO固件
其    他：
*****************************************************************/
extern void Usart3_Init(void);


/*****************************************************************
函 数 名：    putchar
功能描述：    使用printf函数的具体实现
作    者：    HARRIS
创建日期：    2009-9-29     
参 数 表：    单个要发送的字符
返 回 值：    无              
全局变量：    无
模块支持：    GPIO固件
其    他：
修改历史：        
 1、修改日期：
    修 改 人：
    具体描述：
*****************************************************************/
extern int putchar(int ch);



typedef enum
{
  RS485_IDEL = 0,
  RS485_RX_ENABLE,
  RS485_TX_INIT,
  RS485_TX_ENABLE,
  RS485_STATUS_NUM  
}RS485_STATUS;

typedef struct
{
  u8 Intrrupt;
  u8 InIndex;
  u8 OutIndex;
  u8 Buf[256];
}UART_OPTION;
extern UART_OPTION UART_Optx;
extern UART_OPTION UART_Oprx;


extern u16 Uart1RxTime;
extern u16 Uart4RxTime;
extern u16 Uart2RxTime;
extern u16 Uart3RxTime;
extern u16 Uart5RxTime;
extern UART_OPTION UART1_Optx;
extern UART_OPTION UART1_Oprx;
extern UART_OPTION UART2_Optx;
extern UART_OPTION UART2_Oprx;
extern UART_OPTION UART5_Oprx;
extern u16 RS485_SLAVE_TX_2_RX_Delay;
extern u8 MOTO_RS485_RX_TX_Timeout;
extern u8 BMS_RS485_RX_TX_Timeout;
extern const u16 RS485_SLAVE_TX_2_RX_DELAY_List[9];

void Usart2_Init(void);
void UART1_ISR(void);
void UART2_ISR(void);
void UART_Task(void);
void FillUartTxBuf(u8 data);
u8 Deal_UART_Infor(u8* pINFOR,u8 Length);
u8 CheckSum(u8 *ptr,u8 length);
void FillUartTxBufN(u8* pData,u8 num,u8 U1_2);
void Set_zero(u8* data,u16 length);
u8 Deal_ModBus_Infor(u8* pINFOR,u8 Length);
void Usart2_Init_op(u32 bd);
void DEBUG_COM_Protocol(void);
void UART3_ISR(void);
void Usart3_Init(void);
void Usart4_Init(void);
void Usart5_Init(void);
void UART4_ISR(void);
void UART5_ISR(void);
void Usart4_Init_op(u32 bd);
void Usart1_Init_op(u32 bd);
void Usart2_Init_op(u32 bd);



#define RS485_1_DIR_PIN_RE       GPIO_Pin_14
#define RS485_1_DIR_R_PORT       GPIOA
#define RS485_1_DIR_PIN_DE       GPIO_Pin_15
#define RS485_1_DIR_D_PORT       GPIOA
#define RS485_1_RX_Active()      RS485_1_DIR_R_PORT->BRR=RS485_1_DIR_PIN_RE;RS485_1_DIR_D_PORT->BRR=RS485_1_DIR_PIN_DE
#define RS485_1_TX_Active()      RS485_1_DIR_R_PORT->BSRR=RS485_1_DIR_PIN_RE;RS485_1_DIR_D_PORT->BSRR=RS485_1_DIR_PIN_DE


#define RS485_2_DIR_PIN_RE       GPIO_Pin_3
#define RS485_2_DIR_R_PORT       GPIOB
#define RS485_2_DIR_PIN_DE       GPIO_Pin_7
#define RS485_2_DIR_D_PORT       GPIOD
#define RS485_2_RX_Active()      RS485_2_DIR_R_PORT->BRR=RS485_2_DIR_PIN_RE;RS485_2_DIR_D_PORT->BRR=RS485_2_DIR_PIN_DE
#define RS485_2_TX_Active()      RS485_2_DIR_R_PORT->BSRR=RS485_2_DIR_PIN_RE;RS485_2_DIR_D_PORT->BSRR=RS485_2_DIR_PIN_DE

extern u32 rx2;
extern u32 rx3;
extern u32 rx5;


#endif

