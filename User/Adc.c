#include "user_inc.h"

#define DEFAULT_BATTERY_VOLT_SAMPLE_TIME  100   //100ms读取一次电池电压
#define DEFAULT_BATTERY_VOLT_FILTER_LENGTH  128 //滤波器长度128，12.8s
#define DEFAULT_BATTERY_VOLT_STATUS_HOLD_TIME 128 //状态更改的持续次数128，12.8s
//#define BATTERY_VOLT_LOW_THRESHOLD  1489  //20v,可能需要更改
//#define BATTERY_VOLT_HIGH_THRESHOLD 1689

#define BATTERY_00_PERCENT_AD_THRESHOLD 1694  //22.74V
#define BATTERY_10_PERCENT_AD_THRESHOLD 1735  //23.3V
#define BATTERY_30_PERCENT_AD_THRESHOLD 1817  //24.4V
#define BATTERY_100_PERCENT_AD_THRESHOLD 1966  //26.4V
/******************************************************************************/
u16 adc_data[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

u16 BatteryVoltSampleTimeOut = 4000;
u32 I_BatteryVolt = 0;
u16 I_Counter=0;
u16 BatteryVolt_Filterd=BATTERY_30_PERCENT_AD_THRESHOLD;
u16 BatteryVolt_StatusTimer = 0;
u8 BatteryVolt_LowFlag = 0;//1-电池电压低，0-电池电压正常
u32 I_RollAd=0;
u16 Temprature_StatusTimer = 0;
u16 FAN_Status = 0;

void DMA1_Channel1_ForAdc1Init(void) 
{ 
    DMA_InitTypeDef DMA_InitStructure; 
    /*复位DMA,防止出现意外的错误*/
    DMA_DeInit(DMA1_Channel1); 
   
     /*--------DMA配置如下：-------------
     外设 地址：ADC1的数据寄存器
     内存的地址：用户自定义的数组
     内存作为目的地址
     缓冲区的大小：2个半字（根据需要扫描的通道数确定）
     外设地址递增：禁止
     内存地址递增：使能（重要）
     外设一个数据大小：半字（16位）
     内存一个数据大小：半字（16位）
     循环模式开启，Buffer写满后，自动回到初始地址开始传输 （重要）
     优先级：高（以后要重新设置）
     内存到内存传输：禁止
    -----------------------------------*/
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address; 
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&(adc_data[0]); 
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; 
    /*BufferSize=4，因为ADC转换序列4个通道 */
    DMA_InitStructure.DMA_BufferSize = 2; 
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; 
    /*循环模式开启，Buffer写满后，自动回到初始地址开始传输 */
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; 
    DMA_Init(DMA1_Channel1, &DMA_InitStructure); 

    /*使能DMA传输完成中断*/
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    
    /*配置完成后，启动DMA通道 */
    DMA_Cmd(DMA1_Channel1, ENABLE); 
}



/*****************************************************************
函 数 名：    ADC1_ScanModeWithDmaInit
实现描述：    ADC1初始化，主要用途是温度检测和灰度检测
作    者：    HARRIS
创建日期：    2009-9-29     
参 数 表：    无
返 回 值：    无              
全局变量：    无
模块支持：    
其    他：
修改历史：        
 1、修改日期：
    修 改 人：
    具体描述：
*****************************************************************/
void Adc_init(void) 
{ 
    ADC_InitTypeDef ADC_InitStructure; 
    GPIO_InitTypeDef GPIO_InitStructure;
    /*初始化DMA*/
    DMA1_Channel1_ForAdc1Init();
    

    GPIO_InitStructure.GPIO_Pin = ROLLER_PIN|BATT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(ROLLER_PORT, &GPIO_InitStructure);
     
    /*-------- ADC1配置如下-----------
    ADC模式：独立模式
    扫描模式：使能
    连续转换模式：禁止，单次模式使能
    ADC启动模式：软件启动
    数据对齐方式：右对齐
    转换序列长度：2，根据需要设定。
    ----------------------------------*/
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; 
    ADC_InitStructure.ADC_ScanConvMode = ENABLE; 
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 
    //设置转换序列长度为
    ADC_InitStructure.ADC_NbrOfChannel = 2;      
    ADC_Init(ADC1, &ADC_InitStructure); 
    
    /*根据用户的需要添加需要的通道 */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_13Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 2, ADC_SampleTime_13Cycles5); 

    
    /*开启ADC的DMA支持 */
    ADC_DMACmd(ADC1, ENABLE); 

    /*使能ADC1 */
    ADC_Cmd(ADC1, ENABLE); 
   
    /*下面是ADC自动校准，开机后需执行一次，保证精度 */
    ADC_ResetCalibration(ADC1); 
    while(ADC_GetResetCalibrationStatus(ADC1)); 
} 


