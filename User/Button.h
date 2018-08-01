#ifndef _BUTTON_H_
#define _BUTTON_H_

#define TOUCH_SENSOR_MASK         0x03 //0x3F

#define BUTTON_TOUCH_0_PIN        GPIO_Pin_11
#define BUTTON_TOUCH_1_PIN        GPIO_Pin_12
#define BUTTON_TOUCH_PORT         GPIOE

#define BUTTON_FOLLOW_LINE_PIN    GPIO_Pin_10
#define BUTTON_IMM_STOP_PIN       GPIO_Pin_9
#define BUTTON_FOLLOW_LINE_PORT   GPIOE
#define BUTTON_IMM_STOP_PORT      GPIOE

#define BUTTON_VT_PIN             GPIO_Pin_8
#define BUTTON_VT_PORT            GPIOD

#define VT_D1_PIN                 GPIO_Pin_12
#define VT_D2_PIN                 GPIO_Pin_13
#define VT_D3_PIN                 GPIO_Pin_14
#define VT_D4_PIN                 GPIO_Pin_15
#define VT_DX_PORT                GPIOB

#define GET_BUTTON_FOLLOW_LINE_STATUS()   BUTTON_FOLLOW_LINE_PORT->IDR&BUTTON_FOLLOW_LINE_PIN
#define GET_BUTTON_IMM_STOP_STATUS()      BUTTON_IMM_STOP_PORT->IDR&BUTTON_IMM_STOP_PIN
#define GET_BUTTON_TOUCH_0_STATUS()       BUTTON_TOUCH_PORT->IDR&BUTTON_TOUCH_0_PIN
#define GET_BUTTON_TOUCH_1_STATUS()       BUTTON_TOUCH_PORT->IDR&BUTTON_TOUCH_1_PIN
#define GET_BUTTON_VT_STATUS()            BUTTON_VT_PORT->IDR&BUTTON_VT_PIN

#define UPLOAD_BUTTON_STATUS   BUTTON_FOLLOW_LINE_AND_PROGRAM_Flag

typedef enum
{
  BUTTON_FOLLOW_LINE_Index=0,
  BUTTON_IMM_STOP_Index,
  BUTTON_TOUCH_0_Index,
  BUTTON_TOUCH_1_Index,
  BUTTON_VT_Index,
  BUTTON_NUM
}BUTTON_INDEX;

extern u8 BUTTON_IM_STOP_Flag;
extern u16 BUTTON_FOLLOW_LINE_AND_PROGRAM_Flag;
extern u8 remote_value;
extern u8 remote_fresh;
extern u8 TOUCH_SENSOR_Flag;

//-------------------------------------------//
extern void BUTTON_Init(void);
extern void CHECK_BUTTON_TASK(void);
extern u8 GetVt(void);




#endif