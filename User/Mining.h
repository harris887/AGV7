#ifndef _MINING_H_
#define _MINING_H_

typedef enum
{
  ACTION_BRAKE = 0,
  ACTION_TURN_ANGLE,
  ACTION_WAIT,
  ACTION_SET_LINE_TYPE,
  ACTION_SET_BRANCH_DIR,
  ACTION_SET_VEHICLE_DIR,
  ACTION_CHARGE
}ACTIONS;

typedef struct
{
  ACTIONS ActionType;
  s32 value;
}ACTION_INFOR;

#define MAX_ACTION_NUM    8
typedef struct
{
  u16 Id;
  u16 ResponseLineType;
  u16 ResponseLastId;
  u16 ActionNum;
  ACTION_INFOR ActionInfor[MAX_ACTION_NUM];
}RFID_INFOR;

typedef struct
{
  u16 Id;
  u16 ActionNum;
  ACTION_INFOR ActionInfor[MAX_ACTION_NUM];
}RFID_ACTION;

#define RFID_CARD_NUM            14
#define LAST_ID_NOT_CARE         0x00FF
#define BRAKE_DISTANCE_NOT_CARE  -1

#define FUNC_1_RFID_CARD_NUM     8

#define ROAD_TYPE_FORWARD   1
#define ROAD_TYPE_BACKWARD  -1
#define LINE_TYPE_FORWARD   0
#define LINE_TYPE_BACKWARD  1
#define BRANCH_TO_LEFT      1
#define BRANCH_TO_RIGHT     2


extern const RFID_INFOR RFID_Infor[RFID_CARD_NUM];
extern u32 MiningAgvTimeout;
extern s32 RoadType;

extern const RFID_ACTION FORWARD_RFID_Action[FUNC_1_RFID_CARD_NUM];
extern const RFID_ACTION BACKWARD_RFID_Action[FUNC_1_RFID_CARD_NUM];
#endif