void AD_DMA_IrqHandler(void)
{
  //do nothing
  DMA_ClearITPendingBit(DMA1_IT_TC1);
}



void CheckBatteryVolt_TASK(void)
{
  if(BatteryVoltSampleTimeOut == 0)
  {
    BatteryVoltSampleTimeOut = DEFAULT_BATTERY_VOLT_SAMPLE_TIME;
    M_BAT_Precent = (u32)PACK_ANALOG_Infor.PACK_Left * (u32)100 / (u32)PACK_ANALOG_Infor.PACK_TotalCap;
    
    switch(BatteryVolt_LowFlag)
    {
    case 0://30%及以上
      {
        if(M_BAT_Precent < 30)
        {
          BatteryVolt_StatusTimer += 1;
          if(BatteryVolt_StatusTimer >= DEFAULT_BATTERY_VOLT_STATUS_HOLD_TIME)
          {
            BatteryVolt_StatusTimer = 0;
            BatteryVolt_LowFlag = 1;
          }
        }
        else
        {
          BatteryVolt_StatusTimer = 0;
        }      
      }
      break;
    case 1://20%及以上,30%以下
      {
        if(M_BAT_Precent >= 30)
        {
          BatteryVolt_StatusTimer += 1;
          if(BatteryVolt_StatusTimer >= DEFAULT_BATTERY_VOLT_STATUS_HOLD_TIME)
          {
            BatteryVolt_StatusTimer = 0;
            BatteryVolt_LowFlag = 0;
          }
        }
        else if(M_BAT_Precent < 20)
        {
          BatteryVolt_StatusTimer += 1;
          if(BatteryVolt_StatusTimer >= DEFAULT_BATTERY_VOLT_STATUS_HOLD_TIME)
          {
            BatteryVolt_StatusTimer = 0;
            BatteryVolt_LowFlag = 2;
          }
        }
        else
        {
          BatteryVolt_StatusTimer = 0;
        }           
      }
      break;
    case 2: // 20%以下
      {
        if(M_BAT_Precent >= 20)
        {
          BatteryVolt_StatusTimer += 1;
          if(BatteryVolt_StatusTimer >= DEFAULT_BATTERY_VOLT_STATUS_HOLD_TIME)
          {
            BatteryVolt_StatusTimer = 0;
            BatteryVolt_LowFlag = 1;
          }
        }
        else
        {
          BatteryVolt_StatusTimer = 0;
        }      
      }
      break;
    default: 
      {
        BatteryVolt_StatusTimer = 0;
        BatteryVolt_LowFlag = 0;
      }
    }    
    
    //--------- 温度检测 ---------//
#define TEMPRATURE_THRESHOLD                      3000
#define DEFAULT_TEMPRATURE_FAN_CONTROL_COUNTER    600   // 600 * 100ms = 60s
    if(FAN_Status)
    {
      if(PACK_ANALOG_Infor.TEMP_Value[0] < TEMPRATURE_THRESHOLD) // 30.0度
      {
        if(Temprature_StatusTimer < DEFAULT_TEMPRATURE_FAN_CONTROL_COUNTER)
        {
          Temprature_StatusTimer += 1;
        }
        else
        {
          FAN_Status = 0;
          SET_DIDO_Relay(DO_Fan_1, FAN_Status);
          SET_DIDO_Relay(DO_Fan_2, FAN_Status);
        }
      }
      else
      {
        Temprature_StatusTimer = 0;
      }
    }
    else
    {
      if(PACK_ANALOG_Infor.TEMP_Value[0] >= TEMPRATURE_THRESHOLD) // 30.0度
      {
        if(Temprature_StatusTimer < DEFAULT_TEMPRATURE_FAN_CONTROL_COUNTER)
        {
          Temprature_StatusTimer += 1;
        }
        else
        {
          FAN_Status = 1;
          SET_DIDO_Relay(DO_Fan_1, FAN_Status);
          SET_DIDO_Relay(DO_Fan_2, FAN_Status);
        }
      }
      else
      {
        Temprature_StatusTimer = 0;
      }
    }
  }
}