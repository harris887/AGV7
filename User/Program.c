#include "user_inc.h"
#include "string.h"

AGV_STATUS_LIST AGV_RUN_Pro=AGV_STATUS_INIT;
u8 AGV_RUN_SUB_Pro=0;
u32 AGV_Delay=3500;
u16 RFID_STOP_ANGIN_Timeout=0;

u16 ProgramControlCycle=0;
float VehicleWidth=DEFAULT_Vehicle_WIDTH_FLOAT;
float Displacement_coff=1.0;
float Angle_coff=1.0;
u16 SPEED_UP_Length=0;
u16 Current_ID=0;
u16 Dest_ID=0;
s16 Run_Dir = DIR_FORWARD;
u16 LoopDetectThing_time_out = DEFAULT_PLAY_DETECT_THING_TIME_IN_MS;
u8 FollowLineEnable = 0;

MOVEMENT_OPTION_LIST DISPLACEMENT_MOVEMENT_OPTION_List={0,0};
MOVEMENT_OPTION_LIST ANGLE_MOVEMENT_OPTION_List={0,0};
SPEED_UP_OPTION SPEED_UP_OPTION_List[RunFuncNum][MAX_SPEED_UP_LIST_LENGTH];

void MovementListInit(void)
{
  memset(&DISPLACEMENT_MOVEMENT_OPTION_List,0,sizeof(DISPLACEMENT_MOVEMENT_OPTION_List));
  memset(&ANGLE_MOVEMENT_OPTION_List,0,sizeof(ANGLE_MOVEMENT_OPTION_List));
}
void ClearMovementList(MOVEMENT_OPTION_LIST* pLIST)
{
  pLIST->Out_index=pLIST->In_index;
}

