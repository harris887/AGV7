#include "user_inc.h"
#include "math.h"
#include "string.h"

u32 NumOfSysTickInt=0;
//u16 Timer10ms;
u16 Timer_debug=0;
u8 debug_show=0;



void SysTick_Init(u16 ms)
{ 
  /*时钟配置8分频*/
 SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
 /*延时1s的计数值。*/
 SysTick_SetReload(9000*ms);  
 /*使能中断*/
 SysTick_ITConfig(ENABLE);
 /*开始计数*/
 SysTick_CounterCmd(SysTick_Counter_Enable);
}

void SysTick_IrqHandler(void)
{
    NumOfSysTickInt++;
    if(RFID_ONLINE_Timeout!=0) RFID_ONLINE_Timeout--;
    if(Uart1RxTime!=0) Uart1RxTime--;
    if(Uart4RxTime!=0) Uart4RxTime--;
    if(Uart2RxTime!=0) Uart2RxTime--;
    if(Uart3RxTime!=0) Uart3RxTime--;
    if(Uart4RxTime!=0) Uart4RxTime--;
    if(Uart5RxTime!=0) Uart5RxTime--;
    if(HallSensor_Timeout!=0) HallSensor_Timeout--;
    if(RecoverFlash_Timeout!=0) RecoverFlash_Timeout--;
    if(MOTO_RS485_RX_TX_Timeout!=0) MOTO_RS485_RX_TX_Timeout--;
    if(REMOTE_SINGLE_CHANNAL_Timtout!=0) REMOTE_SINGLE_CHANNAL_Timtout--;
    if(REMOTE_CHANNAL_CHANGE_Delay!=0) REMOTE_CHANNAL_CHANGE_Delay--;
    if(BatteryVoltSampleTimeOut!=0) BatteryVoltSampleTimeOut--;
    if(AGV_Delay!=0) AGV_Delay--;
    if(RFID_ReadBlockTimeout!=0) RFID_ReadBlockTimeout--;
    if(PID_TimeOut!=0) PID_TimeOut--;
    if(ROAD_RECORD_Timeout) ROAD_RECORD_Timeout--;
    if(RFID_STOP_ANGIN_Timeout) RFID_STOP_ANGIN_Timeout--;
    if(ProgramControlCycle) ProgramControlCycle--;

    if(voice_time_out) voice_time_out--;
    if(LoopDetectThing_time_out) LoopDetectThing_time_out--;
    if(JumpTimer) JumpTimer--;
    if(LaserTimeout) LaserTimeout--;
    if(LaserBeepTimeout) LaserBeepTimeout--;
    
    if(MOTO_485COMM_Timeout) MOTO_485COMM_Timeout--;
    if(MOTO_READ_RPM_Timeout[0]) MOTO_READ_RPM_Timeout[0]--;
    if(MOTO_READ_RPM_Timeout[1]) MOTO_READ_RPM_Timeout[1]--;    
    if(DIDO_COMM_Timeout) DIDO_COMM_Timeout--;
    if(DIDO_READ_LIGHT_Timeout) DIDO_READ_LIGHT_Timeout--;
    if(WK2124_Timeout) WK2124_Timeout--;
    if(MiningAgvTimeout) MiningAgvTimeout--;
    
    I_RollAd-=(I_RollAd>>8);
    I_RollAd+=AD_Roller;
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    
    Timer_debug++;
    if(Timer_debug>=2000)//2000
    {
        Timer_debug=0;
        debug_show=1;
    }
}

void Delay_us(u16 nTime)
{
  if(nTime!=0)
  { 
    /*设置分频系数，72M/（1+1）=36M*/
    ABS_DELAY_TIMER->PSC = (u16) (1); 
    /*重装载分频系数*/
    ABS_DELAY_TIMER->EGR = 1;
    /*清除标志位*/
    ABS_DELAY_TIMER->SR=0;
    /*失能中断*/
    ABS_DELAY_TIMER->DIER= (u16) 0x00; 
   
    if(nTime>(0xffff/34)) nTime=0xffff;
    else nTime = (nTime*34);
    /*设置重载寄存器。*/
    ABS_DELAY_TIMER->ARR =  (u16)nTime;    
    /*使能定时器6.*/
    ABS_DELAY_TIMER->CR1 = (u16) 0x5;   
    
    /*等待标志位置位*/
    while(((ABS_DELAY_TIMER->SR)&0x1) == 0x00);  
    /*失能定时器6.*/
    ABS_DELAY_TIMER->CR1 = 0; 
     /*清除标志位*/
    ABS_DELAY_TIMER->SR=0;
  }
}


void Delay_ms(u16 nTime)
{
  if(nTime!=0)
  { 
    ABS_DELAY_TIMER->PSC = (u16) (36000-1);
    ABS_DELAY_TIMER->EGR = 1;
    ABS_DELAY_TIMER->SR=0;
    /*不使能中断*/
    ABS_DELAY_TIMER->DIER= (u16) 0x00; 
    if(nTime>=(0xffff/2)) nTime = 0xffff;
    else nTime = (nTime*2);
    /*设置重载寄存器。*/
    ABS_DELAY_TIMER->ARR =  nTime;    
    /*使能定时器6.*/
    ABS_DELAY_TIMER->CR1 = (u16)0x5;   
    
    /*等待标志位置位*/
    while(((ABS_DELAY_TIMER->SR)&0x1) == 0x0);  
    /*不使能定时器6.*/
    ABS_DELAY_TIMER->CR1 = 0; 
    ABS_DELAY_TIMER->SR=0;
  }
}

















