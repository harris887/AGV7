#include "user_inc.h"
#include "string.h"

AGV_STATUS_LIST AGV_RUN_Pro=AGV_STATUS_INIT;
u8 AGV_RUN_SUB_Pro=0;
u32 AGV_Delay = 3500;
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
u32 CHARGE_MinCycle = 0;
u8 FORCE_CHARGE_Flag = 0;
u8 FORCE_CHARGE_STOP_Flag = 0;
u32 FOLLOW_LOOP_Timeout = 0;

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
  /*----------初始化模式---------------*/    
  case AGV_STATUS_INIT:
    {
      if(AGV_Delay == 0)
      {
        //SetBeep(3,300,500);
        Play_Warning(SELF_TEST_OK);
        AGV_Delay = voice_all[SELF_TEST_OK].last_time + 2000;
        
        LED_DISPLAY_Reset();  
        AGV_RUN_Pro = AGV_STATUS_IDLE;
        AGV_RUN_SUB_Pro = 0;
      }
    }
    break;
  /*----------空闲模式---------------*/    
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
        
        //急停按钮按下，进入急停模式
        if(BUTTON_IM_STOP_Flag)
        {
          AGV_RUN_Pro = AGV_STATUS_IM_STOP;
          AGV_RUN_SUB_Pro = 0;    
          break;
        }       
        
        //巡线按钮按下&&霍尔检测到磁条
        if((ON_LINE_Flag) && (FollowLineEnable))
        {
          if(FollowLineEnable == 1)
          {
            FollowLineEnable += 1;
            Play_Warning(AUTO_FOLLOW_LINE);
          }
          AGV_RUN_Pro = AGV_STATUS_FOLLOWLINE;
          AGV_RUN_SUB_Pro = 0;
          break;
        }   
        
        //遥控ON按钮按下进入，遥控状态
        if(REMOTE_SelectFlag)
        {
          AGV_RUN_Pro = AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro = 0;
          break;
        }

        //低电压状态切换
        if(((BatteryVolt_LowFlag >= 2) || (FORCE_CHARGE_Flag)) 
           && (ON_LINE_Flag) 
           && (FollowLineEnable == 0)
           && (CHARGE_MinCycle == 0))
        {
          if(FORCE_CHARGE_Flag) FORCE_CHARGE_Flag -= 1; // test charge 
          CHARGE_MinCycle = DEFAULT_MIN_CHARGE_CYCLE_IN_S * 1000;
          AGV_RUN_Pro = AGV_STATUS_CHARGE;
          AGV_RUN_SUB_Pro = 0;
          break;
        }        
        
        BATT_LOW_LEVEL_1_Warning();        
      }     
    }
    break;
  /*----------巡线模式---------------*/      
  case AGV_STATUS_FOLLOWLINE:
    {
      static u8 FollowLineReset=0;
      static RFID_ACTION* pRFID_ACTION;
      static u16 ActionIndex = 0;
      static u16 MotoModifyFlag = 0;
      static u16 LastRfid = 0;
      static u16 IM_STOP_Ignore = 0;
      static u16 BARRIER_Ignore = 0;
      static u16 OFF_LINE_Ignore = 0;      
      if(AGV_RUN_SUB_Pro==0)
      {
        if(AGV_Delay==0)
        {
          PID_Init();
          LED_DISPLAY_Reset();
          AGV_RUN_SUB_Pro += 1;
          RFID_COMEIN_Flag &= ~1;
          FollowLineReset = 1;
          ActionIndex = 0;
          MotoModifyFlag = 0;
          LastRfid = 0;
          IM_STOP_Ignore = 0;
          BARRIER_Ignore = 0;
          OFF_LINE_Ignore = 0;                
        }
      }
      else
      {      
        LED_FOLLOW_LINE_Display(500);
 
#define ACTION_PRO_OFFSET    3         
        // 巡线 + RFID 响应
        switch(AGV_RUN_SUB_Pro) 
        {
        case 1:
          {
            NEW_FOLLOW_LINE_TASK(&FollowLineReset, Run_Dir);
            if(RFID_COMEIN_Flag & RFID_READ_INFOR_SUCCESS_MASK) 
            {
              RFID_COMEIN_Flag &= ~RFID_READ_INFOR_SUCCESS_MASK;
              printf("-- RFID_COMEIN_Flag %04X--\n", PlaceId);
              if(LastRfid != PlaceId)
              {
                if(RoadType == ROAD_TYPE_FORWARD) pRFID_ACTION = (RFID_ACTION*)FORWARD_RFID_Action;
                else pRFID_ACTION = (RFID_ACTION*)BACKWARD_RFID_Action;
                for(u8 i = 0; i < FUNC_1_RFID_CARD_NUM; i++)
                {
                  if((pRFID_ACTION[i].Id == PlaceId) 
                     && (pRFID_ACTION[i].ActionNum != 0))
                  {
                    pRFID_ACTION = pRFID_ACTION + i;
                    ActionIndex = 0;
                    AGV_RUN_SUB_Pro = 2;
                    MotoModifyFlag = 0;
                    IM_STOP_Ignore = 1;
                    BARRIER_Ignore = 1;
                    OFF_LINE_Ignore = 1;
                    printf("ActionNum = %d\n", pRFID_ACTION->ActionNum);
                    break;
                  }
                }
              }
              else
              {
                printf("BYPASS %04X\n", PlaceId);
              }
            }            
          }
          break;
        case 2: // RFID動作解析
          {
            if(ActionIndex >= pRFID_ACTION->ActionNum)
            {
              if(MotoModifyFlag) FollowLineReset = 1;
              else FollowLineReset = 0;
              LastRfid = PlaceId;
              PlaceId = 0;
              AGV_RUN_SUB_Pro = 1;
              IM_STOP_Ignore = 0;
              BARRIER_Ignore = 0;
              OFF_LINE_Ignore = 0;
            }
            else if(pRFID_ACTION->ActionInfor[ActionIndex].ActionType == ACTION_BRAKE) // 刹车
            {
              FollowLineReset = 1;
              AGV_Delay = 1800;
              AGV_RUN_SUB_Pro = (ACTION_PRO_OFFSET + ACTION_BRAKE);
            }
            else if(pRFID_ACTION->ActionInfor[ActionIndex].ActionType == ACTION_TURN_ANGLE) // 转弯
            {
              FollowLineReset = 1;
              VehicleTurnRound(pRFID_ACTION->ActionInfor[ActionIndex].value);
              AGV_Delay = 10000;      
              AGV_RUN_SUB_Pro = (ACTION_PRO_OFFSET + ACTION_TURN_ANGLE);
            }
            else if(pRFID_ACTION->ActionInfor[ActionIndex].ActionType == ACTION_WAIT)       // 等待
            {
              //AGV_Delay = 1000 * pRFID_ACTION->ActionInfor[ActionIndex].value;    
              AGV_Delay = MOD_BUS_Reg.RFID_WAIT_TIME_IN_MS;
              AGV_RUN_SUB_Pro = (ACTION_PRO_OFFSET + ACTION_WAIT);
            }
            else if(pRFID_ACTION->ActionInfor[ActionIndex].ActionType == ACTION_SET_LINE_TYPE)
            {
              RoadType = pRFID_ACTION->ActionInfor[ActionIndex].value;
              ActionIndex += 1;
              printf("++RoadType = %d\n", RoadType);
            }
            else if(pRFID_ACTION->ActionInfor[ActionIndex].ActionType == ACTION_SET_BRANCH_DIR)
            {
              MB_LINE_DIR_SELECT = pRFID_ACTION->ActionInfor[ActionIndex].value; 
              ActionIndex += 1;
              printf("++MB_LINE_DIR_SELECT = %d\n", MB_LINE_DIR_SELECT);
            }
            else if(pRFID_ACTION->ActionInfor[ActionIndex].ActionType == ACTION_SET_VEHICLE_DIR)
            {
              Run_Dir = pRFID_ACTION->ActionInfor[ActionIndex].value;
              if(Run_Dir == DIR_BACKWARD) 
              {
                MODE_BUS_HALL_Addr = BACKWARD_MODE_BUS_HALL_ADDR;
              }
              else
              {
                MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR;
              }
              ActionIndex += 1;
              printf("++Run_Dir = %d\n", Run_Dir);
            }
            else if(pRFID_ACTION->ActionInfor[ActionIndex].ActionType == ACTION_FINISH)
            {
              FollowLineEnable = 0;
              if(MOD_BUS_Reg.M_CONTROL_MODE == M_CONTROL_MODE_FOLLOW_LINE)
              {
                MOD_BUS_Reg.M_CONTROL_MODE = M_CONTROL_MODE_IDLE;
              }
              if(BatteryVolt_LowFlag >= 2)
              {
                AGV_RUN_Pro = AGV_STATUS_CHARGE;
                AGV_RUN_SUB_Pro = 0;                  
              }
              else
              {
                AGV_RUN_Pro = AGV_STATUS_IDLE;
                AGV_RUN_SUB_Pro = 0;  
              }
            }
          }
          break;
        case (ACTION_PRO_OFFSET + ACTION_BRAKE): // 刹车
          {
            if(AGV_Delay != 0)
            {
              SLOW_DOWN_Task(&FollowLineReset, 1300); // 1000 -> 1400 -> 1200 -> 1300
            }
            else 
            {
              MotoModifyFlag = 1;
              ActionIndex += 1;
              AGV_RUN_SUB_Pro = 2;
            }
          }
          break; 
        case (ACTION_PRO_OFFSET + ACTION_TURN_ANGLE):
          {
            if(AGV_Delay != 0)
            {
              u32 run_flag = AGV_USER_PROGRAM_IN_DISPLACEMENT_Task(&FollowLineReset);
              if(run_flag == 0) AGV_Delay = 0;
            }
            else
            {
              MotoModifyFlag = 1;
              ActionIndex += 1;
              AGV_RUN_SUB_Pro = 2;
            }      
          }
          break;       
        case (ACTION_PRO_OFFSET + ACTION_WAIT):
          {
            if(AGV_Delay != 0)
            {
              // do nothing
            }
            else
            {
              ActionIndex += 1;
              AGV_RUN_SUB_Pro = 2;
            }  
          }
          break;  
        case (ACTION_PRO_OFFSET + ACTION_SET_LINE_TYPE):
          {
          }
          break;      
        case (ACTION_PRO_OFFSET + ACTION_SET_BRANCH_DIR):
          {
          }
          break;    
        case (ACTION_PRO_OFFSET + ACTION_SET_VEHICLE_DIR):
          {
          }
          break;       
        case (ACTION_PRO_OFFSET + ACTION_FINISH):
          {
          }
          break;            
        }
        
        // ---- 响应其他传感器 ----//
        //急停按钮按下，进入急停模式
        if((BUTTON_IM_STOP_Flag) && (IM_STOP_Ignore == 0))
        {
          AGV_RUN_Pro=AGV_STATUS_IM_STOP;
          AGV_RUN_SUB_Pro=0;    
          printf("-- BUTTON_IM_STOP_Flag --\n");
          break;
        }        
        
        //臂章，进入
        if((((TOUCH_SENSOR_Flag & (1 << ((Run_Dir == DIR_FORWARD)?0:1))) != 0)
          || ((LASER_SENSOR_Flag & (1 << ((Run_Dir == DIR_FORWARD)?0:1))) != 0))
          && (BARRIER_Ignore == 0))
        {
          Play_Warning(DETECT_TING);
          AGV_RUN_Pro = AGV_STATUS_BARRIER;
          AGV_RUN_SUB_Pro = 0;    
          printf("-- TOUCH_SENSOR_Flag --\n");
          break;
        }        
        
        //遥控ON按钮按下进入，遥控状态
        if(REMOTE_SelectFlag)
        {
          AGV_RUN_Pro = AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro = 0;
          FollowLineEnable = 0; // 结束巡线流程
          printf("-- REMOTE_SelectFlag --\n");
          break;
        }
        
        if((ON_LINE_Flag == 0) && (OFF_LINE_Ignore == 0))
        {
          AGV_RUN_Pro = AGV_STATUS_OFF_LINE;
          
          AGV_RUN_SUB_Pro = 0;  
          FollowLineEnable = 0; // 结束巡线流程
          printf("-- OFF_LINE_Flag --\n");
          break;
        }        

        BATT_LOW_LEVEL_1_Warning(); 
      }
    }
    break;
    /*巡线模式时，没有检测到磁条*/
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
        LED_BARRIER_Display(1000); //黄灯闪烁
        SetBeepForever(500,1500);  //警报音
        
        //1秒时间匀减速刹车，待测试**
        SLOW_DOWN_Task(&reset,700);   
        
        //遥控ON按钮按下进入，遥控状态
        if(REMOTE_SelectFlag)
        {
          AGV_RUN_Pro=AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro=0;
          break;
        }   
        
        //急停按钮按下，进入急停模式
        if(BUTTON_IM_STOP_Flag)
        {
          AGV_RUN_Pro = AGV_STATUS_IM_STOP;
          AGV_RUN_SUB_Pro = 0;    
          break;
        }
        
        //低电压状态切换
        if((BatteryVolt_LowFlag >= 2) && (AGV_Delay == 0))
        {
          MOTO_IM_STOP();
          AGV_RUN_Pro = AGV_STATUS_LOW_POWER;
          AGV_RUN_SUB_Pro = 0;
          break;
        }        
        
        //巡线按钮按下&&霍尔检测到磁条
        if((ON_LINE_Flag) && (AGV_Delay == 0))
        {
          AGV_RUN_Pro = AGV_STATUS_IDLE;
          AGV_RUN_SUB_Pro = 0;
          break;
        }           
        
        BATT_LOW_LEVEL_1_Warning(); 
      }
    }
    break;
  /*----------遥控模式---------------*/    
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
        
        //急停按钮按下，进入急停模式
        if(BUTTON_IM_STOP_Flag)
        {
          AGV_RUN_Pro=AGV_STATUS_IM_STOP;
          AGV_RUN_SUB_Pro=0;  
          break;
        }
        
        //进入空闲模式
        if(REMOTE_SelectFlag == 0)
        {
          AGV_Delay = 1500;
          LED_DISPLAY_Reset();
          AGV_RUN_Pro = AGV_STATUS_IDLE;
          AGV_RUN_SUB_Pro = 0;  
          break;
        }         
        
        BATT_LOW_LEVEL_1_Warning(); 
      }
    }
    break;
  /*----------低电压模式---------------*/     
  case AGV_STATUS_LOW_POWER:
    {
      if(AGV_RUN_SUB_Pro == 0)
      {
        AGV_Delay = 1500;
        LED_DISPLAY_Reset();
        AGV_RUN_SUB_Pro += 1;
      }
      else
      {
        LED_LOW_POWER_Display(500);

        if(AGV_Delay==0)
        {
          AGV_Delay = 20000; //60s播放一次
          Play_Warning(BAT_LOW_20P);
        }
        
        //遥控ON按钮按下进入，遥控状态
        if((REMOTE_SelectFlag) && (BUTTON_IM_STOP_Flag == 0))
        {
          AGV_RUN_Pro = AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro = 0;
          break;
        }

        if(BatteryVolt_LowFlag < 2)
        {
          AGV_Delay = 1500;
          LED_DISPLAY_Reset();
          AGV_RUN_Pro = AGV_STATUS_IDLE;
          AGV_RUN_SUB_Pro = 0;  
          break;
        }        
      }
    }
    break;
  /*----------臂章模式---------------*/     
  case AGV_STATUS_BARRIER:
    {
      static u8 slow_flag=0;
      static u8 reset = 0;
      if(AGV_RUN_SUB_Pro==0)
      {
        reset = 1;
        if((TOUCH_SENSOR_Flag & (1 << ((Run_Dir == DIR_FORWARD) ? 0 : 1))) != 0)
        {
          MOTO_IM_STOP();
          slow_flag = 0;
        }
        else 
        {
          slow_flag = 1;
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
        
        //遥控ON按钮按下进入，遥控状态
        if((REMOTE_SelectFlag) && (BUTTON_IM_STOP_Flag == 0))
        {
          AGV_RUN_Pro = AGV_STATUS_REMOTE;
          AGV_RUN_SUB_Pro = 0;
          break;
        } 
        
        //急停按钮按下，进入急停模式
        if(BUTTON_IM_STOP_Flag)
        {
          AGV_RUN_Pro = AGV_STATUS_IM_STOP;
          AGV_RUN_SUB_Pro = 0;    
          break;
        }        
        
        //没有障碍物后，进入空闲状态
        if(((TOUCH_SENSOR_Flag & (1<<((Run_Dir==DIR_FORWARD)?0:1)))==0)
            && ((LASER_SENSOR_Flag & (1<<((Run_Dir==DIR_FORWARD)?0:1)))==0)
            && (AGV_Delay==0))
        {
          AGV_Delay = 2400;
          LED_DISPLAY_Reset();
          AGV_RUN_Pro = AGV_STATUS_IDLE; 
          AGV_RUN_SUB_Pro = 0;    
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
  /*----------紧急停止模式---------------*/   
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
        
        //急停按钮松开，进入空闲模式
        if(BUTTON_IM_STOP_Flag == 0)
        {
          SetBeep(3,300,500);
          AGV_Delay = 2400;
          LED_DISPLAY_Reset();
          AGV_RUN_Pro = AGV_STATUS_IDLE;
          AGV_RUN_SUB_Pro = 0;    
          break;
        }
        
        BATT_LOW_LEVEL_1_Warning(); 
      }
    }
    break;
  case AGV_STATUS_CHARGE: //充电
    {
      static u8 reset;
      if(AGV_RUN_SUB_Pro==0)
      {
        reset = 1;
        LED_DISPLAY_Reset();
        CHARGE_FULL_Flag = 0;
        AGV_RUN_SUB_Pro += 1;
        AGV_Delay = 1000;
      }
      else
      { 
        LED_CHARGE_Display(1000);
        switch(AGV_RUN_SUB_Pro)
        {
        case 1: // 刹车
          {
            if(AGV_Delay == 0)
            {
              AGV_Delay = 1500;
              AGV_RUN_SUB_Pro += 1;
              Play_Warning(START_CHARGE);
              printf("-- CHARGE 1: START_CHARGE\n");
            }
          }
          break;
        case 2: // 打开電池充电口
          {
            if(AGV_Delay == 0)
            {
              SET_DIDO_Relay(DO_CHARGE_Enable, 1);
              AGV_Delay = 1000;
              AGV_RUN_SUB_Pro += 1;
              printf("-- CHARGE 2: DO_CHARGE_Enable\n");
            }
          }
          break;      
        case 3:
          { 
            if(AGV_Delay == 0) 
            {
              SET_Charge(CHARGE_ON);
              AGV_Delay = 5 * (60 * 60) * 1000; // 100 * 1000 , 4 * (60 * 60) * 1000
              AGV_RUN_SUB_Pro += 1;
              printf("-- CHARGE 3: ARM PULL OUT \n");
            }
          }
          break;
        case 4:
          {
            //检测电流,小于额定充电电流，结束充电
            if((AGV_Delay == 0) || (FORCE_CHARGE_STOP_Flag) || (CHARGE_FULL_Flag))
            {
              FORCE_CHARGE_STOP_Flag = 0;
              CHARGE_FULL_Flag = 0;
              SET_Charge(CHARGE_OFF);
              AGV_Delay = 2200;
              AGV_RUN_SUB_Pro += 1;
              printf("-- CHARGE 4: ARM PULL IN \n");
            }            
          }
          break;
        case 5:
          {
            if(AGV_Delay==0)
            {              
              //断开充电继电器
              SET_DIDO_Relay(DO_CHARGE_Enable, 0);
              AGV_Delay = 4000;
              AGV_RUN_SUB_Pro = 0;
              AGV_RUN_Pro = AGV_STATUS_IDLE;
              printf("-- CHARGE 5: DO_CHARGE_Disable\n");
              Play_Warning(STOP_CHARGE);
            }        
          }
          break;
        }
      }
    }
    break;    
  }
  //记录位移
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
  
  if(BatteryVolt_LowFlag == 1)
  {
    counter++;
    if(counter >= 300000) // 5分钟
    {
      counter = 0;
      Play_Warning(BAT_LOW_30P);
      //SetBeep(5,300,700);//响5声
    }
  }
  else
  {
    counter=0;
  }
}