void AGV_RUN_Task(void)
{
  switch(AGV_RUN_Pro)
  {
  /*----------��ʼ��ģʽ---------------*/    
  case AGV_STATUS_INIT:
    {
      if(AGV_Delay==0)
      {
        //SetBeep(3,300,500);
        Play_Warning(SELF_TEST_OK);
        AGV_Delay = voice_all[SELF_TEST_OK].last_time + 2000;
        
        LED_DISPLAY_Reset();  
        AGV_RUN_Pro=AGV_STATUS_IDLE;
        AGV_RUN_SUB_Pro=0;
      }
    }
    break;
  /*----------����ģʽ---------------*/    
  case AGV_STATUS_IDLE:
    {
      if(AGV_RUN_SUB_Pro==0)
      {
        LED_WATER_Display(500);
        if(AGV_Delay==0)
        {
          remote_fresh = 0;
          Clear_FollowLineTempBaseSpeed();
          AGV_RUN_SUB_Pro+=1;
        }
      }
      else if(AGV_RUN_SUB_Pro==1)
      {
        LED_WATER_Display(500);
        
        //�͵�ѹ״̬�л�
        if(BatteryVolt_LowFlag>=2)
        {
          AGV_RUN_Pro=AGV_STATUS_LOW_POWER;
          AGV_RUN_SUB_Pro=0;
          break;
        }
        
        //��ͣ��ť���£����뼱ͣģʽ
        if(BUTTON_IM_STOP_Flag)
        {
          AGV_RUN_Pro=AGV_STATUS_IM_STOP;
          AGV_RUN_SUB_Pro=0;    
          break;
        }       
        
        //Ѳ�߰�ť����&&������⵽����
        //if((MOD_BUS_Reg.M_CONTROL_MODE==M_CONTROL_MODE_FOLLOW_LINE)
        //   &&(MOD_BUS_Reg.AUTO_FOLLOW_ENABLE) 
        //   &&(ON_LINE_Flag)
        //     &&(BUTTON_FOLLOW_LINE_AND_PROGRAM_Flag)) 
        //if((ON_LINE_Flag)&&(Current_ID==0))
        if((ON_LINE_Flag) && (FollowLineEnable))
        {
          //FollowLineEnable = 0;
          //MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR;
          //INIT_SpeedLimted();
          AGV_RUN_Pro=AGV_STATUS_FOLLOWLINE;
          AGV_RUN_SUB_Pro=0;
          break;
        }   
        
        //ң��ON��ť���½��룬ң��״̬
        if(REMOTE_SelectFlag)
        {
          AGV_RUN_Pro=AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro=0;
          break;
        }

        BATT_LOW_LEVEL_1_Warning();        
      }     
    }
    break;
  /*----------Ѳ��ģʽ---------------*/      
  case AGV_STATUS_FOLLOWLINE:
    {
      static u8 FollowLineReset=0;
      if(AGV_RUN_SUB_Pro==0)
      {
        if(AGV_Delay==0)
        {
          PID_Init();
          LED_DISPLAY_Reset();
          AGV_RUN_SUB_Pro+=1;
          RFID_COMEIN_Flag&=~1;
          FollowLineReset=1;
        }
      }
      else
      {
        LED_FOLLOW_LINE_Display(500);
        
        NEW_FOLLOW_LINE_TASK(&FollowLineReset,Run_Dir);
        
        //��ͣ��ť���£����뼱ͣģʽ
        if(BUTTON_IM_STOP_Flag)
        {
          AGV_RUN_Pro=AGV_STATUS_IM_STOP;
          AGV_RUN_SUB_Pro=0;    
          printf("-- BUTTON_IM_STOP_Flag --\n");
          break;
        }
        
        //���£�����
        if(((TOUCH_SENSOR_Flag & (1<<((Run_Dir==DIR_FORWARD)?0:1))) != 0)
          || ((LASER_SENSOR_Flag & (1<<((Run_Dir==DIR_FORWARD)?0:1))) != 0)) 
        {
          Play_Warning(DETECT_TING);
          AGV_RUN_Pro=AGV_STATUS_BARRIER;
          AGV_RUN_SUB_Pro=0;    
          printf("-- TOUCH_SENSOR_Flag --\n");
          break;
        }
        
        //ң��ON��ť���½��룬ң��״̬
        if(REMOTE_SelectFlag)
        {
          AGV_RUN_Pro=AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro=0;
          printf("-- REMOTE_SelectFlag --\n");
          break;
        }        
        //a.Ѳ����������̧�𣬽��뼱ͣģʽ
        //b.�û�Ҫ��Ѳ��ֹͣ�����뼱ͣģʽ
        //c.����Ŵ��󣬽��뼱ͣģʽ
        //if((BUTTON_FOLLOW_LINE_AND_PROGRAM_Flag==0)
        //  || (MOD_BUS_Reg.AUTO_FOLLOW_ENABLE==0)
        //    || (ON_LINE_Flag==0))
        if(ON_LINE_Flag==0)
        {
          AGV_RUN_Pro= AGV_STATUS_OFF_LINE;
          
          AGV_RUN_SUB_Pro=0;  
          printf("-- ON_LINE_Flag --\n");
          break;
        }

        //�͵�ѹ״̬�л�
        if(BatteryVolt_LowFlag>=2)
        {
          MOTO_IM_STOP();
          AGV_RUN_Pro=AGV_STATUS_LOW_POWER;
          AGV_RUN_SUB_Pro=0;
          printf("-- BatteryVolt_LowFlag --\n");
          break;
        }
        
        //����RFID
        if(RFID_COMEIN_Flag & 0x4)  
        {
          RFID_COMEIN_Flag &= ~0x4;
          //if((RFID_STOP_ANGIN_Timeout==0)&&(MOD_BUS_Reg.RFID_WAIT_TIME_IN_MS!=0))
          if(PlaceId == 0x80FF)
          {
            PlaceId = 0;
            if(Run_Dir == DIR_BACKWARD)
            {
              Run_Dir = DIR_FORWARD;
              MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR;
              AGV_RUN_Pro = AGV_STATUS_CHARGE;
              AGV_RUN_SUB_Pro = 0;   
              break;  
            }            
          }
          else if((PlaceId == 0x8001) && (MB_LINE_DIR_SELECT == 1))
          {
            PlaceId = 0;
          }
          else if(RFID_STOP_ANGIN_Timeout==0)  
          {
            if((PlaceId == 0x8001) && (MB_LINE_DIR_SELECT == 0))
            {
              Run_Dir = DIR_BACKWARD;
              MODE_BUS_HALL_Addr = BACKWARD_MODE_BUS_HALL_ADDR;
            }
            if(PlaceId == 0x8007)  
            {
              MB_LINE_DIR_SELECT = 0; // ��
            }

            //PlaceId = 0;
            AGV_RUN_Pro=AGV_STATUS_RFID_COMEIN;
            AGV_RUN_SUB_Pro=0;   
            break;
          }
        }   
        
        BATT_LOW_LEVEL_1_Warning(); 
      }
    }
    break;
    /*Ѳ��ģʽʱ��û�м�⵽����*/
  case AGV_STATUS_OFF_LINE:
    {
      static u8 reset;
      if(AGV_RUN_SUB_Pro==0)
      {
        reset = 1;
        AGV_Delay = 1500;
        LED_DISPLAY_Reset();
        AGV_RUN_SUB_Pro+=1;      
      }
      else
      {
        //
        LED_BARRIER_Display(1000);//�Ƶ���˸
        SetBeepForever(500,1500);//������
        
        //1��ʱ���ȼ���ɲ����������**
        SLOW_DOWN_Task(&reset,700);   
        
        //ң��ON��ť���½��룬ң��״̬
        if(REMOTE_SelectFlag)
        {
          AGV_RUN_Pro=AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro=0;
          break;
        }   
        
        //��ͣ��ť���£����뼱ͣģʽ
        if(BUTTON_IM_STOP_Flag)
        {
          AGV_RUN_Pro=AGV_STATUS_IM_STOP;
          AGV_RUN_SUB_Pro=0;    
          break;
        }
        
        //�͵�ѹ״̬�л�
        if((BatteryVolt_LowFlag>=2)&&(AGV_Delay==0))
        {
          MOTO_IM_STOP();
          AGV_RUN_Pro=AGV_STATUS_LOW_POWER;
          AGV_RUN_SUB_Pro=0;
          break;
        }        
        
        //Ѳ�߰�ť����&&������⵽����
        //if((MOD_BUS_Reg.M_CONTROL_MODE==M_CONTROL_MODE_FOLLOW_LINE)
        //   &&(MOD_BUS_Reg.AUTO_FOLLOW_ENABLE) 
        //   &&(ON_LINE_Flag)
        //     &&(BUTTON_FOLLOW_LINE_AND_PROGRAM_Flag)) 
        if((ON_LINE_Flag)&&(AGV_Delay==0))
        {
          AGV_RUN_Pro=AGV_STATUS_IDLE;
          AGV_RUN_SUB_Pro=0;
          break;
        }           
        
        BATT_LOW_LEVEL_1_Warning(); 
      }
    }
    break;
  /*----------RFID�ȴ�ģʽ----------*/  
  case AGV_STATUS_RFID_COMEIN:
    {
      static u8 reset=0;
      if(AGV_RUN_SUB_Pro==0)
      {
        reset = 1;
        AGV_Delay=1500;
        //SetBeep(2,200,500);  
        LED_DISPLAY_Reset();
        AGV_RUN_SUB_Pro+=1;
      }
      else
      {
        LED_RFID_Display(500);
        
        switch(AGV_RUN_SUB_Pro)
        {
        case 1:
          {
            if(AGV_Delay != 0)
            {
              SLOW_DOWN_Task(&reset,1000);  
            }
            else
            {
              AGV_Delay = 5000; // 5000
              AGV_RUN_SUB_Pro+=1;
            }
          }
          break;
        case 2: // �ȴ� ns
          {
            if(AGV_Delay != 0)
            {
               
            }
            else
            {
              if((PlaceId == 0x0004) || (PlaceId == 0x0005))
              {
                AGV_Delay = 10000;
                AGV_RUN_SUB_Pro+=1;                
              }
              else
              {
                printf("-- 180 --\n");
                VehicleTurnRound(180);
                AGV_Delay = 10000;
                AGV_RUN_SUB_Pro+=1;
              }
              
              PlaceId = 0;
            }
          }          
          break;
        case 3:
          {
            if(AGV_Delay != 0)
            {
              AGV_USER_PROGRAM_IN_DISPLACEMENT_Task(&reset); 
            }
            else
            {
              AGV_Delay = 5000;
              AGV_RUN_SUB_Pro+=1;
            }
          }          
          break;
        case 4: // �ȴ� ns
          {
                     
          }
          break;
        }
        
        
        // ң��ON��ť���½��룬ң��״̬
        if((REMOTE_SelectFlag)&&(BUTTON_IM_STOP_Flag==0))
        {
          AGV_RUN_Pro=AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro=0;
          break;
        }
        
        //60s��ص�����
        if(AGV_Delay==0)
        { 
          RFID_STOP_ANGIN_Timeout = 10000;
          AGV_Delay=100;
          LED_DISPLAY_Reset();
          AGV_RUN_Pro=AGV_STATUS_IDLE;
          AGV_RUN_SUB_Pro=0;  
          break;
        }        
      }
    }
    break;
  /*----------ң��ģʽ---------------*/    
  case AGV_STATUS_REMOTE:
    {
      if(AGV_RUN_SUB_Pro==0)
      {
        LED_DISPLAY_Reset();
        AGV_RUN_SUB_Pro+=1;
        AGV_Delay = 100;
      }
      else
      {
        LED_WATER_Display(300);

        REMOTE_Task();
        
        //��ͣ��ť���£����뼱ͣģʽ
        if(BUTTON_IM_STOP_Flag)
        {
          AGV_RUN_Pro=AGV_STATUS_IM_STOP;
          AGV_RUN_SUB_Pro=0;  
          break;
        }
        
        //�������ģʽ
        if(REMOTE_SelectFlag==0)
        {
          AGV_Delay=1500;
          LED_DISPLAY_Reset();
          AGV_RUN_Pro=AGV_STATUS_IDLE;
          AGV_RUN_SUB_Pro=0;  
          break;
        }         
        
        BATT_LOW_LEVEL_1_Warning(); 
      }
    }
    break;
  /*----------�͵�ѹģʽ---------------*/     
  case AGV_STATUS_LOW_POWER:
    {
      if(AGV_RUN_SUB_Pro==0)
      {
        AGV_Delay=1500;
        LED_DISPLAY_Reset();
        AGV_RUN_SUB_Pro+=1;
      }
      else
      {
        LED_LOW_POWER_Display(500);
        //SetBeepForever(300,700);
        if(AGV_Delay==0)
        {
          AGV_Delay = 20000;//60s����һ��
          Play_Warning(BAT_LOW_20P);
        }
        
        //ң��ON��ť���½��룬ң��״̬
        if((REMOTE_SelectFlag)&&(BUTTON_IM_STOP_Flag==0))
        {
          AGV_RUN_Pro=AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro=0;
          break;
        }

        if(BatteryVolt_LowFlag<2)
        {
          AGV_Delay=1500;
          LED_DISPLAY_Reset();
          AGV_RUN_Pro=AGV_STATUS_IDLE;
          AGV_RUN_SUB_Pro=0;  
          break;
        }        
      }
    }
    break;
  /*----------����ģʽ---------------*/     
  case AGV_STATUS_BARRIER:
    {
      static u8 slow_flag=0;
      static u8 reset = 0;
      if(AGV_RUN_SUB_Pro==0)
      {
        reset = 1;
        if((TOUCH_SENSOR_Flag & (1<<((Run_Dir==DIR_FORWARD)?0:1)))!=0)
        {
          MOTO_IM_STOP();
          slow_flag=0;
        }
        else 
        {
          slow_flag=1;
        }
        LoopDetectThing_time_out = DEFAULT_PLAY_DETECT_THING_TIME_IN_MS;
        AGV_Delay = 2000;
        LED_DISPLAY_Reset();
        AGV_RUN_SUB_Pro++;
      }
      else
      {
        LED_BARRIER_Display(500);
        
        if(slow_flag)
        {
          SLOW_DOWN_Task(&reset,700); 
        }
        
        //ң��ON��ť���½��룬ң��״̬
        if((REMOTE_SelectFlag)&&(BUTTON_IM_STOP_Flag==0))
        {
          AGV_RUN_Pro=AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro=0;
          break;
        } 
        
        //��ͣ��ť���£����뼱ͣģʽ
        if(BUTTON_IM_STOP_Flag)
        {
          AGV_RUN_Pro=AGV_STATUS_IM_STOP;
          AGV_RUN_SUB_Pro=0;    
          break;
        }        
        
        //û���ϰ���󣬽������״̬
        if(((TOUCH_SENSOR_Flag & (1<<((Run_Dir==DIR_FORWARD)?0:1)))==0)
            && ((LASER_SENSOR_Flag & (1<<((Run_Dir==DIR_FORWARD)?0:1)))==0)
            && (AGV_Delay==0))
        {
          AGV_Delay=2400;
          LED_DISPLAY_Reset();
          AGV_RUN_Pro=AGV_STATUS_FOLLOWLINE;//AGV_STATUS_IDLE;
          AGV_RUN_SUB_Pro=0;    
          break;
        }
        
        if(LoopDetectThing_time_out==0)
        {
          LoopDetectThing_time_out = DEFAULT_PLAY_DETECT_THING_TIME_IN_MS;
          Play_Warning(DETECT_TING);
        }
      }
    }
    break;
  /*----------����ֹͣģʽ---------------*/   
  case AGV_STATUS_IM_STOP:
    {
      if(AGV_RUN_SUB_Pro==0)
      {
        SetBeep(1,1000,200);
        MOTO_IM_STOP();
        LED_DISPLAY_Reset();
        AGV_RUN_SUB_Pro++;
      }
      else
      {
        LED_IM_STOP_Display(500);
        
        //��ͣ��ť�ɿ����������ģʽ
        if(BUTTON_IM_STOP_Flag==0)
        {
          SetBeep(3,300,500);
          AGV_Delay=2400;
          LED_DISPLAY_Reset();
          AGV_RUN_Pro=AGV_STATUS_IDLE;
          AGV_RUN_SUB_Pro=0;    
          break;
        }
        
        BATT_LOW_LEVEL_1_Warning(); 
      }
    }
    break;
  case AGV_STATUS_CHARGE: //���
    {
      static u8 reset;
      if(AGV_RUN_SUB_Pro==0)
      {
        reset = 1;
        LED_DISPLAY_Reset();
        AGV_RUN_SUB_Pro+=1;
        AGV_Delay=2500;
      }
      else
      { 
        switch(AGV_RUN_SUB_Pro)
        {
        case 1:
          {
            if(AGV_Delay != 0)
            {
              SLOW_DOWN_Task(&reset, 1000); 
            }
            else
            {
              FollowLineEnable = 0;
              AGV_RUN_SUB_Pro = 0;
              AGV_RUN_Pro = AGV_STATUS_IDLE;            
            }
          }
          break;
        case 2:
          {
            if(AGV_Delay==0)
            {
            }
          }
          break;      
        case 3:
          { //������,С�ڶ���������������

          }
          break;
        case 4:
          {
            if(AGV_Delay==0)
            {              
              //�Ͽ����̵���
              AGV_Delay=4000;
              AGV_RUN_SUB_Pro=0;
              AGV_RUN_Pro = AGV_STATUS_IDLE;
            }        
          }
          break;
        }
      }
    }
    break;    
  }
  //��¼λ��
  //ROAD_RECORD_Task();
}

