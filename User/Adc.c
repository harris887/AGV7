#include "user_inc.h"

#define DEFAULT_BATTERY_VOLT_SAMPLE_TIME  100   //100ms��ȡһ�ε�ص�ѹ
#define DEFAULT_BATTERY_VOLT_FILTER_LENGTH  128 //�˲�������128��12.8s
#define DEFAULT_BATTERY_VOLT_STATUS_HOLD_TIME 128 //״̬���ĵĳ�������128��12.8s
//#define BATTERY_VOLT_LOW_THRESHOLD  1489  //20v,������Ҫ����
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
u8 BatteryVolt_LowFlag = 0;//1-��ص�ѹ�ͣ�0-��ص�ѹ����
u32 I_RollAd=0;
u16 Temprature_StatusTimer = 0;
u16 FAN_Status = 0;

void DMA1_Channel1_ForAdc1Init(void) 
{ 
    DMA_InitTypeDef DMA_InitStructure; 
    /*��λDMA,��ֹ��������Ĵ���*/
    DMA_DeInit(DMA1_Channel1); 
   
     /*--------DMA�������£�-------------
     ���� ��ַ��ADC1�����ݼĴ���
     �ڴ�ĵ�ַ���û��Զ��������
     �ڴ���ΪĿ�ĵ�ַ
     �������Ĵ�С��2�����֣�������Ҫɨ���ͨ����ȷ����
     �����ַ��������ֹ
     �ڴ��ַ������ʹ�ܣ���Ҫ��
     ����һ�����ݴ�С�����֣�16λ��
     �ڴ�һ�����ݴ�С�����֣�16λ��
     ѭ��ģʽ������Bufferд�����Զ��ص���ʼ��ַ��ʼ���� ����Ҫ��
     ���ȼ����ߣ��Ժ�Ҫ�������ã�
     �ڴ浽�ڴ洫�䣺��ֹ
    -----------------------------------*/
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address; 
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&(adc_data[0]); 
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; 
    /*BufferSize=4����ΪADCת������4��ͨ�� */
    DMA_InitStructure.DMA_BufferSize = 2; 
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; 
    /*ѭ��ģʽ������Bufferд�����Զ��ص���ʼ��ַ��ʼ���� */
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; 
    DMA_Init(DMA1_Channel1, &DMA_InitStructure); 

    /*ʹ��DMA��������ж�*/
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    
    /*������ɺ�����DMAͨ�� */
    DMA_Cmd(DMA1_Channel1, ENABLE); 
}



/*****************************************************************
�� �� ����    ADC1_ScanModeWithDmaInit
ʵ��������    ADC1��ʼ������Ҫ��;���¶ȼ��ͻҶȼ��
��    �ߣ�    HARRIS
�������ڣ�    2009-9-29     
�� �� ��    ��
�� �� ֵ��    ��              
ȫ�ֱ�����    ��
ģ��֧�֣�    
��    ����
�޸���ʷ��        
 1���޸����ڣ�
    �� �� �ˣ�
    ����������
*****************************************************************/
void Adc_init(void) 
{ 
    ADC_InitTypeDef ADC_InitStructure; 
    GPIO_InitTypeDef GPIO_InitStructure;
    /*��ʼ��DMA*/
    DMA1_Channel1_ForAdc1Init();
    

    GPIO_InitStructure.GPIO_Pin = ROLLER_PIN|BATT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(ROLLER_PORT, &GPIO_InitStructure);
     
    /*-------- ADC1��������-----------
    ADCģʽ������ģʽ
    ɨ��ģʽ��ʹ��
    ����ת��ģʽ����ֹ������ģʽʹ��
    ADC����ģʽ���������
    ���ݶ��뷽ʽ���Ҷ���
    ת�����г��ȣ�2��������Ҫ�趨��
    ----------------------------------*/
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; 
    ADC_InitStructure.ADC_ScanConvMode = ENABLE; 
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 
    //����ת�����г���Ϊ
    ADC_InitStructure.ADC_NbrOfChannel = 2;      
    ADC_Init(ADC1, &ADC_InitStructure); 
    
    /*�����û�����Ҫ�����Ҫ��ͨ�� */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_13Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 2, ADC_SampleTime_13Cycles5); 

    
    /*����ADC��DMA֧�� */
    ADC_DMACmd(ADC1, ENABLE); 

    /*ʹ��ADC1 */
    ADC_Cmd(ADC1, ENABLE); 
   
    /*������ADC�Զ�У׼����������ִ��һ�Σ���֤���� */
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
    case 0://30%������
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
    case 1://20%������,30%����
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
    case 2: // 20%����
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
    
    //--------- �¶ȼ�� ---------//
#define TEMPRATURE_THRESHOLD                      3000
#define DEFAULT_TEMPRATURE_FAN_CONTROL_COUNTER    600   // 600 * 100ms = 60s
    if(FAN_Status)
    {
      if(PACK_ANALOG_Infor.TEMP_Value[0] < TEMPRATURE_THRESHOLD) // 30.0��
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
      if(PACK_ANALOG_Infor.TEMP_Value[0] >= TEMPRATURE_THRESHOLD) // 30.0��
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