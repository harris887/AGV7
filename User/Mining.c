#include "user_inc.h"

u32 MiningAgvTimeout = 0;

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