void ROUND_SpeedLimted(void)
{
  //转弯限速 ,待测试
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
  //初始化限速 ,待测试
  if(Get_ANALOG_SD_Speed() > FOLLOW_LINE_ROUND_SPEED)
  {
    Set_FollowLineTempBaseSpeed(FOLLOW_LINE_ROUND_SPEED);
  }
}

/************************************************************
 ** 功能：加减速过程初始化---
 ** 参数：accelerated_speed_cmps  - 加速度，单位cm/s
 **       max_speed_cmps          - 最大速度，单位cm/s
 **       cycle_time              - 电机的调整时间周期，单位s
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

// 计算车体位移流程的加减速过程
void Caculate_DisplacmentProcess(MOVEMENT_OPTION* pM,SPEED_OPTION_LIST* pS,u8 coff_enable,SPEED_UP_OPTION* pSPEED)
{
  u32 total_displacement=pM->value;
  u32 dir=pM->dir;
  u32 index=0,i;
  //角度和位移模式更新相关，20160921
  //Wonder_Disp_or_Angle_value=total_displacement;//
  //Finish_Disp_or_Angle_value=0;  
  
  if(coff_enable)
  {
    //位移补偿，20160813
    total_displacement=(u32)(((float)total_displacement)*Displacement_coff);
  }
  if(total_displacement<(pSPEED[0].total_disp*2))
  {
	;
  }
  else if(total_displacement>(pSPEED[SPEED_UP_Length-2].total_disp*2))
  {//加速+匀速+减速
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
  {//加速-减速
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

//计算车体位移流程的加减速过程
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

  if(dir==0)//方向：逆时针
  {
    for(i=0;i<pS->InIndex;i++)
    {
      pS->buf[i].L_Speed=-pS->buf[i].L_Speed;
    }
  }
  else //顺时针
  {
    for(i=0;i<pS->InIndex;i++) 
    {
      pS->buf[i].R_Speed=-pS->buf[i].R_Speed;
    }
  }
}

u32 AGV_USER_PROGRAM_IN_DISPLACEMENT_Task(u8* pReset)
{
#define DISPLACEMENT_FINISH_DELAY  20
  static u32 run_flag = 0;
  if(*pReset)
  {
    *pReset=0;
    run_flag = DISPLACEMENT_FINISH_DELAY;
  }
  //计算位移编程的执行流程
  if(SPEED_OPTION_List.InIndex==SPEED_OPTION_List.OutIndex)
  {
    if(DISPLACEMENT_MOVEMENT_OPTION_List.In_index!=DISPLACEMENT_MOVEMENT_OPTION_List.Out_index)
    {
      MOVEMENT_OPTION* pM=&DISPLACEMENT_MOVEMENT_OPTION_List.buf[DISPLACEMENT_MOVEMENT_OPTION_List.Out_index&LIST_LENGTH_MASK];
      Caculate_DisplacmentProcess(pM,&SPEED_OPTION_List,1,SPEED_UP_OPTION_List[DirectRun]);
      DISPLACEMENT_MOVEMENT_OPTION_List.Out_index+=1;
      run_flag = DISPLACEMENT_FINISH_DELAY;
    }
  }
  
  //计算角度编程的执行流程
  if(SPEED_OPTION_List.InIndex==SPEED_OPTION_List.OutIndex)
  {
    if(ANGLE_MOVEMENT_OPTION_List.In_index!=ANGLE_MOVEMENT_OPTION_List.Out_index)
    {
      MOVEMENT_OPTION* pM=&ANGLE_MOVEMENT_OPTION_List.buf[ANGLE_MOVEMENT_OPTION_List.Out_index&LIST_LENGTH_MASK];
      Caculate_AngleProcess(
           pM,&SPEED_OPTION_List);    
      ANGLE_MOVEMENT_OPTION_List.Out_index+=1;
      run_flag = DISPLACEMENT_FINISH_DELAY;
    }
  }    
  
  //执行机构，处理最底层设置PWM  
  if(ProgramControlCycle == 0)
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
    {//执行完毕
      if(run_flag != 0) run_flag -= 1;
    }
  } 
  return run_flag;
}

// 顺时针为正数，逆时针为负数
void VehicleTurnRound(s16 value)
{
  MOVEMENT_OPTION* pM=
        &ANGLE_MOVEMENT_OPTION_List.buf[ANGLE_MOVEMENT_OPTION_List.In_index&LIST_LENGTH_MASK];
  if(value < 0) pM->dir = 0;
  else pM->dir = 1;
  pM->value = abs_32(value);
  ANGLE_MOVEMENT_OPTION_List.In_index += 1;
}


u8 StartFollowLine(void)
{
  if(FollowLineEnable == 0)
  {
    FollowLineEnable = 1;
    RoadType = ROAD_TYPE_FORWARD;
    Run_Dir = DIR_FORWARD;
    MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR;
    MB_LINE_DIR_SELECT = BRANCH_TO_RIGHT;
    return 1;
  }
  return 0;
}


