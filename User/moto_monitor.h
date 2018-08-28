#ifndef _moto_monitor_h_
#define _moto_monitor_h_

#include "user_inc.h"

typedef struct
{
  s16 real_rpm;
  s16 real_mms;
  s16 real_rpm_reg;//从驱动器读回来的值
  s16 rev_3;
  u32 counter;
}MONITOR_STATUS;

//#define COFF_001RPM_TO_MMS  0.0633554 
//#define COFF_MMS_TO_RPM     0.1578396

extern float COFF_001RPM_TO_MMS;
//extern float COFF_MMS_TO_RPM;
extern float COFF_01RPM_TO_MMS;
extern float COFF_MMS_TO_D1RPM;
extern float COFF_DISTANCE;
extern MONITOR_STATUS MONITOR_St[MOTO_NUM];
extern MODBUS_SAMPLE MODBUS_Monitor;
extern u16 MOTO_READ_RPM_Timeout[MOTO_NUM];

extern void MONITOR_STATUS_Init(void);
extern void Analysis_Receive_From_Monitor(u8 data,MODBUS_SAMPLE* pMODBUS, MONITOR_STATUS* st);



#endif