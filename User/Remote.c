#include "user_inc.h"
#include "string.h"

#define JOYSTICK_PRINTF_DEBUG 0
#define REMOTE_OR_FOLLOWLINE_SELECT_CHANNEL 4
#define DIRECTION_LEFT_RIGHT_CHANNEL  0
#define DIRECTION_FORWARD_BACKWARD_CHANNEL  1
#define SPEED_GAIN_CHANNEL  2


u8 REMOTE_Pro=0;
u8 REMOTE_ChannalIndex=0;
u8 REMOTE_SINGLE_CHANNAL_Timtout=0;
u8 REMOTE_GetPulseFlag=0;
u16 REMOTE_TimeCounter=0;
u8 REMOTE_CHANNAL_CHANGE_Delay=0;
u8 REMOTE_SelectFlag=0;//1-手柄遥控态ON,0-手柄遥控态OFF
//u8 ControlMode=0;//0-自动控制，1-遥控
REMOTE_OPTION REMOTE_OPTION_List[REMOTE_CHANNEL_NUM];

void REMOTE_Init(void)
{
    //1us计数自增1
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF; 
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(REMOTE_COUNTER_TIMER, &TIM_TimeBaseStructure);

    TIM_Cmd(REMOTE_COUNTER_TIMER, DISABLE);
    memset(REMOTE_OPTION_List,0,sizeof(REMOTE_OPTION_List));
    
    PWM_Select_Port_Init();
}

void PWM_Select_Port_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
   
  //select port init 初始化
  GPIO_InitStructure.GPIO_Pin = REMOTE_PWM_SELECT_PIN;		   //by johnson  2015-05-011 :s0-s2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(REMOTE_PWM_SELECT_PORT, &GPIO_InitStructure);
  PWM_Port_Select(0);
   
  //Ctrl_PWM_enable
  GPIO_InitStructure.GPIO_Pin = REMOTE_PWM_CTRL_PIN;		   //by johnson  2015-05-011 :Ctrl_PWM_enable
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(REMOTE_PWM_CTRL_PORT, &GPIO_InitStructure);  
  clr_Ctrl_PWM_enable;//低电平有效

  //Ctrl_PWM_IN，输入+中断 
  GPIO_InitStructure.GPIO_Pin = REMOTE_PWM_IN_PIN;		   
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(REMOTE_PWM_IN_PORT, &GPIO_InitStructure);  	 
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);  
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line8;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);    
  
}

/*******************************************************************
选择PWM控制通道
********************************************************************/
void PWM_Port_Select(u8 num)
{
     u8 num_data;
     num_data = num;
     
     switch(num_data)
     {
        case 0: {   clr_S0; clr_S1; clr_S2;     }break;
        case 1: {   clr_S0; clr_S1; set_S2;     }break;
        case 2: {   clr_S0; set_S1; clr_S2;     }break;
        case 3: {   clr_S0; set_S1; set_S2;     }break;
        case 4: {   set_S0; clr_S1; clr_S2;     }break;
        case 5: {   set_S0; clr_S1; set_S2;     }break;
        case 6: {   set_S0; set_S1; clr_S2;     }break;
        case 7: {   set_S0; set_S1; set_S2;     }break;
        default:break;                                                       
     } 
}

/*******************************************************************
读取PWM引脚状态	,返回值为高低电平值
********************************************************************/
u8 PWM_In_Data(void)
{
  return Read_Ctrl_PWM_in;
}

