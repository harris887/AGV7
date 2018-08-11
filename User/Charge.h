#ifndef _charge_h_
#define _charge_h_

#include "user_inc.h"



extern MODBUS_SAMPLE MODBUS_Charge;


void Analysis_Receive_From_Charge(u8 data,MODBUS_SAMPLE* pMODBUS);
void SET_Charge(u8 on_off);








#endif
