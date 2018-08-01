#ifndef _BUZZER_H_
#define _BUZZER_H_


#define RELAY_ON    1
#define RELAY_OFF   0

#define BUZZER_ON     1
#define BUZZER_OFF    0
#define BUZZER_A_PORT   GPIOC
#define BUZZER_A_PIN    GPIO_Pin_5
#define BUZZER_B_PORT   GPIOE
#define BUZZER_B_PIN    GPIO_Pin_6

#define RELAY_1_PIN     GPIO_Pin_7 
#define RELAY_2_PIN     GPIO_Pin_15
#define RELAY_3_PIN     GPIO_Pin_8 
#define RELAY_PORT      GPIOE 

extern u16 Relay_status;

void BUZZER_Init(void);
void SetBuzzer(u8 on_off);

void BEEP_TASK(void);
void SetBeep(u16 BeepTimes,u16 BeepOnMs,u16 BeepDealy);
void SetBeepForever(u16 BeepOnMs,u16 BeepDealy);
void RELAY_Init(void);
void SetRelay(u8 num_0_1,u8 on_off);




#endif