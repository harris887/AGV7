#include "user_inc.h"
#include "string.h"




AGV_STATUS_LIST AGV_RUN_Pro=AGV_STATUS_INIT;
u8 AGV_RUN_SUB_Pro=0;
u32 AGV_Delay=3500;
u16 RFID_STOP_ANGIN_Timeout=0;

u16 ProgramControlCycle=0;
//float VehicleWidth=DEFAULT_Vehicle_WIDTH_FLOAT;
float Displacement_coff=1.0;
float Angle_coff=1.0;

u16 Current_ID=0;
u16 Dest_ID=0;
s16 Run_Dir=DIR_FORWARD;
u16 LoopDetectThing_time_out = DEFAULT_PLAY_DETECT_THING_TIME_IN_MS;

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
        Play_Warning(SELF_TEST);
        AGV_Delay = voice_all[SELF_TEST].last_time + 2000;
        
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
        if((ON_LINE_Flag)&&(Current_ID==0))
        {
          MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR ;
          Run_Dir=DIR_FORWARD;
          INIT_SpeedLimted();
          AGV_RUN_Pro=AGV_STATUS_FOLLOWLINE;
          AGV_RUN_SUB_Pro=0;
          break;
        }
        
        if(remote_fresh)
        {
          remote_fresh = 0;
          if((remote_value>=MIN_DEST_ID)&&(remote_value<=MAX_DEST_ID))
          {
            Dest_ID = remote_value<<1;
            if((ON_LINE_Flag)&&(Dest_ID != Current_ID))
            {
              Play_Warning(TO_1_PLACE + (Dest_ID>>1) - MIN_DEST_ID);
              //SetBeep(6,100,100);//test
              if(Dest_ID > Current_ID)
              {
                MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR ;
                Run_Dir = DIR_FORWARD;
              }
              else
              {
                MODE_BUS_HALL_Addr = BACKWARD_MODE_BUS_HALL_ADDR ;
                Run_Dir = DIR_BACKWARD;
              }
              ROUND_SpeedLimted();
              AGV_RUN_Pro=AGV_STATUS_FOLLOWLINE;
              AGV_RUN_SUB_Pro=0;
              break;
            }
          }
        }
    
    
        
        //���ڴ���ʱ�������뿪����ģʽ
        //if((MOD_BUS_Reg.M_CONTROL_MODE==M_CONTROL_MODE_FOLLOW_LINE)
        //   &&(MOD_BUS_Reg.AUTO_FOLLOW_ENABLE) 
        //   &&(ON_LINE_Flag==0)
        //     &&(BUTTON_FOLLOW_LINE_AND_PROGRAM_Flag)) 
        //{
        //  AGV_RUN_Pro=AGV_STATUS_OFF_LINE;
        //  AGV_RUN_SUB_Pro=0;
        //  break;
        //}        
        
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
        
        if(RFID_COMEIN_Flag & 0x4)
        {
          RFID_COMEIN_Flag=0;
          Current_ID = PlaceId<<1;
          
          if(Dest_ID == 0)
          {
            Clear_FollowLineTempBaseSpeed();//�����ʼ������ʱ�ٶ�
          }
          
          if((Current_ID == Dest_ID) || (Dest_ID == 0))
          {
            Play_Warning(AT_1_PLACE + (Current_ID>>1) - MIN_DEST_ID);
            AGV_Delay = voice_all[AT_1_PLACE + (Current_ID>>1) - MIN_DEST_ID].last_time;
            AGV_RUN_Pro=AGV_STATUS_RFID_COMEIN;
            AGV_RUN_SUB_Pro=0;  
            break;
          }
          ROUND_SpeedLimted();
        }     
        
        NEW_FOLLOW_LINE_TASK(&FollowLineReset,Run_Dir);
        
        //��ͣ��ť���£����뼱ͣģʽ
        if(BUTTON_IM_STOP_Flag)
        {
          AGV_RUN_Pro=AGV_STATUS_IM_STOP;
          AGV_RUN_SUB_Pro=0;    
          break;
        }
        
        //����ֹͣ����
        if(remote_fresh)
        {
          remote_fresh = 0;
          if((remote_value > MAX_DEST_ID) && (Current_ID != 0))
          {
            Play_Warning(CANCLE_TO_X_PLACE);
            if(Run_Dir == DIR_FORWARD) 
            {
              Current_ID = (Current_ID&(~1)) + 1;
            }
            else 
            {
              Current_ID = (Current_ID&(~1)) - 1;
            }
            AGV_RUN_Pro=AGV_STATUS_OFF_LINE;
            AGV_RUN_SUB_Pro=0;
            break;
          }
        }
        
        //���£�����
        //if(TOUCH_SENSOR_Flag&TOUCH_SENSOR_MASK)
        if(((TOUCH_SENSOR_Flag & (1<<((Run_Dir==DIR_FORWARD)?0:1)))!=0)
          || ((TOUCH_SENSOR_Flag & (3<<((Run_Dir==DIR_FORWARD)?2:4)))!=0))        
        {
          Play_Warning(DETECT_TING);
          AGV_RUN_Pro=AGV_STATUS_BARRIER;
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
          break;
        }

        //�͵�ѹ״̬�л�
        if(BatteryVolt_LowFlag>=2)
        {
          MOTO_IM_STOP();
          AGV_RUN_Pro=AGV_STATUS_LOW_POWER;
          AGV_RUN_SUB_Pro=0;
          break;
        }
        
        //����RFID
        //if((RFID_COMEIN_Flag&1)&&(RFID_ReadBlockDelay==0))
        //if(RFID_COMEIN_Flag&1)  
        //{
        //  RFID_COMEIN_Flag&=~1;
        //  if((RFID_STOP_ANGIN_Timeout==0)&&(MOD_BUS_Reg.RFID_WAIT_TIME_IN_MS!=0))
        //  {
        //    AGV_RUN_Pro=AGV_STATUS_RFID_COMEIN;
        //    AGV_RUN_SUB_Pro=0;   
        //    break;
        //  }
        //}   
        
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
        //AGV_Delay=1500;
        //SetBeep(2,200,500);
        LED_DISPLAY_Reset();
        AGV_RUN_SUB_Pro+=1;
      }
      else
      {
        LED_RFID_Display(500);
        
        //1��ʱ���ȼ���ɲ����������**
        SLOW_DOWN_Task(&reset,1000);        
        
        //ң��ON��ť���½��룬ң��״̬
        if((REMOTE_SelectFlag)&&(BUTTON_IM_STOP_Flag==0))
        {
          AGV_RUN_Pro=AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro=0;
          break;
        }
        
        //60s��ص�����
        if(AGV_Delay==0)
        {
          //RFID ��ȡʧ�ܱ���
          //if(RFID_ReadBlockSuccessTimes!=RFID_ReadBlockTimes)
          //{
          //  RFID_ReadBlockTimes=RFID_ReadBlockSuccessTimes;
          //  SetBeep(5,150,150);
          //}
          
          RFID_STOP_ANGIN_Timeout=5000;
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
          Play_Warning(BAT_LOW_10P);
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
        //if((TOUCH_SENSOR_Flag&TOUCH_SENSOR_MASK)==0)
        if(((TOUCH_SENSOR_Flag & (1<<((Run_Dir==DIR_FORWARD)?0:1)))==0)
          && ((TOUCH_SENSOR_Flag & (3<<((Run_Dir==DIR_FORWARD)?2:4)))==0)
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
