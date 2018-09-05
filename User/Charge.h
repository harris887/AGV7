#ifndef _charge_h_
#define _charge_h_

#include "user_inc.h"

#define DEFAULT_MIN_CHARGE_CYCLE_IN_S  300
#define CHARGE_ON                      1
#define CHARGE_OFF                     0
#define MIN_CHARGE_CURRENT_IN_0D01A    200
#define CHARGE_COMM_MIN_COUNTER        -30
typedef struct
{
  u16 Voltage; // ��ѹ (��λ��0.01V)
  u16 Current; // ���� (��λ��0.01A)
  u16 Cap;     // ����
  u16 Time;    // ʱ�� (��λ��min)
  u16 Refresh;
  u32 RxNum;
}CHARGE_STATUS;

extern MODBUS_SAMPLE MODBUS_Charge;
extern u32 CHARGE_COMM_Timout;
extern CHARGE_STATUS CHARGE_St;
extern u8 CHARGE_FULL_Flag;
extern s32 CHARGE_COMM_Counter;

void Analysis_Receive_From_Charge(u8 data,MODBUS_SAMPLE* pMODBUS);
void SET_Charge(u8 on_off);
void CHARGE_Task(void);







#endif
