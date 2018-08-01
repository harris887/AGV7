#include "user_inc.h"
#include "string.h"

#define BUTTON_PRINTF_DEBUG 0
u8 remote_value = 0;
u8 remote_fresh = 0;

u8 TOUCH_SENSOR_Flag = 0;

typedef struct
{
  u8 ButtonPro;
  u8 ButtonStatus;//0-抬起，1-按下
  u8 HoldTime;
  u8 ButtonStatusChangFlag;
}BUTTON_OPTION;

BUTTON_OPTION BUTTON_Op[BUTTON_NUM];

u8 BUTTON_IM_STOP_Flag=0;
u16 BUTTON_FOLLOW_LINE_AND_PROGRAM_Flag=0;

void BUTTON_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin =  BUTTON_FOLLOW_LINE_PIN ; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
  GPIO_Init(BUTTON_FOLLOW_LINE_PORT, &GPIO_InitStructure); 
  
  GPIO_InitStructure.GPIO_Pin =  BUTTON_IMM_STOP_PIN ; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
  GPIO_Init(BUTTON_IMM_STOP_PORT, &GPIO_InitStructure); 
  
  GPIO_InitStructure.GPIO_Pin =  BUTTON_TOUCH_0_PIN | BUTTON_TOUCH_1_PIN; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
  GPIO_Init(BUTTON_TOUCH_PORT, &GPIO_InitStructure);   
  
  GPIO_InitStructure.GPIO_Pin =  BUTTON_VT_PIN ; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
  GPIO_Init(BUTTON_VT_PORT, &GPIO_InitStructure);    
  
  GPIO_InitStructure.GPIO_Pin =  VT_D1_PIN|VT_D2_PIN|VT_D3_PIN|VT_D4_PIN ; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
  GPIO_Init(VT_DX_PORT, &GPIO_InitStructure); 
  
  memset(BUTTON_Op,0,sizeof(BUTTON_Op));
  BUTTON_IM_STOP_Flag=0;
  BUTTON_FOLLOW_LINE_AND_PROGRAM_Flag=0;  
}

u8 GetVt(void)
{
  return ((VT_DX_PORT->IDR&(VT_D1_PIN|VT_D2_PIN|VT_D3_PIN|VT_D4_PIN))>>12);
}

void CHECK_BUTTON_TASK(void)
{
#define AVOID_SHARK_TIME  5   //5*50ms防抖时间
  //平时为高，按下为低
  static u32 NumOfSysTickIntBk;
  static u16 counter=0;
  //static u16 hold_time=0;
  //static u16 release_time=0;
  //static u8 button_pro=0;
  u8 button_down_flag;
  u8 temp,i;
  if(NumOfSysTickIntBk!=NumOfSysTickInt)
  {
    NumOfSysTickIntBk=NumOfSysTickInt;
    counter++;
    if(counter>2)
    {
      counter=0;
    }
    else return;
  }
  else return;
  

  for(i=0;i<BUTTON_NUM;i++)
  {
    switch(i)
    {
    case BUTTON_FOLLOW_LINE_Index:
      button_down_flag=(GET_BUTTON_FOLLOW_LINE_STATUS())?0:1;//低电平时表示按键按下
      break;
    case BUTTON_IMM_STOP_Index:
      button_down_flag=(GET_BUTTON_IMM_STOP_STATUS())?0:1;//低电平时表示按键按下
      break;
    case BUTTON_TOUCH_0_Index:
      button_down_flag=(GET_BUTTON_TOUCH_0_STATUS())?0:1;
      break;
    case BUTTON_TOUCH_1_Index:
      button_down_flag=(GET_BUTTON_TOUCH_1_STATUS())?0:1;
      break;      
    case BUTTON_VT_Index:
      button_down_flag = (GET_BUTTON_VT_STATUS())?1:0; //高电平时表示按键按下
      break;
    }
    
    switch(BUTTON_Op[i].ButtonPro)
    {
    case 0:
      if(button_down_flag!=0)
      {
        if(BUTTON_Op[i].HoldTime>=AVOID_SHARK_TIME) 
        {
          BUTTON_Op[i].ButtonStatus=1;
          BUTTON_Op[i].ButtonStatusChangFlag=1;
          BUTTON_Op[i].HoldTime=0;
          BUTTON_Op[i].ButtonPro=1;

#if (BUTTON_PRINTF_DEBUG)
          printf("%d ON \n",i);
#endif
        }
        else
        {
          BUTTON_Op[i].HoldTime+=1;
        }
      }
      else
      {
        BUTTON_Op[i].HoldTime=0;
      }
      break;
    case 1:
      if(button_down_flag==0)
      {
        if(BUTTON_Op[i].HoldTime>=AVOID_SHARK_TIME) 
        {
          BUTTON_Op[i].ButtonStatus=0;
          BUTTON_Op[i].ButtonStatusChangFlag=1;
          BUTTON_Op[i].HoldTime=0;
          BUTTON_Op[i].ButtonPro=0;
          //SetBeep(1,500,500);
#if (BUTTON_PRINTF_DEBUG)
          printf("%d OFF \n",i);
#endif
        }
        else
        {
          BUTTON_Op[i].HoldTime+=1;
        }
      }
      else
      {
        BUTTON_Op[i].HoldTime=0;
      }      
      break;
    }
  }
  BUTTON_FOLLOW_LINE_AND_PROGRAM_Flag=BUTTON_Op[BUTTON_FOLLOW_LINE_Index].ButtonStatus?0:1; 
  BUTTON_IM_STOP_Flag=BUTTON_Op[BUTTON_IMM_STOP_Index].ButtonStatus;
  //触碰开关作为第5个广电开关
  if(BUTTON_Op[BUTTON_TOUCH_1_Index].ButtonStatus)
  {
    TOUCH_SENSOR_Flag|=(1<<0);
    M_LightSensorStatus[4]=1;
  }
  else
  {
    TOUCH_SENSOR_Flag&=~(1<<0);
    M_LightSensorStatus[4]=0;
  }
  
  if(BUTTON_Op[BUTTON_TOUCH_0_Index].ButtonStatus)
  {
    TOUCH_SENSOR_Flag|=(1<<1);
    M_LightSensorStatus[5]=1;
  }
  else
  {
    TOUCH_SENSOR_Flag&=~(1<<1);
    M_LightSensorStatus[5]=0;
  }
  
  if((BUTTON_Op[BUTTON_VT_Index].ButtonStatusChangFlag!=0)&&(BUTTON_Op[BUTTON_VT_Index].ButtonStatus!=0))
  {
    BUTTON_Op[BUTTON_VT_Index].ButtonStatusChangFlag = 0;
    remote_value = GetVt();
    remote_fresh = 1;
    
    
    SetBeep(1,100,50);
#if (BUTTON_PRINTF_DEBUG)    
    printf("Remote %d\n",remote_value);
#endif
  }
}