void BATT_LOW_LEVEL_1_Warning(void)
{
  static u32 counter=0;
  static u32 NumOfSysTickIntBk;
  if(NumOfSysTickInt!=NumOfSysTickIntBk)
  {
    NumOfSysTickIntBk=NumOfSysTickInt;
  }
  else return;  
  
  if(BatteryVolt_LowFlag==1)
  {
    counter++;
    if(counter>=300000)//5����
    {
      counter=0;
      Play_Warning(BAT_LOW_20P);
      //SetBeep(5,300,700);//��5��
    }
  }
  else
  {
    counter=0;
  }
}

void ROUND_SpeedLimted(void)
{
  //ת������ ,������
  if(Get_ANALOG_SD_Speed() > FOLLOW_LINE_ROUND_SPEED)
  {
    if(Run_Dir>0)
    {
      if(Current_ID==(ROUND_LOW_ID<<1))
      {
        Set_FollowLineTempBaseSpeed(FOLLOW_LINE_ROUND_SPEED);
      }
      else if(Current_ID==(ROUND_HIGH_ID<<1))
      {
        Clear_FollowLineTempBaseSpeed();
      }
      else if((Current_ID>(ROUND_LOW_ID<<1)) && (Current_ID<(ROUND_HIGH_ID<<1)))
      {
        Set_FollowLineTempBaseSpeed(FOLLOW_LINE_ROUND_SPEED);
      }
    }
    else
    {
      if(Current_ID==(ROUND_LOW_ID<<1))
      {
        Clear_FollowLineTempBaseSpeed();
      }
      else if(Current_ID==(ROUND_HIGH_ID<<1))
      {
        Set_FollowLineTempBaseSpeed(FOLLOW_LINE_ROUND_SPEED);
      }
      else if((Current_ID>(ROUND_LOW_ID<<1)) && (Current_ID<(ROUND_HIGH_ID<<1)))
      {
        Set_FollowLineTempBaseSpeed(FOLLOW_LINE_ROUND_SPEED);
      }
    }
  }
}


