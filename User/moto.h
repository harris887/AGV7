#ifndef _moto_h_
#define _moto_h_



/*电机_驱动器相关设置*/
#define FULL_SPEED_STEP             1000.0
#define ROAD_RECORD_ONCE_TIME_MS    100
#define MAX_WHEEL_RUN_LENGTH_IN_CM_PER_SECOND (max_wheel_speed*1.0)
#define MAX_WHEEL_RUN_LENGTH_IN_CM_PER_100MS  (max_wheel_speed*0.1)
#define MAX_WHEEL_RUN_LENGTH_IN_CM_PER_10MS  (max_wheel_speed*0.01)

#define DEFAULT_WHEEL_DIAMETER_IN_MM  121
#define MAX_MOTO_SPEED_IN_RPM         200 //200 400
#define WHEEL_DIAMETER_IN_CM          (DEFAULT_WHEEL_DIAMETER_IN_MM * 0.1)//12.1  //25 20 31 22.5 ，20.3 
#define MAX_MOTO_SPEED_IN_D1RPM       (MAX_MOTO_SPEED_IN_RPM * 10)
#define SPEED_DOWN_RATIO              1.0        //电机齿轮箱减速比
#define MAX_REMOTE_SPEED_IN_D1RPM    (MAX_MOTO_SPEED_IN_RPM * 10)


#define MAX_SPEED_STEP    1000

typedef enum
{
  LEFT_MOTO_INDEX=0,
  RIGHT_MOTO_INDEX,
  MOTO_NUM
}MOTO_INDEX_ENUM;

#define PWM_CYCLE_COUNTER 10000
#define ZERO_SPEED_PWM_COUNTER  (PWM_CYCLE_COUNTER>>1)
//速度，模式，脉冲数


#define CONTROL_MODE_PROGRAM 0
#define CONTROL_MODE_REMOTE   1

#define WHEEL_L1_ENUM     0
#define WHEEL_L2_ENUM     1
#define WHEEL_R1_ENUM     2
#define WHEEL_R2_ENUM     3
#define WHEEL_RUN_DIR_POSTIVE 1
#define WHEEL_RUN_DIR_NEGTIVE (-1)

#define MOVE_MODE_NULL        0
#define MOVE_MODE_FORWARD     1
#define MOVE_MODE_BACKWARD    2
#define MOVE_MODE_LEFTWARD    3
#define MOVE_MODE_RIGHTWARD   4
#define MOVE_MODE_UP_ROLL     5
#define MOVE_MODE_DOWN_ROLL   6


#define PROGRAM_RUN_STATUS_IDEL   0
#define PROGRAM_RUN_STATUS_ING    1
#define PROGRAM_RUN_STATUS_FINISH 2
extern u8 PROGRAM_RUN_Status;//0-空闲，1-运行一次，2-运行完毕
extern u8 PROGRAM_RUN_Index;

//设置这个值会改变遥控的最大速度，10--1kHZ,5--2KHZ,1-10KHZ
//最小为1，最大请在100以内
#define REMOTE_MIN_PULSE_TIME 10
typedef struct
{
  u32 PulseNum;
  u32 PulseUpTime;
  u32 PulseDownTime;
  u32 PulseDelayCounter;
  u8  ActiveFlag;
  u8  ActivePro;
  u8  rev[2];
}MOTO_OPTION;

extern MOTO_OPTION* pMOTO_OPTION[4];
extern MOTO_OPTION  MOTO_OPTION_List[1][4];
//遥控时使用，不影响固定路线
extern MOTO_OPTION  REMOTE_MOTO_OPTION_List[4];
extern s16 FourWheelSpeed[4];

extern u16 ROAD_RECORD_Timeout;
extern s32 LeftRoadLength;
extern s32 RightRoadLength;
extern float max_wheel_speed;
extern u16 MOTO_485COMM_Timeout;

void MOTO_Init(void);
void MOTO_TEST(void);
void PROGRAM_TASK(void);
void FOLLOW_LINE_TASK(u8* pFollowLineReset);

void MOTO_IM_STOP(void);


void SetSpeedRate(MOTO_INDEX_ENUM MOTO_SELECT,s16 Speed);
void MOTO_FaultCheck_TASK(void);
//void FOLLOW_LINE_TASK(void);
void ROAD_RECORD_Task(void);

//============== 位移&角度模式抽象 ================== 
//暂定30%速度,有加减速过程。
#define MAX_SPEED_UP_STEP 32
#define MAX_SPEED_DOWN_STEP 32
#define MAX_SPEED_HOLD_STEP 32
#define MAX_SPEED_OPTION_NUM  (MAX_SPEED_UP_STEP+MAX_SPEED_HOLD_STEP+MAX_SPEED_DOWN_STEP)
typedef struct
{
  u32 repeat_times;//100ms的次数
  s16 L_Speed;
  s16 R_Speed;
}SPEED_OPTION;

typedef struct
{
  u16 InIndex;
  u16 OutIndex;
  SPEED_OPTION buf[MAX_SPEED_OPTION_NUM];  
}SPEED_OPTION_LIST;
extern SPEED_OPTION_LIST SPEED_OPTION_List;

#define DEFALUT_AGV_FOLLOW_LINE_SPEED_PERSENT 30
extern u32 follow_speed_persent;
u8 SLOW_DOWN_Task(u8* reset,u16 time_in_ms);

void Set_FollowLineTempBaseSpeed(s32 value);
u16 Get_FollowLineTempBaseSpeed(void);
void Clear_FollowLineTempBaseSpeed(void);
void NEW_FOLLOW_LINE_TASK(u8* pFollowLineReset,s16 dir);

extern s32 pid_out_global;
extern u16 hall_value;
extern u8 speed_step_g;

void MOTO_SPEED_CONTROL_TASK(void);
s32 Get_ANALOG_SD_Speed(void);
#define FOLLOW_LINE_ROUND_SPEED       250 //0.4m/s 
#define FOLLOW_LINE_NOT_MIDDLE_SPEED  200 //0.2m/s
#define MOTO_ZERO_FREE_TIME_IN_MS     2000  //电机停止后2s进入自由状态

extern void SetD1Rpm(MOTO_INDEX_ENUM MOTO_SELECT,s16 d1rpm);

extern u8 moto_enable_status[MOTO_NUM];
extern float RoadLength[MOTO_NUM];
extern u32 ReadMotoRpmTimes[MOTO_NUM];

extern void Set_FollowLineTempAcc(s32 value);
extern void Clear_FollowLineTempAcc(void);
#endif