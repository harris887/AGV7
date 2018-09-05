#ifndef _voice_warner_h_
#define _voice_warner_h_

typedef enum
{
  BAT_LOW_20P = 0,
  BAT_LOW_30P,
  START_CHARGE,
  STOP_CHARGE,
  
  SELF_TEST_START ,
  SELF_TEST_ERROR,
  SELF_TEST_OK,
  
  AUTO_FOLLOW_LINE,
  REMOTE_MODE,

  DETECT_TING,
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