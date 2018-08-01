#ifndef __Adc_h
#define __Adc_h

/******************************************************************************/
/*预处理部分*/
#include "user_inc.h"

/******************************************************************************/
/*宏定义*/
#define ADC1_DR_Address    ((u32)0x4001244C)
#define ADC_DMA_IRQChannel      DMA1_Channel1_IRQChannel


/******************************************************************************/
#define ROLLER_PIN  GPIO_Pin_0
#define ROLLER_PORT  GPIOC
#define BATT_PIN    GPIO_Pin_3
#define BATT_PORT   GPIOC


#define AD_Batt             (adc_data[0])+69  //电池
#define AD_Roller           (adc_data[1])  //电位器

#define THRESHOLD_ROLLER_AD_FORWARD   512   //1.5/12
#define THRESHOLD_ROLLER_AD_BACKWARD  3584  //10.5/12

#define THRESHOLD_ROLLER_AD_FORWARD_DEAD   376   //1.1/12
#define THRESHOLD_ROLLER_AD_BACKWARD_DEAD  3720  //10.9/12

extern u16 BatteryVoltSampleTimeOut;
extern u8 BatteryVolt_LowFlag;//1-电池电压低，0-电池电压正常
extern u16 adc_data[16];
extern u32 I_RollAd;
extern void Adc_init( void );
void AD_DMA_IrqHandler(void);
extern void CheckBatteryVolt_TASK(void);
#endif

