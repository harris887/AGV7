#ifndef program_h_
#define program_h_

#define DEFAULT_RFID_WAIT_TIME_IN_MS          10000 
#define DEFAULT_PLAY_DETECT_THING_TIME_IN_MS  5000
#define DEFAUTL_LASER_DISTANCE_CM             50
#define DEFAUTL_LASER_WIDTH_CM                50

extern u32 AGV_Delay;

#define DEFAULT_Vehicle_WIDTH_FLOAT   35.0    //车体宽度，51.0 ,56.0 , 45.0

#define MIN_DEST_ID     1
#define MAX_DEST_ID     10

#define ROUND_LOW_ID    9
#define ROUND_HIGH_ID   10

#define DIR_FORWARD     1
#define DIR_BACKWARD    -1

#define MAX_LIST_LENGTH 32
#define LIST_LENGTH_MASK  (MAX_LIST_LENGTH-1)
typedef struct
{
  u32 dir;
  u32 value;
}MOVEMENT_OPTION;

typedef struct
{
  u8 In_index;
  u8 Out_index;
  MOVEMENT_OPTION buf[MAX_LIST_LENGTH];
}MOVEMENT_OPTION_LIST;

typedef enum
{
  DirectRun = 0,
  CircleRun,
  RunFuncNum,
}VEHICLE_RUN_FUNC;

extern MOVEMENT_OPTION_LIST DISPLACEMENT_MOVEMENT_OPTION_List;
extern MOVEMENT_OPTION_LIST ANGLE_MOVEMENT_OPTION_List;
extern u16 ProgramControlCycle;

extern void AGV_RUN_Task(void);
extern u16 RFID_STOP_ANGIN_Timeout;

extern void AGV_USER_PROGRAM_IN_SPEED_Task(u8* pReset);
void BATT_LOW_LEVEL_1_Warning(void);
void MovementListInit(void);
u32 AGV_USER_PROGRAM_IN_DISPLACEMENT_Task(u8* pReset);



typedef struct
{
  float total_time;//加速总时间
  float total_disp;//加速过程的位移
  float current_speed;//此时的速度
}SPEED_UP_OPTION;
#define MAX_SPEED_UP_LIST_LENGTH  32
extern u16 SPEED_UP_Length;

#define DEFAULT_PROGRAM_CYCLE_IN_MS 10  
void Caculate_DisplacmentProcess(MOVEMENT_OPTION* pM,SPEED_OPTION_LIST* pS,u8 coff_enable,SPEED_UP_OPTION* pSPEED);
void Caculate_AngleProcess(MOVEMENT_OPTION* pM,SPEED_OPTION_LIST* pS);
extern SPEED_UP_OPTION SPEED_UP_OPTION_List[RunFuncNum][MAX_SPEED_UP_LIST_LENGTH];
void SPEED_UP_DOWN_STRUCT_Init(float accelerated_speed_cmps,float max_speed_cmps,float cycle_time,SPEED_UP_OPTION* pSPEED);
s16 speed_to_pwm(float speed_in_cmps);
void ROUND_SpeedLimted(void);
typedef enum
{
  AGV_STATUS_INIT=0,
  AGV_STATUS_IDLE,
  AGV_STATUS_FOLLOWLINE,
  AGV_STATUS_REMOTE,
  AGV_STATUS_LOW_POWER,
  AGV_STATUS_IM_STOP,
  AGV_STATUS_BARRIER,
  AGV_STATUS_RFID_COMEIN,
  
  AGV_STATUS_USER_PROGRAM,
  AGV_STATUS_OFF_LINE,
  AGV_STATUS_CHARGE
}AGV_STATUS_LIST;
extern AGV_STATUS_LIST AGV_RUN_Pro;
extern u16 LoopDetectThing_time_out;
extern void INIT_SpeedLimted(void);
extern void VehicleTurnRound(s16 value);
extern u8 FollowLineEnable;
extern s16 Run_Dir;
extern u32 CHARGE_MinCycle;
extern u8 FORCE_CHARGE_Flag;
extern u8 FORCE_CHARGE_STOP_Flag;
extern u32 FOLLOW_LOOP_Timeout;

u8 StartFollowLine(void);
#endif