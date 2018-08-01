#ifndef __Delay_h
#define __Delay_h

#define REMOTE_FILTER_LENGTH      4
#define REMOTE_ZERO_COUNTER_VALUE 1500
#define REMOTE_CHANNEL_NUM  6
typedef struct
{
  s32 pwm_step;//
  s32 filter_buf[REMOTE_FILTER_LENGTH];//4¸ö´°¿Ú
  u32 error_counter;
  u32 valid_flag;
}REMOTE_OPTION;

extern REMOTE_OPTION REMOTE_OPTION_List[REMOTE_CHANNEL_NUM];

#define ABS_DELAY_TIMER         TIM4


#define BEEP_200MS      1
#define BEEP_400MS      2
#define Beep(cnt,time)   (BeepCnt+=cnt,BeepTime=time)

extern u8 BeepCnt,BeepTime;
extern u8   interval;
extern u16 Timer10ms;
extern u8 BeepFlag;
extern u16 Timer_debug;
extern u8 debug_show;
extern u16 Tim3IntNum;
extern u32 NumOfSysTickInt;
extern u16 time_to_open_fan;//20us
extern u8 REMOTE_GetPulseFlag;
extern u16 REMOTE_TimeCounter;


extern void SysTick_Init(u16 ms);
extern void Delay_Init(void);
extern void Delay_ms(u16 nTime);
extern void Delay_us(u16 nTime);

extern void SysTick_IrqHandler(void);


void BeepTask( void );
void REMOTE_Task(void);
void TIM2_Init(void);
void TIM3_Init(void);
void TIM3_Handler(void);
void CONTROL_MODE_TASK(void);
void CacluteRemoteSpeed(s16* pFourWheelSpeed);


#endif