void CacluteRemoteSpeed(s16* pFourWheelSpeed)
{
  //支持，
  //1-前进，2-后退，3-左转，4-右转
  u8 move_mode=0;
  s32 up_down_dir,left_right_dir;
  s32 Speed_gain;
  s16 abs_speed;
  //数据有效性检测，待添加
  
  up_down_dir=-(REMOTE_OPTION_List[DIRECTION_FORWARD_BACKWARD_CHANNEL].pwm_step);//上正，下负
  left_right_dir=REMOTE_OPTION_List[DIRECTION_LEFT_RIGHT_CHANNEL].pwm_step;//左负，右正
  Speed_gain=-REMOTE_OPTION_List[SPEED_GAIN_CHANNEL].pwm_step;
  
  //数据钳位，确保在预定范围
  if(up_down_dir<-400) up_down_dir=-400;
  else if(up_down_dir>400) up_down_dir=400;
  if(left_right_dir<-400) left_right_dir=-400;
  else if(left_right_dir>400) left_right_dir=400;
  
  //变为200~800之间
  if(Speed_gain<-300) Speed_gain=-300;
  else if(Speed_gain>300) Speed_gain=300;
  Speed_gain+=500;
  
  
  //死区控制，并且50~350,除以3得到100级速度
  if(up_down_dir<-50) 
  {
    up_down_dir+=50;
    up_down_dir/=3;
    
    up_down_dir=(up_down_dir*Speed_gain)/100;
  }
  else if(up_down_dir>50)
  {
    up_down_dir-=50;
    up_down_dir/=3;
    
    up_down_dir=(up_down_dir*Speed_gain)/100;
  }
  else
  {
    up_down_dir=0;
  }
  
  //死区控制，并且50~350,除以3得到100级速度
  if(left_right_dir<-50) 
  {
    left_right_dir+=50;
    left_right_dir/=3;
    
    left_right_dir=(left_right_dir*Speed_gain)/100;
  }
  else if(left_right_dir>50)
  {
    left_right_dir-=50;
    left_right_dir/=3;
    
    left_right_dir=(left_right_dir*Speed_gain)/100;
  }
  else
  {
    left_right_dir=0;
  }  
  

  //前后左右
  //if(roll_mode==0)
  if(1)
  {
    u32 a,b;
    a=abs_32(up_down_dir);
    b=abs_32(left_right_dir);
    if(a>=b)
    {
      //1-前进，2-后退，
      if(up_down_dir>=0) move_mode=1;
      else move_mode=2;
      abs_speed=a;
    }
    else
    {
      //3-左转，4-右转
      if(left_right_dir>=0) move_mode=4;
      else move_mode=3;
      abs_speed=b;
    }
  }

  
  //1--3  头
  //2--4  尾
  //1-前进，2-后退，3-左平移，4-右平移，5-顺时针转圈，6-逆时针转圈
  switch(move_mode)
  {
  case 0:
    pFourWheelSpeed[0]=0;//左轮   
    pFourWheelSpeed[1]=0;//右轮
    break;
  case 1:
    pFourWheelSpeed[0]=abs_speed;//左轮   
    pFourWheelSpeed[1]=abs_speed;//右轮
    break;
  case 2:
    pFourWheelSpeed[0]=-abs_speed;//左轮 
    pFourWheelSpeed[1]=-abs_speed;//右轮 
    break;
  case 3:
    pFourWheelSpeed[0]=0;//左轮  -abs_speed
    pFourWheelSpeed[1]=abs_speed;//右轮
    break;
  case 4:
    pFourWheelSpeed[0]=abs_speed;//左轮   
    pFourWheelSpeed[1]=0;//右轮-abs_speed
    break;
  }
}

