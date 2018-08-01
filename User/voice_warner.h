#ifndef _voice_warner_h_
#define _voice_warner_h_

typedef enum
{
  BAT_LOW_10P = 0,
  BAT_LOW_20P,
  
  SELF_TEST,
  
  AT_1_PLACE,
  AT_2_PLACE,
  AT_3_PLACE,
  AT_4_PLACE,
  AT_5_PLACE,
  AT_6_PLACE,
  AT_7_PLACE,
  AT_8_PLACE,
  AT_9_PLACE,
  AT_10_PLACE,
  AT_11_PLACE,
  AT_12_PLACE,
  
  TO_1_PLACE,
  TO_2_PLACE,
  TO_3_PLACE,
  TO_4_PLACE,
  TO_5_PLACE,
  TO_6_PLACE,
  TO_7_PLACE,
  TO_8_PLACE,
  TO_9_PLACE,
  TO_10_PLACE,
  TO_11_PLACE,
  TO_12_PLACE,  
  
  DETECT_TING,
  
  CANCLE_TO_X_PLACE,
  
  VOICE_NUM,
}VOICE_CONTEXT;

typedef struct
{
  VOICE_CONTEXT ctx;
  u8 index;
  u16 last_time;
}voice_option;


extern const voice_option voice_all[VOICE_NUM];
extern u16 voice_time_out;

void Play_Warning(VOICE_CONTEXT ctx);
void VOICE_PLAY_TASK(void);

#endif