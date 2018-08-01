#ifndef _Pid_h_
#define _Pid_h_




typedef struct
{
  s32 Setvalue;
  s32 CurrentValue;
  s32 Diff;
  s32 Err_A0;
  s32 Err_A1;
  s32 Integral;
  s32 Out;
  float Kp;
  float Ki;
  float Kd;
  s32 delay[3];
}PID_OPTION;


extern u16 PID_TimeOut;
extern PID_OPTION Pid;

void PID_Init(void);
//s32 PULSE_PID(u8 expect_mid,u8 current_mid);
s32 SPEED_PID(u16 expect_mid,u16 current_mid);











#endif