//遥控器摇杆PWM扫描
void JOYSTICK_SCAN_TASK(void)
{
  //获取手柄的速度
  switch(REMOTE_Pro)
  {
  case 0:
    {
      //修改通道号
      PWM_Port_Select(REMOTE_ChannalIndex);
      REMOTE_CHANNAL_CHANGE_Delay=2;
      REMOTE_Pro++;
    }
    break;
  case 1:
    {
      //等待通道稳定后开始检测正脉冲
      if(REMOTE_CHANNAL_CHANGE_Delay==0)
      {
        REMOTE_TimeCounter=0;
        REMOTE_GetPulseFlag=1;
        REMOTE_SINGLE_CHANNAL_Timtout=50;//50ms 超时
        REMOTE_Pro++;
      }
    }
    break;
  case 2:
    {
      u8 i;
      if(REMOTE_SINGLE_CHANNAL_Timtout!=0)
      {
        if(REMOTE_GetPulseFlag==3)
        {
          //1.禁止外部引脚中断
          
          //更新数据
          for(i=0;i<(REMOTE_FILTER_LENGTH-1);i++)
          {
            REMOTE_OPTION_List[REMOTE_ChannalIndex].filter_buf[i]=REMOTE_OPTION_List[REMOTE_ChannalIndex].filter_buf[i+1];
          }
          REMOTE_OPTION_List[REMOTE_ChannalIndex].filter_buf[i]=((s32)REMOTE_TimeCounter)-((s32)REMOTE_ZERO_COUNTER_VALUE);
          REMOTE_OPTION_List[REMOTE_ChannalIndex].valid_flag>>=1;
          REMOTE_OPTION_List[REMOTE_ChannalIndex].valid_flag|=(1<<i);
          if(REMOTE_OPTION_List[REMOTE_ChannalIndex].valid_flag==((1<<REMOTE_FILTER_LENGTH)-1))
          {
            s32 temp=0;
            for(i=0;i<REMOTE_FILTER_LENGTH;i++) temp+=REMOTE_OPTION_List[REMOTE_ChannalIndex].filter_buf[i];
            REMOTE_OPTION_List[REMOTE_ChannalIndex].pwm_step=temp/REMOTE_FILTER_LENGTH;
          }
          
          REMOTE_ChannalIndex++;
          REMOTE_ChannalIndex=(REMOTE_ChannalIndex>=REMOTE_CHANNEL_NUM)?0:REMOTE_ChannalIndex;
          REMOTE_GetPulseFlag=0;
          REMOTE_Pro=0;
        }
      }
      else
      {
        REMOTE_GetPulseFlag=3;//强制为空闲态
        
        //超时了，该通道出错
        for(i=0;i<(REMOTE_FILTER_LENGTH-1);i++)
        {
          REMOTE_OPTION_List[REMOTE_ChannalIndex].filter_buf[i]=REMOTE_OPTION_List[REMOTE_ChannalIndex].filter_buf[i+1];
        }
        REMOTE_OPTION_List[REMOTE_ChannalIndex].filter_buf[i]=0;//出错的情况下赋值0
        REMOTE_OPTION_List[REMOTE_ChannalIndex].valid_flag>>=1;
        REMOTE_OPTION_List[REMOTE_ChannalIndex].error_counter++;
        
        REMOTE_ChannalIndex++;
        REMOTE_ChannalIndex=(REMOTE_ChannalIndex>=REMOTE_CHANNEL_NUM)?0:REMOTE_ChannalIndex;   
        REMOTE_GetPulseFlag=0;
        REMOTE_Pro=0;
      }
    }
    break;
  }
}

#define REMOTE_SHAKE_TIME 5

//查询手柄上的遥控使能拨杆
void CHECK_REMOTE_ENABLE_TASK(void)
{
  static u32 NumOfSysTickIntBk;
  static u16 counter=0;
  static u8 ok_time=0;
  static u8 fail_time=0;
  if(NumOfSysTickIntBk!=NumOfSysTickInt)
  {
    NumOfSysTickIntBk=NumOfSysTickInt;
    counter++;
    if(counter>50)//150
    {
      counter=0;
    }
    else return;
  }
  else return;
  
  //通道5，ON启动遥控模式
  if(REMOTE_OPTION_List[REMOTE_OR_FOLLOWLINE_SELECT_CHANNEL].valid_flag==((1<<REMOTE_FILTER_LENGTH)-1))
  {
    if(REMOTE_OPTION_List[REMOTE_OR_FOLLOWLINE_SELECT_CHANNEL].pwm_step>300)
    {
      if(ok_time<REMOTE_SHAKE_TIME) 
      {
        ok_time++;  
      }
      else
      {
        if(REMOTE_SelectFlag!=true)
        {
          REMOTE_SelectFlag=true;//变为遥控模式
          SetBeep(3,100,200);
          
#if (JOYSTICK_PRINTF_DEBUG)          
          printf("REMOTE MODE! \n");
#endif
        }
      }
      fail_time=0;
    }
    else
    {
      if(fail_time<REMOTE_SHAKE_TIME) fail_time++;
      ok_time=0;
    }
  }
  else
  {
    if(fail_time<REMOTE_SHAKE_TIME) fail_time++;
    ok_time=0;
  }
  
  if(fail_time>=REMOTE_SHAKE_TIME)
  {
    if(REMOTE_SelectFlag!=false)
    {
      REMOTE_SelectFlag=false;//遥控模式失效
      SetBeep(1,300,200);
      
#if (JOYSTICK_PRINTF_DEBUG)  
      printf("SELF PROGRAM MODE! \n");
#endif
    }
  }
}


void REMOTE_Task(void)
{
  //遥控控制电机部分
  //if(ControlMode==CONTROL_MODE_REMOTE)
  {
    CacluteRemoteSpeed(FourWheelSpeed);
    
    SetPwm(LEFT_MOTO_INDEX,FourWheelSpeed[0]);
    SetPwm(RIGHT_MOTO_INDEX,FourWheelSpeed[1]); 
  }
}