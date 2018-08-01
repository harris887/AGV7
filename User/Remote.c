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
u8 REMOTE_SelectFlag=0;//1-�ֱ�ң��̬ON,0-�ֱ�ң��̬OFF
//u8 ControlMode=0;//0-�Զ����ƣ�1-ң��
REMOTE_OPTION REMOTE_OPTION_List[REMOTE_CHANNEL_NUM];

void REMOTE_Init(void)
{
    //1us��������1
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
   
  //select port init ��ʼ��
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
  clr_Ctrl_PWM_enable;//�͵�ƽ��Ч

  //Ctrl_PWM_IN������+�ж� 
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
ѡ��PWM����ͨ��
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
��ȡPWM����״̬	,����ֵΪ�ߵ͵�ƽֵ
********************************************************************/
u8 PWM_In_Data(void)
{
  return Read_Ctrl_PWM_in;
}

void CacluteRemoteSpeed(s16* pFourWheelSpeed)
{
  //֧�֣�
  //1-ǰ����2-���ˣ�3-��ת��4-��ת
  u8 move_mode=0;
  s32 up_down_dir,left_right_dir;
  s32 Speed_gain;
  s16 abs_speed;
  //������Ч�Լ�⣬�����
  
  up_down_dir=-(REMOTE_OPTION_List[DIRECTION_FORWARD_BACKWARD_CHANNEL].pwm_step);//�������¸�
  left_right_dir=REMOTE_OPTION_List[DIRECTION_LEFT_RIGHT_CHANNEL].pwm_step;//�󸺣�����
  Speed_gain=-REMOTE_OPTION_List[SPEED_GAIN_CHANNEL].pwm_step;
  
  //����ǯλ��ȷ����Ԥ����Χ
  if(up_down_dir<-400) up_down_dir=-400;
  else if(up_down_dir>400) up_down_dir=400;
  if(left_right_dir<-400) left_right_dir=-400;
  else if(left_right_dir>400) left_right_dir=400;
  
  //��Ϊ200~800֮��
  if(Speed_gain<-300) Speed_gain=-300;
  else if(Speed_gain>300) Speed_gain=300;
  Speed_gain+=500;
  
  
  //�������ƣ�����50~350,����3�õ�100���ٶ�
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
  
  //�������ƣ�����50~350,����3�õ�100���ٶ�
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
  

  //ǰ������
  //if(roll_mode==0)
  if(1)
  {
    u32 a,b;
    a=abs_32(up_down_dir);
    b=abs_32(left_right_dir);
    if(a>=b)
    {
      //1-ǰ����2-���ˣ�
      if(up_down_dir>=0) move_mode=1;
      else move_mode=2;
      abs_speed=a;
    }
    else
    {
      //3-��ת��4-��ת
      if(left_right_dir>=0) move_mode=4;
      else move_mode=3;
      abs_speed=b;
    }
  }

  
  //1--3  ͷ
  //2--4  β
  //1-ǰ����2-���ˣ�3-��ƽ�ƣ�4-��ƽ�ƣ�5-˳ʱ��תȦ��6-��ʱ��תȦ
  switch(move_mode)
  {
  case 0:
    pFourWheelSpeed[0]=0;//����   
    pFourWheelSpeed[1]=0;//����
    break;
  case 1:
    pFourWheelSpeed[0]=abs_speed;//����   
    pFourWheelSpeed[1]=abs_speed;//����
    break;
  case 2:
    pFourWheelSpeed[0]=-abs_speed;//���� 
    pFourWheelSpeed[1]=-abs_speed;//���� 
    break;
  case 3:
    pFourWheelSpeed[0]=0;//����  -abs_speed
    pFourWheelSpeed[1]=abs_speed;//����
    break;
  case 4:
    pFourWheelSpeed[0]=abs_speed;//����   
    pFourWheelSpeed[1]=0;//����-abs_speed
    break;
  }
}

//ң����ҡ��PWMɨ��
void JOYSTICK_SCAN_TASK(void)
{
  //��ȡ�ֱ����ٶ�
  switch(REMOTE_Pro)
  {
  case 0:
    {
      //�޸�ͨ����
      PWM_Port_Select(REMOTE_ChannalIndex);
      REMOTE_CHANNAL_CHANGE_Delay=2;
      REMOTE_Pro++;
    }
    break;
  case 1:
    {
      //�ȴ�ͨ���ȶ���ʼ���������
      if(REMOTE_CHANNAL_CHANGE_Delay==0)
      {
        REMOTE_TimeCounter=0;
        REMOTE_GetPulseFlag=1;
        REMOTE_SINGLE_CHANNAL_Timtout=50;//50ms ��ʱ
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
          //1.��ֹ�ⲿ�����ж�
          
          //��������
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
        REMOTE_GetPulseFlag=3;//ǿ��Ϊ����̬
        
        //��ʱ�ˣ���ͨ������
        for(i=0;i<(REMOTE_FILTER_LENGTH-1);i++)
        {
          REMOTE_OPTION_List[REMOTE_ChannalIndex].filter_buf[i]=REMOTE_OPTION_List[REMOTE_ChannalIndex].filter_buf[i+1];
        }
        REMOTE_OPTION_List[REMOTE_ChannalIndex].filter_buf[i]=0;//���������¸�ֵ0
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

//��ѯ�ֱ��ϵ�ң��ʹ�ܲ���
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
  
  //ͨ��5��ON����ң��ģʽ
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
          REMOTE_SelectFlag=true;//��Ϊң��ģʽ
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
      REMOTE_SelectFlag=false;//ң��ģʽʧЧ
      SetBeep(1,300,200);
      
#if (JOYSTICK_PRINTF_DEBUG)  
      printf("SELF PROGRAM MODE! \n");
#endif
    }
  }
}


void REMOTE_Task(void)
{
  //ң�ؿ��Ƶ������
  //if(ControlMode==CONTROL_MODE_REMOTE)
  {
    CacluteRemoteSpeed(FourWheelSpeed);
    
    SetPwm(LEFT_MOTO_INDEX,FourWheelSpeed[0]);
    SetPwm(RIGHT_MOTO_INDEX,FourWheelSpeed[1]); 
  }
}