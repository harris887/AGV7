#include "user_inc.h"
#include "string.h"

#define BUTTON_PRINTF_DEBUG 0
u8 remote_value = 0;
u8 remote_fresh = 0;

u8 TOUCH_SENSOR_Flag = 0;

typedef struct
{
  u8 ButtonPro;
  u8 ButtonStatus;//0-Ì§Æð£¬1-°´ÏÂ
  u8 HoldTime;
  u8 ButtonStatusChangFlag;
}BUTTON_OPTION;

BUTTON_OPTION BUTTON_Op[BUTTON_NUM];

u8 BUTTON_IM_STOP_Flag = 0;
u16 BUTTON_FOLLOW_LINE_AND_PROGRAM_Flag = 0;




