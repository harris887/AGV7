#include "user_inc.h"

s32 RoadType = ROAD_TYPE_FORWARD;

#define VIDEO_DELAY_S  (DEFAULT_RFID_WAIT_TIME_IN_MS / 1000)
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
                                                
#if (! FUNC_1_ADD_MORE_CARD)
  {0x8007, 6, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_WAIT, VIDEO_DELAY_S},
               {ACTION_TURN_ANGLE, 180}, 
               {ACTION_WAIT, VIDEO_DELAY_S}, 
               {ACTION_SET_LINE_TYPE, ROAD_TYPE_BACKWARD}, 
               {ACTION_SET_BRANCH_DIR, BRANCH_TO_LEFT}, }},
               
#else
  {0x8007, 4, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_WAIT, VIDEO_DELAY_S},
               {ACTION_TURN_ANGLE, 180}, 
               {ACTION_WAIT, VIDEO_DELAY_S}, }},
               //{ACTION_SET_LINE_TYPE, ROAD_TYPE_BACKWARD}, 
               //{ACTION_SET_BRANCH_DIR, BRANCH_TO_LEFT}, }},

  {0x8008, 6, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_WAIT, VIDEO_DELAY_S},
               {ACTION_TURN_ANGLE, 180}, 
               {ACTION_WAIT, VIDEO_DELAY_S}, 
               {ACTION_SET_LINE_TYPE, ROAD_TYPE_BACKWARD}, 
               {ACTION_SET_BRANCH_DIR, BRANCH_TO_LEFT}, }},
#endif
};


const RFID_ACTION BACKWARD_RFID_Action[FUNC_1_RFID_CARD_NUM] = {
  
  {0x80FF, 3, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE}, 
               {ACTION_SET_VEHICLE_DIR, DIR_FORWARD}, 
               {ACTION_FINISH, 0}}},
  
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
                        
#if (! FUNC_1_ADD_MORE_CARD)
  {0x8007, 0, },
#else
  {0x8007, 2, {{ACTION_BRAKE, BRAKE_DISTANCE_NOT_CARE},
               {ACTION_TURN_ANGLE, 180}, }},
                        
  {0x8008, 0, },
#endif
};