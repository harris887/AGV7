#ifndef __LEDDISP_H__
#define __LEDDISP_H__

#include "user_inc.h"

#define LED1_PIN      GPIO_Pin_3
#define LED1_PORT     GPIOE
#define LED2_PIN      GPIO_Pin_4
#define LED2_PORT     GPIOE
#define LED3_PIN      GPIO_Pin_5
#define LED3_PORT     GPIOE


#define LED5_PIN      GPIO_Pin_11
#define LED5_PORT     GPIOA
#define LED6_PIN      GPIO_Pin_12
#define LED6_PORT     GPIOA


#define SetLED1()       (LED1_PORT->BSRR=LED1_PIN)
#define ClrLED1()       (LED1_PORT->BRR=LED1_PIN)
#define SetLED2()       (LED2_PORT->BSRR=LED2_PIN)
#define ClrLED2()       (LED2_PORT->BRR=LED2_PIN)
#define SetLED3()       (LED3_PORT->BSRR=LED3_PIN)
#define ClrLED3()       (LED3_PORT->BRR=LED3_PIN)


#define SetLED5()       (LED5_PORT->BSRR=LED5_PIN)
#define ClrLED5()       (LED5_PORT->BRR=LED5_PIN)
#define SetLED6()       (LED6_PORT->BSRR=LED6_PIN)
#define ClrLED6()       (LED6_PORT->BRR=LED6_PIN)

//#define KEY_PIN      GPIO_Pin_8
//#define KEY_PORT     GPIOA


extern void LedDispInit(void);
extern void LED_DISPLAY_Reset(void);
extern void LED_WATER_Display(u16 SPEED);
extern void LED_LOW_POWER_Display(u16 SPEED);
extern void LED_IM_STOP_Display(u16 SPEED);
extern void LED_FOLLOW_LINE_Display(u16 SPEED);
extern void LED_BARRIER_Display(u16 SPEED);
extern void LED_RFID_Display(u16 SPEED);
#endif