void INIT_SpeedLimted(void)
{
  //��ʼ������ ,������
  if(Get_ANALOG_SD_Speed() > FOLLOW_LINE_ROUND_SPEED)
  {
    Set_FollowLineTempBaseSpeed(FOLLOW_LINE_ROUND_SPEED);
  }
}

/************************************************************
 ** ���ܣ��Ӽ��ٹ��̳�ʼ��---
 ** ������accelerated_speed_cmps  - ���ٶȣ���λcm/s
 **       max_speed_cmps          - ����ٶȣ���λcm/s
 **       cycle_time              - ����ĵ���ʱ�����ڣ���λs
*************************************************************/
void SPEED_UP_DOWN_STRUCT_Init(float accelerated_speed_cmps,float max_speed_cmps,float cycle_time,SPEED_UP_OPTION* pSPEED)
{
  int i;
  float speed=0;
  float time=0;
  float disp=0;
  for(i=0;i<MAX_SPEED_UP_LIST_LENGTH;i++)
  {
    time+=cycle_time;
    speed+=(accelerated_speed_cmps*cycle_time);
    disp+=cycle_time*speed;
    pSPEED[i].total_time=time;
    pSPEED[i].current_speed=speed;
    pSPEED[i].total_disp=disp;
    if(speed>=max_speed_cmps) 
    {
      i+=1;
      break;
    }
  }
  SPEED_UP_Length=i;
}

