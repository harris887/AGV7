#include "user_inc.h"


u16 PID_TimeOut=0;

#define MAX_INTERGE 500     //积分限定值

PID_OPTION Pid={0,0,0,0,0,0,0,
  4.0,  //Kp  //10
  0.0,  //Ki
  3.0   //Kd
  };



void PID_Init(void)
{
  Pid.Integral=0;
  PID_TimeOut=100;
}

s32 SPEED_PID(u16 expect_mid,u16 current_mid)
{
  //if((current_mid<MIN_SENSOR_VALUE)||(current_mid>MAX_SENSOR_VALUE)) return 0;//没有差速脉冲
  
  Pid.Setvalue=expect_mid;
  Pid.CurrentValue=current_mid;
  Pid.Err_A1=Pid.CurrentValue-Pid.Setvalue;
  Pid.Integral+=Pid.Err_A1;
  if(Pid.Integral>MAX_INTERGE) Pid.Integral=MAX_INTERGE;
  else if(Pid.Integral<-MAX_INTERGE) Pid.Integral=-MAX_INTERGE;
  
  Pid.Diff=Pid.Err_A1-Pid.Err_A0;
  Pid.Err_A0=Pid.Err_A1;
  Pid.Out=(Pid.Kp*Pid.Err_A1)+(Pid.Ki*Pid.Integral)+(Pid.Kd*Pid.Diff); 
  
  return Pid.Out;
}

