#ifndef _REMOTE_H_
#define _REMOTE_H_

#define REMOTE_COUNTER_TIMER    TIM5


extern u8 REMOTE_SINGLE_CHANNAL_Timtout;
extern u8 REMOTE_CHANNAL_CHANGE_Delay;
//extern u8 ControlMode;//0-×Ô¶¯¿ØÖÆ£¬1-Ò£¿Ø

extern u8 REMOTE_SelectFlag;//1-ÊÖ±úÒ£¿ØÌ¬ON,0-ÊÖ±úÒ£¿ØÌ¬OFF

#define REMOTE_PWM_SELECT_PORT      GPIOD
#define REMOTE_PWM_SELECT_S0_PIN    GPIO_Pin_3
#define REMOTE_PWM_SELECT_S1_PIN    GPIO_Pin_1
#define REMOTE_PWM_SELECT_S2_PIN    GPIO_Pin_0
#define REMOTE_PWM_SELECT_PIN       (REMOTE_PWM_SELECT_S0_PIN | REMOTE_PWM_SELECT_S1_PIN | REMOTE_PWM_SELECT_S2_PIN)

#define REMOTE_PWM_CTRL_PORT        GPIOD
#define REMOTE_PWM_CTRL_PIN         GPIO_Pin_15

#define REMOTE_PWM_IN_PORT  GPIOA
#define REMOTE_PWM_IN_PIN   GPIO_Pin_8

#define set_S0  GPIO_SetBits(REMOTE_PWM_SELECT_PORT,REMOTE_PWM_SELECT_S0_PIN)
#define clr_S0  GPIO_ResetBits(REMOTE_PWM_SELECT_PORT,REMOTE_PWM_SELECT_S0_PIN)

#define set_S1  GPIO_SetBits(REMOTE_PWM_SELECT_PORT,REMOTE_PWM_SELECT_S1_PIN)
#define clr_S1  GPIO_ResetBits(REMOTE_PWM_SELECT_PORT,REMOTE_PWM_SELECT_S1_PIN)

#define set_S2  GPIO_SetBits(REMOTE_PWM_SELECT_PORT,REMOTE_PWM_SELECT_S2_PIN)
#define clr_S2  GPIO_ResetBits(REMOTE_PWM_SELECT_PORT,REMOTE_PWM_SELECT_S2_PIN)

//#define Read_Ctrl_PWM_in     GPIO_ReadInputDataBit(REMOTE_PWM_IN_PORT,REMOTE_PWM_IN_PIN)
#define Read_Ctrl_PWM_in      (REMOTE_PWM_IN_PORT->IDR & REMOTE_PWM_IN_PIN)

#define set_Ctrl_PWM_enable  GPIO_SetBits(REMOTE_PWM_CTRL_PORT,REMOTE_PWM_CTRL_PIN)
#define clr_Ctrl_PWM_enable  GPIO_ResetBits(REMOTE_PWM_CTRL_PORT,REMOTE_PWM_CTRL_PIN)

void REMOTE_Init(void);
void CacluteRemoteSpeed(s16* pFourWheelSpeed);
void REMOTE_Task(void);

void PWM_Select_Port_Init(void);
void PWM_Port_Select(u8 num);
u8 PWM_In_Data(void);


//Ò£¿ØÆ÷Ò¡¸ËÉ¨Ãè
extern void JOYSTICK_SCAN_TASK(void);
extern void CHECK_REMOTE_ENABLE_TASK(void);


#endif