// ���㳵��λ�����̵ļӼ��ٹ���
void Caculate_DisplacmentProcess(MOVEMENT_OPTION* pM,SPEED_OPTION_LIST* pS,u8 coff_enable,SPEED_UP_OPTION* pSPEED)
{
  u32 total_displacement=pM->value;
  u32 dir=pM->dir;
  u32 index=0,i;
  //�ǶȺ�λ��ģʽ������أ�20160921
  //Wonder_Disp_or_Angle_value=total_displacement;//
  //Finish_Disp_or_Angle_value=0;  
  
  if(coff_enable)
  {
    //λ�Ʋ�����20160813
    total_displacement=(u32)(((float)total_displacement)*Displacement_coff);
  }
  if(total_displacement<(pSPEED[0].total_disp*2))
  {
	;
  }
  else if(total_displacement>(pSPEED[SPEED_UP_Length-2].total_disp*2))
  {//����+����+����
    for(i=0;i<(SPEED_UP_Length-1);i++)
    {
      pS->buf[index].L_Speed=speed_to_pwm(dir?-pSPEED[i].current_speed:pSPEED[i].current_speed);//cm/s->pwm_persent
      pS->buf[index].R_Speed=pS->buf[index].L_Speed;
      pS->buf[index].repeat_times=100/DEFAULT_PROGRAM_CYCLE_IN_MS;
      index+=1;
    }
    {
      pS->buf[index].L_Speed=speed_to_pwm(dir?-pSPEED[SPEED_UP_Length-1].current_speed:pSPEED[SPEED_UP_Length-1].current_speed);;
      pS->buf[index].R_Speed=pS->buf[index].L_Speed;
      pS->buf[index].repeat_times=(total_displacement-(pSPEED[SPEED_UP_Length-2].total_disp*2))*(1000/DEFAULT_PROGRAM_CYCLE_IN_MS)/pSPEED[SPEED_UP_Length-1].current_speed;
      index+=1;    
    }
    for(i=0;i<(SPEED_UP_Length-1);i++)
    {
      pS->buf[index].L_Speed=speed_to_pwm(dir?-pSPEED[(SPEED_UP_Length-2)-i].current_speed:pSPEED[(SPEED_UP_Length-2)-i].current_speed);//cm/s->pwm_persent
      pS->buf[index].R_Speed=pS->buf[index].L_Speed;
      pS->buf[index].repeat_times=100/DEFAULT_PROGRAM_CYCLE_IN_MS;
      index+=1;
    }    
  }
  else
  {//����-����
    u32 speed_up_step;
    for(i=1;i<SPEED_UP_Length;i++)
    {
      if(total_displacement<(pSPEED[i].total_disp*2)) break;
    }
    speed_up_step=i;
    
    for(i=0;i<speed_up_step;i++)
    {
      pS->buf[index].L_Speed=speed_to_pwm(dir?-pSPEED[i].current_speed:pSPEED[i].current_speed);//cm/s->pwm_persent
      pS->buf[index].R_Speed=pS->buf[index].L_Speed;
      pS->buf[index].repeat_times=100/DEFAULT_PROGRAM_CYCLE_IN_MS;
      index+=1;
    }
    
    {
      pS->buf[index].L_Speed=speed_to_pwm(dir?-pSPEED[speed_up_step-1].current_speed:pSPEED[speed_up_step-1].current_speed);;
      pS->buf[index].R_Speed=pS->buf[index].L_Speed;
      pS->buf[index].repeat_times=(total_displacement-(pSPEED[speed_up_step-1].total_disp*2))*(1000/DEFAULT_PROGRAM_CYCLE_IN_MS)/pSPEED[speed_up_step-1].current_speed;
      index+=1;    
    }

    for(i=0;i<speed_up_step;i++)
    {
      pS->buf[index].L_Speed=speed_to_pwm(dir?-pSPEED[(speed_up_step-1)-i].current_speed:pSPEED[(speed_up_step-1)-i].current_speed);//cm/s->pwm_persent
      pS->buf[index].R_Speed=pS->buf[index].L_Speed;
      pS->buf[index].repeat_times=100/DEFAULT_PROGRAM_CYCLE_IN_MS;
      index+=1;
    }
  
  }

  {
    pS->buf[index].L_Speed=0;		
    pS->buf[index].R_Speed=pS->buf[index].L_Speed;
    pS->buf[index].repeat_times=10;
    index+=1;
  }
    
  pS->InIndex=index;
  pS->OutIndex=0;
}

