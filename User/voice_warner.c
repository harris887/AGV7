#include "user_inc.h"
#include "string.h"

#define VOICE_COM             CH_VOICE 

#define VOICE_MODULE_V0       0
#define VOICE_MODULE_V1       1
#define VOICE_MODULE_VERSION  VOICE_MODULE_V1 //0-老版需要停止，1-新版不需要停止

#define MAX_VOICE_VOLUME      0x1c
#define DEFAULT_VOICE_VOLUME  0x10  
#define VOICE_TIME_OUT        3000
const u8 VOICE_SELECT[7]={0x01,0x51,0x00,0x00,0x10,0x00,0x02};
u8 voice_data_buf[7];
u16 voice_time_out = 0;
u8 voice_volume = 0;
u8 voice_index = 1;
u16 voice_time_out_value = VOICE_TIME_OUT;
u8 voice_play_flag = 0;

const voice_option voice_all[VOICE_NUM]=
{
  { BAT_LOW_10P , 1 , 3900 },
  { BAT_LOW_20P , 2 , 3600 },
  
  { SELF_TEST   , 11 , 10500},
  
  { AT_1_PLACE   , 21 , 1500},
  { AT_2_PLACE   , 22 , 1500},
  { AT_3_PLACE   , 23 , 1500},
  { AT_4_PLACE   , 24 , 1500},
  { AT_5_PLACE   , 25 , 1500},
  { AT_6_PLACE   , 26 , 1500},
  { AT_7_PLACE   , 27 , 1500},
  { AT_8_PLACE   , 28 , 1500},
  { AT_9_PLACE   , 29 , 1500},
  { AT_10_PLACE   , 30 , 1500},
  { AT_11_PLACE   , 31 , 1500},
  { AT_12_PLACE   , 32 , 1500},  
  
  { TO_1_PLACE   , 41 , 1500},
  { TO_2_PLACE   , 42 , 1500},
  { TO_3_PLACE   , 43 , 1500},
  { TO_4_PLACE   , 44 , 1500},
  { TO_5_PLACE   , 45 , 1500},
  { TO_6_PLACE   , 46 , 1500},
  { TO_7_PLACE   , 47 , 1500},
  { TO_8_PLACE   , 48 , 1500},
  { TO_9_PLACE   , 49 , 1500},
  { TO_10_PLACE   , 50 , 1500},
  { TO_11_PLACE   , 51 , 1800},
  { TO_12_PLACE   , 52 , 1800},    
  
  { DETECT_TING   , 61 , 3500}, 
  
  { CANCLE_TO_X_PLACE   , 4 , 1500}, 
  
};

void Play_Warning(VOICE_CONTEXT ctx)
{
  voice_index = voice_all[ctx].index;
  voice_time_out_value = voice_all[ctx].last_time;
  voice_volume = DEFAULT_VOICE_VOLUME;
  
  voice_play_flag = 1;
}

void PLAY_Voice(u8 index,u8 volume)
{
  u8 i,tmp;
  volume = (volume>MAX_VOICE_VOLUME)?MAX_VOICE_VOLUME:volume;
  memcpy(voice_data_buf,VOICE_SELECT,sizeof(VOICE_SELECT));
  voice_data_buf[2] = index;
  voice_data_buf[4] = volume;
  tmp = 0;
  for(i=0;i<5;i++)
  {
    tmp^=voice_data_buf[i];
  }
  voice_data_buf[5] = tmp;
  
  FillUartTxBuf_NEx(voice_data_buf, sizeof(VOICE_SELECT), VOICE_COM);
  //FillUartTxBufN(voice_data_buf,sizeof(VOICE_SELECT),VOICE_COM);
  //FillUartTxBufN(voice_data_buf,sizeof(VOICE_SELECT),1); //debug
}

void VOICE_PLAY_TASK(void)
{
  static u8 pro=0;
  switch(pro)
  {
  case 0:
    {
      if(voice_play_flag)
      {
        voice_play_flag=0;
        PLAY_Voice(voice_index,voice_volume);
        voice_time_out = voice_time_out_value;
        pro+=1;
      }
    }
    break;
  case 1:
    {
      if(voice_play_flag) pro=0;
      if(voice_time_out==0)
      {
#if (VOICE_MODULE_VERSION==VOICE_MODULE_V0)
        PLAY_Voice(0,0);
#endif
        pro=0;
      }
    }
    break;
  default: pro=0;
  }
}