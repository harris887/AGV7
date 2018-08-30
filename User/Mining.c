#include "user_inc.h"

s32 RoadType = ROAD_TYPE_FORWARD;

/*
u32 MiningAgvTimeout = 0;
//--------------- CASE 3 ------------------------------------------//
const RFID_INFOR RFID_Infor[RFID_CARD_NUM] = {
  {0x0001, LINE_TYPE_BACKWARD, LAST_ID_NOT_CARE, 1, {{ACTION_BRAKE, 25}, }},
  
  {0x0002, LINE_TYPE_BACKWARD, LAST_ID_NOT_CARE, 3, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
                                                     {ACTION_TURN_ANGLE, 180},
                                                     {ACTION_SET_VEHICLE_DIR, DIR_BACKWARD}, }},
  
  {0x0003, LINE_TYPE_FORWARD , LAST_ID_NOT_CARE, 2, {{ACTION_BRAKE, 25},
                                                     {ACTION_TURN_ANGLE, 90}, }},
  
  {0x0004, LINE_TYPE_FORWARD , 0x0005          , 2, {{ACTION_BRAKE, 25},
                                                     {ACTION_TURN_ANGLE, 90}, }},
                                                     
  {0x0005, LINE_TYPE_FORWARD , 0x0004          , 3, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE} ,
                                                     {ACTION_WAIT, 30},
                                                     {ACTION_TURN_ANGLE, 180}, }},
                                                     
  {0x0006, LINE_TYPE_FORWARD , LAST_ID_NOT_CARE, 2, {{ACTION_BRAKE, 25},
                                                     {ACTION_TURN_ANGLE, 90}, }},
                                                     
  {0x0007, LINE_TYPE_FORWARD , 0x0008          , 2, {{ACTION_BRAKE, 25},
                                                     {ACTION_TURN_ANGLE, 90}, }},
                                                     
  {0x0008, LINE_TYPE_FORWARD , 0x0007          , 3, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
                                                     {ACTION_WAIT, 30},
                                                     {ACTION_TURN_ANGLE, 180}, }},  
  
  {0x0009, LINE_TYPE_FORWARD , LAST_ID_NOT_CARE, 2, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
                                                     {ACTION_WAIT, 30}, }},
  
  {0x000A, LINE_TYPE_FORWARD , LAST_ID_NOT_CARE, 2, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
                                                     {ACTION_WAIT, 30}, }},
                                                     
  {0x000B, LINE_TYPE_FORWARD , LAST_ID_NOT_CARE, 2, {{ACTION_BRAKE, 25},
                                                     {ACTION_TURN_ANGLE, 90}, }},
                                                     
  {0x000C, LINE_TYPE_FORWARD , 0x000D          , 2, {{ACTION_BRAKE, 25},
                                                     {ACTION_TURN_ANGLE, 90}, }},
                                                     
  {0x000D, LINE_TYPE_FORWARD , 0x000C          , 3, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
                                                     {ACTION_WAIT, 30},
                                                     {ACTION_TURN_ANGLE, 180}, }},  
                                                     
  {0x000E, LINE_TYPE_FORWARD , LAST_ID_NOT_CARE, 5, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
                                                     {ACTION_WAIT, 30},
                                                     {ACTION_TURN_ANGLE, 180}, 
                                                     {ACTION_SET_LINE_TYPE, LINE_TYPE_BACKWARD}, 
                                                     {ACTION_SET_BRANCH_DIR, BRANCH_TO_RIGHT}, }},    
};

void MiningAgvSpecFlow(u8* reset)
{
#define ACTION_PRO_OFFSET    3
  static u8 LineType = LINE_TYPE_FORWARD; 
  static u8 BranchSelectDirection = BRANCH_TO_LEFT;
  static s16 VehicleDirection = DIR_FORWARD;
  static u16 LastRfid = 0;
  static u8 Pro = 0;
  static u16 ActionIndex = 0;
  static u8 InsideReset = 0;
  static const RFID_INFOR* pRFID_INFOR = NULL; 
  if(*reset)
  {
    LineType = LINE_TYPE_FORWARD;           // 路线分类：前进路线、返回路线
    BranchSelectDirection = BRANCH_TO_LEFT; // 遇到磁条分叉时的方向选择
    VehicleDirection = DIR_FORWARD;         // 车体巡线方向：正向用前边磁导航传感器，逆向用后边磁导航传感器
    Run_Dir = DIR_FORWARD;
    MB_LINE_DIR_SELECT = BRANCH_TO_RIGHT;
    MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR;
    LastRfid = 0;
    InsideReset = 1;
    Pro = 0;
  }
  
  switch(Pro)
  {
  case 0: // 巡线
    {
      NEW_FOLLOW_LINE_TASK(&InsideReset, VehicleDirection);
      
      // 检测RFID
      if(RFID_COMEIN_Flag & 0x4) 
      {
        u8 i;
        RFID_COMEIN_Flag &= ~0x4;
        for(i = 0; i < RFID_CARD_NUM; i++)
        {
          if(RFID_Infor[i].Id == (PlaceId & 0xFF))
          {
            pRFID_INFOR = RFID_Infor + i;
            Pro = 1;
            break;
          }
        }
      }
    }
    break;
  case 1: // RFID处理_01
    {
      if((LineType == pRFID_INFOR->ResponseLineType) && 
          ((LAST_ID_NOT_CARE == pRFID_INFOR->ResponseLastId) || (LastRfid == pRFID_INFOR->ResponseLastId)))
      {
        ActionIndex = 0;
        Pro = 2;
      }
      else
      {
        Pro = 0;
        printf("-- ByPass RFID %04X --\n", pRFID_INFOR->Id);
      }
    }
    break;
  case 2: // RFID处理_02
    {
      if(ActionIndex >= pRFID_INFOR->ActionNum)
      {
        InsideReset = 1;
        LastRfid = (PlaceId & 0xFF);
        PlaceId = 0;
        Pro = 0;
      }
      else if(pRFID_INFOR->ActionInfor[ActionIndex].ActionType == ACTION_BRAKE)      // 刹车
      {
        InsideReset = 1;
        MiningAgvTimeout = 1500;
        Pro = (ACTION_PRO_OFFSET + ACTION_BRAKE);
      }
      else if(pRFID_INFOR->ActionInfor[ActionIndex].ActionType == ACTION_TURN_ANGLE) // 转弯
      {
        VehicleTurnRound(pRFID_INFOR->ActionInfor[ActionIndex].value);
        InsideReset = 1;
        MiningAgvTimeout = 10000;      
        Pro = (ACTION_PRO_OFFSET + ACTION_TURN_ANGLE);
      }
      else if(pRFID_INFOR->ActionInfor[ActionIndex].ActionType == ACTION_WAIT)       // 等待
      {
        InsideReset = 1;
        MiningAgvTimeout = 1000 * pRFID_INFOR->ActionInfor[ActionIndex].value;      
        Pro = (ACTION_PRO_OFFSET + ACTION_WAIT);
      }
      else if(pRFID_INFOR->ActionInfor[ActionIndex].ActionType == ACTION_SET_LINE_TYPE)
      {
        LineType = pRFID_INFOR->ActionInfor[ActionIndex].value;
        ActionIndex += 1;
      }
      else if(pRFID_INFOR->ActionInfor[ActionIndex].ActionType == ACTION_SET_BRANCH_DIR)
      {
        BranchSelectDirection = pRFID_INFOR->ActionInfor[ActionIndex].value;
        MB_LINE_DIR_SELECT = pRFID_INFOR->ActionInfor[ActionIndex].value; // 兼容老版本
        ActionIndex += 1;
      }
      else if(pRFID_INFOR->ActionInfor[ActionIndex].ActionType == ACTION_SET_VEHICLE_DIR)
      {
        VehicleDirection = pRFID_INFOR->ActionInfor[ActionIndex].value;
        Run_Dir = pRFID_INFOR->ActionInfor[ActionIndex].value;
        if(VehicleDirection == DIR_BACKWARD) 
        {
          MODE_BUS_HALL_Addr = BACKWARD_MODE_BUS_HALL_ADDR;
        }
        else
        {
          MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR;
        }
        ActionIndex += 1;
      }      
    }
    break;
  case (ACTION_PRO_OFFSET + ACTION_BRAKE): // 刹车
    {
      if(MiningAgvTimeout != 0)
      {
        SLOW_DOWN_Task(&InsideReset, 1000);
      }
      else 
      {      
        ActionIndex += 1;
        Pro = 2;
      }
    }
    break;
  case (ACTION_PRO_OFFSET + ACTION_TURN_ANGLE):
    {
      if(MiningAgvTimeout != 0)
      {
        AGV_USER_PROGRAM_IN_DISPLACEMENT_Task(&InsideReset);
      }
      else
      {
        ActionIndex += 1;
        Pro = 2;
      }      
    }
    break;
  case (ACTION_PRO_OFFSET + ACTION_WAIT):
    {
      if(MiningAgvTimeout != 0)
      {
        // do nothing
      }
      else
      {
        ActionIndex += 1;
        Pro = 2;
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
  }
}
*/
//--------------- CASE 1 ------------------------------------------//
#define VIDEO_DELAY_S  5
const RFID_ACTION FORWARD_RFID_Action[FUNC_1_RFID_CARD_NUM] = {
  
  {0x80FF, 0,},
  
  {0x8001, 1, {{ACTION_SET_BRANCH_DIR, BRANCH_TO_RIGHT}, }},
  
  {0x0002, 4, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_WAIT, VIDEO_DELAY_S}, 
               {ACTION_TURN_ANGLE, 180}, 
               {ACTION_WAIT, VIDEO_DELAY_S}, }},
  
  {0x0003, 4, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_WAIT, VIDEO_DELAY_S},
               {ACTION_TURN_ANGLE, 180}, 
               {ACTION_WAIT, VIDEO_DELAY_S}, }},
                                                     
  {0x0004, 2, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE} ,
               {ACTION_WAIT, VIDEO_DELAY_S}, }},
                                                     
  {0x0005, 2, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE} ,
               {ACTION_WAIT, VIDEO_DELAY_S}, }},
                                                     
  {0x0006, 4, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_WAIT, VIDEO_DELAY_S},
               {ACTION_TURN_ANGLE, 180}, 
               {ACTION_WAIT, VIDEO_DELAY_S}, }},
                                                     
  {0x8007, 6, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_WAIT, VIDEO_DELAY_S},
               {ACTION_TURN_ANGLE, 180}, 
               {ACTION_WAIT, VIDEO_DELAY_S}, 
               {ACTION_SET_LINE_TYPE, ROAD_TYPE_BACKWARD}, 
               {ACTION_SET_BRANCH_DIR, BRANCH_TO_LEFT}, }},
};


const RFID_ACTION BACKWARD_RFID_Action[FUNC_1_RFID_CARD_NUM] = {
  
  {0x80FF, 3, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE}, 
               {ACTION_SET_VEHICLE_DIR, DIR_FORWARD}, 
               {ACTION_CHARGE, 0}}},
  
  {0x8001, 3, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_TURN_ANGLE, 180}, 
               {ACTION_SET_VEHICLE_DIR, DIR_BACKWARD}, }},
  
  {0x0002, 2, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_TURN_ANGLE, 180}, }},
  
  {0x0003, 2, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_TURN_ANGLE, 180}, }},
                                                     
  {0x0004, 0, },
                                                     
  {0x0005, 0, },
                                                     
  {0x0006, 2, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_TURN_ANGLE, 180}, }},
                                                     
  {0x8007, 0, },
};