s16 speed_to_pwm(float speed_in_cmps)
{
  return (s16)(speed_in_cmps*FULL_SPEED_STEP/MAX_WHEEL_RUN_LENGTH_IN_CM_PER_SECOND);
}

//���㳵��λ�����̵ļӼ��ٹ���
void Caculate_AngleProcess(MOVEMENT_OPTION* pM,SPEED_OPTION_LIST* pS)
{
  u8 i;
  u32 total_angle=pM->value;
  u32 dir=pM->dir;  
  MOVEMENT_OPTION temp;
  float disp;
  disp=(3.14*VehicleWidth*total_angle*Angle_coff)/360;//180
  temp.dir=0;
  temp.value=disp;
  
  Caculate_DisplacmentProcess(&temp,pS,0,SPEED_UP_OPTION_List[CircleRun]);

  if(dir==0)//������ʱ��
  {
    for(i=0;i<pS->InIndex;i++)
    {
      pS->buf[i].L_Speed=-pS->buf[i].L_Speed;
    }
  }
  else //˳ʱ��
  {
    for(i=0;i<pS->InIndex;i++) 
    {
      pS->buf[i].R_Speed=-pS->buf[i].R_Speed;
    }
  }
}

void AGV_USER_PROGRAM_IN_DISPLACEMENT_Task(u8* pReset)
{
  if(*pReset)
  {
    *pReset=0;
  }
  //ִ�л�����������ײ�����PWM  
  if(ProgramControlCycle==0)
  {
    ProgramControlCycle=DEFAULT_PROGRAM_CYCLE_IN_MS;
    if(SPEED_OPTION_List.InIndex!=SPEED_OPTION_List.OutIndex)
    {
      if(SPEED_OPTION_List.buf[SPEED_OPTION_List.OutIndex].repeat_times!=0)
      {
        s16 LP=SPEED_OPTION_List.buf[SPEED_OPTION_List.OutIndex].L_Speed;
        s16 RP=SPEED_OPTION_List.buf[SPEED_OPTION_List.OutIndex].R_Speed;
      
        SetD1Rpm(LEFT_MOTO_INDEX, (float)LP*0.001*(float)MAX_MOTO_SPEED_IN_D1RPM);
        SetD1Rpm(RIGHT_MOTO_INDEX, (float)RP*0.001*(float)MAX_MOTO_SPEED_IN_D1RPM);

        SPEED_OPTION_List.buf[SPEED_OPTION_List.OutIndex].repeat_times-=1;
        if(SPEED_OPTION_List.buf[SPEED_OPTION_List.OutIndex].repeat_times==0)
        {
          SPEED_OPTION_List.OutIndex+=1;
        }

      }
      else
      {
        SPEED_OPTION_List.OutIndex+=1;        
      }
    }
    else
    {//ִ�����

    }
  }
  
  //����λ�Ʊ�̵�ִ������
  if(SPEED_OPTION_List.InIndex==SPEED_OPTION_List.OutIndex)
  {
    if(DISPLACEMENT_MOVEMENT_OPTION_List.In_index!=DISPLACEMENT_MOVEMENT_OPTION_List.Out_index)
    {
      MOVEMENT_OPTION* pM=&DISPLACEMENT_MOVEMENT_OPTION_List.buf[DISPLACEMENT_MOVEMENT_OPTION_List.Out_index&LIST_LENGTH_MASK];
      Caculate_DisplacmentProcess(pM,&SPEED_OPTION_List,1,SPEED_UP_OPTION_List[DirectRun]);
      DISPLACEMENT_MOVEMENT_OPTION_List.Out_index+=1;
    }
  }
  
  //����Ƕȱ�̵�ִ������
  if(SPEED_OPTION_List.InIndex==SPEED_OPTION_List.OutIndex)
  {
    if(ANGLE_MOVEMENT_OPTION_List.In_index!=ANGLE_MOVEMENT_OPTION_List.Out_index)
    {
      MOVEMENT_OPTION* pM=&ANGLE_MOVEMENT_OPTION_List.buf[ANGLE_MOVEMENT_OPTION_List.Out_index&LIST_LENGTH_MASK];
      Caculate_AngleProcess(
           pM,&SPEED_OPTION_List);    
      ANGLE_MOVEMENT_OPTION_List.Out_index+=1;
    }
  }   
}

// ˳ʱ��Ϊ��������ʱ��Ϊ����
void VehicleTurnRound(s16 value)
{
  MOVEMENT_OPTION* pM=
        &ANGLE_MOVEMENT_OPTION_List.buf[ANGLE_MOVEMENT_OPTION_List.In_index&LIST_LENGTH_MASK];
  if(value < 0) pM->dir = 0;
  else pM->dir = 1;
  pM->value = abs_32(value);
  ANGLE_MOVEMENT_OPTION_List.In_index += 1;
}





