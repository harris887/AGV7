#ifndef _speed_line_h
#define _speed_line_h


typedef struct
{
  u16 UpTime;
  u16 DownTime;
  u16 Round_100MS_PulseNum;//100MS,近似走的脉冲数，因为希望调整周期为100ms所以设定此值
}SPEED_STEP_OPTION;

#define SPEED_STEP_NUM  100
extern const SPEED_STEP_OPTION SPEED_STEP_OPTION_List[SPEED_STEP_NUM];

SPEED_STEP_OPTION* GetSpeedStepOption(u16 speed_1_100);









#endif