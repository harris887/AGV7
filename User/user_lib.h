#ifndef _user_lib_h
#define _user_lib_h
#include "user_inc.h"

#define PI 3.1415926

s32 lmax(s32 A,s32 B);
s32 lmin(s32 A,s32 B);

void UPLOAD_CAR_STATUS_Task(void);
s16 StartPWMFilter(s16 t0,s16 t1);
float GetAutoTiltAngleK(float CurrentAngle,float sd_k);
u32 abs_32(s32 value);
float roundf(float a);

#endif