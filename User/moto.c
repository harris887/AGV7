#include "user_inc.h"
#include "string.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"

#define MOTO_COMM_PORT_ENUM             5   // 使用 USART5 发送
#define MOTO_CONTROL_CYCLE              12  // 20ms 的RS485通信周期
#define DEFAULT_MOTO_READ_RPM_TIME_OUT  100
u16 MOTO_485COMM_Timeout = 2000;
s16 moto_speed_in_rpm[MOTO_NUM] = {0,0};
u8 moto_enable_status[MOTO_NUM] = {0,0};
u8 moto_enable_status_change_flag[MOTO_NUM] = {0,0};
u16 MOTO_READ_RPM_Timeout[MOTO_NUM] = {DEFAULT_MOTO_READ_RPM_TIME_OUT,DEFAULT_MOTO_READ_RPM_TIME_OUT};
u8 moto_reset_speed_up_down_time_flag[MOTO_NUM] = {0,0};

#define MOTO_SPEED_UP_DOWN_DELAY_TIME     20  //100
const u8 MODBUS_MOTO_ENBALE[8] =
{0x01 ,0x06 ,0x00 ,0x38 ,0x00 ,0x01 ,0xC9 ,0xC7};
const u8 MODBUS_MOTO_RPM_SET[8] = 
{0x01 ,0x06 ,0x00 ,0x33 ,0x00 ,0x64 ,0x78 ,0x2E};
const u8 MODBUS_MOTO_STOP_SET[8] = 
{0x01 ,0x06 ,0x00 ,0x1F ,0x00 ,0x00 ,0xB8 ,0x0C};
//{0x01 ,0x06 ,0x00 ,0x1F ,0x00 ,0x01 ,0x79 ,0xCCC};
const u8 MODBUS_MOTO_UP_TIME_SET[8] = 
{0x01 ,0x06 ,0x00 ,0x30 ,0x00 ,0x64 ,0x00 ,0x00};
const u8 MODBUS_MOTO_DOWN_TIME_SET[8] = 
{0x01 ,0x06 ,0x00 ,0x31 ,0x00 ,0x64 ,0x00 ,0x00};
const u8 MODBUS_MOTO_RPM_READ[8] = 
{0x01 ,0x03 ,0x10 ,0x2D ,0x00 ,0x01 ,0x85, 0xF6};//0x01
//{0x01 ,0x03 ,0x10 ,0x00 ,0x00 ,0x01 ,0x85, 0xF6};//0x01
u8 moto_comm_buff[256];
u32 ReadMotoRpmTimes[MOTO_NUM] = {0,0};
u32 SetMotoRpmTimes[MOTO_NUM] = {0,0};
s16 RealRpm[MOTO_NUM] = {0,0};


//计算位移使用，-1000~1000
//s16 LeftRealSpeed=0;
//s16 RightRealSpeed=0;
u16 ROAD_RECORD_Timeout=0;
s32 LeftRoadLength=0;
s32 RightRoadLength=0;
s16 LeftRealSpeed;
s16 RightRealSpeed;

s16 FourWheelSpeed[4]={0,0,0,0};
float max_wheel_speed;//车轮最大速度，单位cm/S
SPEED_OPTION_LIST SPEED_OPTION_List={0,0};

/*根据车轮参数计算轮子/车体的额定速度,单位:cm/s
  输入：
  WHEEL_DIAMETER_IN_CM: 轮子直径(单位:cm)
  MAX_MOTO_SPEED_IN_RPM: 设定的电机的最高(100%)转速(单位:rpm)
  SPEED_DOWN_RATIO: 齿轮箱减速比
  返回：车轮最大速度，单位cm/s
*/
float Caculate_MAX_WHEEL_SPEED(float wheel_diameter_in_cm,float max_moto_speed_in_rpm,float speed_down_ratio)
{
  return (wheel_diameter_in_cm*PI)*(max_moto_speed_in_rpm/speed_down_ratio/60.0);
}

void MOTO_Init(void)
{
  max_wheel_speed = Caculate_MAX_WHEEL_SPEED(WHEEL_DIAMETER_IN_CM, MAX_MOTO_SPEED_IN_RPM, SPEED_DOWN_RATIO);
  SPEED_UP_DOWN_STRUCT_Init(50, 100, 0.1,SPEED_UP_OPTION_List[DirectRun]);//加速度20cm/S^2,最高速度40cm/S,加速周期0.1s ,40
  SPEED_UP_DOWN_STRUCT_Init(10 , 20, 0.1,SPEED_UP_OPTION_List[CircleRun]);
  COFF_001RPM_TO_MMS = WHEEL_DIAMETER_IN_CM * 10.0 * PI * 0.01 / 60.0;
  COFF_01RPM_TO_MMS = (WHEEL_DIAMETER_IN_CM * 10.0 * PI * 0.1) / 60.0;
  COFF_MMS_TO_D1RPM = 60.0 / (WHEEL_DIAMETER_IN_CM * 10.0 * PI * 0.1) ;
  //COFF_DISTANCE = (float)MOD_BUS_Reg.COFF_DISTANCE_1000TIME * 0.001;
  
  if(LOG_Level <= LEVEL_INFO) printf("max_wheel_speed = %f , COFF_001RPM_TO_MMS = %f\n", max_wheel_speed, COFF_001RPM_TO_MMS);
  if(LOG_Level <= LEVEL_INFO) printf("COFF_MMS_TO_D1RPM = %f , COFF_DISTANCE = %f\n", COFF_MMS_TO_D1RPM, COFF_DISTANCE);
}

void SetD1Rpm(MOTO_INDEX_ENUM MOTO_SELECT,s16 d1rpm)
{
  if(d1rpm > MAX_MOTO_SPEED_IN_D1RPM) d1rpm = MAX_MOTO_SPEED_IN_D1RPM;
  else if(d1rpm < -MAX_MOTO_SPEED_IN_D1RPM)  d1rpm = -MAX_MOTO_SPEED_IN_D1RPM;  
  
  if(MOTO_SELECT == LEFT_MOTO_INDEX)
  {
    RealRpm[LEFT_MOTO_INDEX] = d1rpm;
    moto_speed_in_rpm[LEFT_MOTO_INDEX] = d1rpm;
  }
  else if(MOTO_SELECT==RIGHT_MOTO_INDEX)
  {
    RealRpm[RIGHT_MOTO_INDEX] = d1rpm;
    moto_speed_in_rpm[RIGHT_MOTO_INDEX] = -d1rpm;
  }  
}

//Speed:-1000~1000
void SetSpeedRate(MOTO_INDEX_ENUM MOTO_SELECT,s16 Speed)
{
  s32 d1rpm;
  if(Speed > MAX_SPEED_STEP) Speed = MAX_SPEED_STEP;
  else if(Speed < -MAX_SPEED_STEP)  Speed = -MAX_SPEED_STEP;  
  
  if(MOTO_SELECT == LEFT_MOTO_INDEX)
  {
    LeftRealSpeed = Speed;
    d1rpm = Speed * MAX_MOTO_SPEED_IN_D1RPM / MAX_SPEED_STEP;
    
    RealRpm[LEFT_MOTO_INDEX] = d1rpm;
    moto_speed_in_rpm[LEFT_MOTO_INDEX] = d1rpm;    
  }
  else if(MOTO_SELECT==RIGHT_MOTO_INDEX)
  {
    RightRealSpeed = Speed;
    d1rpm = Speed * MAX_MOTO_SPEED_IN_D1RPM / MAX_SPEED_STEP;

    RealRpm[RIGHT_MOTO_INDEX] = d1rpm;
    moto_speed_in_rpm[RIGHT_MOTO_INDEX] = -d1rpm;    
  }
}

void MOTO_IM_STOP(void)
{
  SetD1Rpm(LEFT_MOTO_INDEX,  0);
  SetD1Rpm(RIGHT_MOTO_INDEX, 0);
}


//巡线速度，1000是最大速度，后续跟随电位器变化100~500范围
//260~3700
#define MIN_SPEED_AD_ADJUST     400
#define MAX_SPEED_AD_ADJUST     (MIN_SPEED_AD_ADJUST+3400)
#define FOLLOW_LINE_MAX_SPEED   500   //限定在0.5m/s 
#define FOLLOW_LINE_MIN_SPEED   0  
#define max_pwm_speed_up_100ms  10   //0.5m/ss-25  0.2m/ss-10


// return : 0 - stop_status , 1 - Runing
u8 SLOW_DOWN_Task(u8* reset,u16 time_in_ms)
{
  static s16 slow_value_every_time[MOTO_NUM] = {0, 0};
  static s16 Speed_bk[MOTO_NUM];
  u8 status = 1;
  u8 i;
  if(PID_TimeOut == 0)
  {
    PID_TimeOut = 100;
    if(*reset)
    {
      *reset=0;
      if(time_in_ms < 100) time_in_ms = 100;
      for(i = 0; i < MOTO_NUM; i++)
      {
        Speed_bk[i] = RealRpm[i];
        slow_value_every_time[i] = (s32)RealRpm[i] / ((s32)time_in_ms / 100);
        if(slow_value_every_time[i] == 0) slow_value_every_time[i] = RealRpm[i];
      }     
    }
    else
    {
      if((RealRpm[LEFT_MOTO_INDEX] != 0) || (RealRpm[RIGHT_MOTO_INDEX] != 0))    
      {
        for(i = 0; i < MOTO_NUM; i++)
        {
          if(abs(Speed_bk[i]) <= abs(slow_value_every_time[i]))
          {
            Speed_bk[i] = 0;
          }
          else 
          {
            Speed_bk[i] -= slow_value_every_time[i];
          }
          SetD1Rpm((MOTO_INDEX_ENUM)i, Speed_bk[i]);
        }   
      }
      else
      {
        for(i = 0;i < MOTO_NUM; i++)
        {
          SetD1Rpm((MOTO_INDEX_ENUM)i, 0);
        }    
        status = 0;
      }
    }
  }
  return status;
}

//=========================================
u8 FollowLineTempSpeed_ValidFlag = 0;
u16 FollowLineTempSpeed_Value = 0;
u8 FollowLineTempAcc_ValidFlag = 0;
u16 FollowLineTempAcc_Value = 20;

//设置临时的巡线速度/PWM
void Set_FollowLineTempBaseSpeed(s32 value)
{
  FollowLineTempSpeed_ValidFlag = 1;
  FollowLineTempSpeed_Value = value;
}

u16 Get_FollowLineTempBaseSpeed(void)
{
  return FollowLineTempSpeed_Value;
}

//清除临时的巡线速度/PWM
void Clear_FollowLineTempBaseSpeed(void)
{
  FollowLineTempSpeed_ValidFlag=0;
}

s32 Get_ANALOG_SD_Speed(void)
{
  s32 value;
  s32 RollValue;
  RollValue = I_RollAd>>8;
  RollValue = lmin(RollValue,MAX_SPEED_AD_ADJUST);
  RollValue = lmax(RollValue,MIN_SPEED_AD_ADJUST);
  value = (FOLLOW_LINE_MAX_SPEED * (MAX_SPEED_AD_ADJUST - RollValue)) \
              /(MAX_SPEED_AD_ADJUST-MIN_SPEED_AD_ADJUST);
  return value;
}

void Set_FollowLineTempAcc(s32 value)
{
  FollowLineTempAcc_ValidFlag = 1;
  FollowLineTempAcc_Value = value;
}

void Clear_FollowLineTempAcc(void)
{
  FollowLineTempAcc_ValidFlag = 0;
}

s32 Get_FollowLineExpectAcc(void)
{
  s32 value;
  if(FollowLineTempAcc_ValidFlag != 0)
    value = FollowLineTempAcc_Value;
  else
    value = max_pwm_speed_up_100ms;
  return value;
}

/*获取巡线期望的额定速度*/
s32 Get_FollowLineExpectBaseSpeed(void)
{
  s32 value;
  if(FollowLineTempSpeed_ValidFlag!=0)
  {
    value = FollowLineTempSpeed_Value;
  }
  else if(MOD_BUS_Reg.AUTO_FOLLOW_SPEED_CONTROL_MODE == AUTO_FOLLOW_SPEED_CONTROL_MODE_ANALOG)
  {
    s32 RollValue;
    RollValue = I_RollAd>>8;
    RollValue = lmin(RollValue,MAX_SPEED_AD_ADJUST);
    RollValue = lmax(RollValue,MIN_SPEED_AD_ADJUST);
    value = (FOLLOW_LINE_MAX_SPEED * (MAX_SPEED_AD_ADJUST - RollValue)) \
              /(MAX_SPEED_AD_ADJUST-MIN_SPEED_AD_ADJUST);
  }
  else
  {
    value = ((s32)FOLLOW_LINE_MAX_SPEED * (s32)MOD_BUS_Reg.AUTO_FOLLOW_SPEED) / 100;
  }
  return value;
}

s32 pid_out_global;
u16 hall_value;
u8 speed_step_g;

char temp_buff[256];


void NEW_FOLLOW_LINE_TASK(u8* pFollowLineReset, s16 dir)
{
  static s32 curr_FollowLineBaseSpeed=0;//速度以PWM的数值表示

  if(PID_TimeOut==0)
  {
    //u8 speed_step;
    s32 pid_out;
    s32 left_speed=FOLLOW_LINE_MIN_SPEED;
    s32 right_speed=FOLLOW_LINE_MIN_SPEED;
    s32 expect_FollowLineBaseSpeed;
    s32 acc;
    
    PID_TimeOut=100;  //100ms一个调整周期
    
    //初始化
    if(*pFollowLineReset!=0) 
    {
      *pFollowLineReset=0;
      curr_FollowLineBaseSpeed=0;
    }
    
    hall_value = GetSensorMiddleIndex(&SENSOR_STATUS_New);
    
    //获取期望巡线的标准速度
    expect_FollowLineBaseSpeed = Get_FollowLineExpectBaseSpeed();
    expect_FollowLineBaseSpeed = lmin(expect_FollowLineBaseSpeed,FOLLOW_LINE_MAX_SPEED);
    expect_FollowLineBaseSpeed = lmax(expect_FollowLineBaseSpeed,FOLLOW_LINE_MIN_SPEED);
    acc = Get_FollowLineExpectAcc();
    /*
    //车体不正时的速度限制
    if((hall_value>(WONDER_MID_SENSOR_INDEX+5))||(hall_value<(WONDER_MID_SENSOR_INDEX-5)))
    {
      expect_FollowLineBaseSpeed = lmin(FOLLOW_LINE_NOT_MIDDLE_SPEED,expect_FollowLineBaseSpeed);
    }
    */
    
    if((curr_FollowLineBaseSpeed + acc) < expect_FollowLineBaseSpeed)
    {
      curr_FollowLineBaseSpeed += acc;
    }
    else if(curr_FollowLineBaseSpeed < expect_FollowLineBaseSpeed)
    {
      curr_FollowLineBaseSpeed = expect_FollowLineBaseSpeed;
    }
    else if((curr_FollowLineBaseSpeed - acc) < expect_FollowLineBaseSpeed)
    {
      curr_FollowLineBaseSpeed = expect_FollowLineBaseSpeed;
    }
    else
    {
      curr_FollowLineBaseSpeed -= acc;
    }
    
    
    pid_out = -SPEED_PID(WONDER_MID_SENSOR_INDEX * 10, hall_value * 10); //hall_value*10
    
    if(pid_out<-1000) pid_out=-1000;
    if(pid_out>1000) pid_out=1000;
    
    pid_out_global = pid_out;
    
    if(pid_out<0)//左偏，左慢右快
    {
      left_speed = curr_FollowLineBaseSpeed+((pid_out*curr_FollowLineBaseSpeed)/1000);
      right_speed = curr_FollowLineBaseSpeed;
    }
    else//右偏，左快右慢
    {
      left_speed = curr_FollowLineBaseSpeed;
      right_speed = curr_FollowLineBaseSpeed-((pid_out*curr_FollowLineBaseSpeed)/1000);
    }
    
    if(dir<0)
    {
      s16 a = left_speed;
      s16 b = right_speed;
      left_speed = -b;
      right_speed = -a;      
    }

    SetSpeedRate(LEFT_MOTO_INDEX, left_speed);
    SetSpeedRate(RIGHT_MOTO_INDEX, right_speed);
  }
}


//向电机驱动器发送指令，使能电机驱动器
void Enable_Moto_RS485(u8 moto_enum, u8 status)
{
  u16 cal_crc;
  status = (status)?1:0;  
  memcpy(moto_comm_buff,MODBUS_MOTO_ENBALE,sizeof(MODBUS_MOTO_ENBALE));
  moto_comm_buff[0] = moto_enum + 1;
  moto_comm_buff[5] = status;
  
  cal_crc=ModBus_CRC16_Calculate(moto_comm_buff , 6);
  moto_comm_buff[6]=cal_crc&0xFF;
  moto_comm_buff[7]=cal_crc>>8;
  FillUartTxBufN(moto_comm_buff,sizeof(MODBUS_MOTO_ENBALE),MOTO_COMM_PORT_ENUM);
}

void SetMotoStop(u8 moto_enum, u8 status)
{
  u16 cal_crc;
  status = (status)?1:0;  
  memcpy(moto_comm_buff,MODBUS_MOTO_STOP_SET,sizeof(MODBUS_MOTO_STOP_SET));
  moto_comm_buff[0] = moto_enum + 1;
  moto_comm_buff[5] = status?1:0;
  
  cal_crc=ModBus_CRC16_Calculate(moto_comm_buff , 6);
  moto_comm_buff[6]=cal_crc&0xFF;
  moto_comm_buff[7]=cal_crc>>8;
  FillUartTxBufN(moto_comm_buff,sizeof(MODBUS_MOTO_STOP_SET),MOTO_COMM_PORT_ENUM);
}

void SetMotoSpeedUpTime(u8 moto_enum, u16 time)
{
  u16 cal_crc;
  memcpy(moto_comm_buff,MODBUS_MOTO_UP_TIME_SET,sizeof(MODBUS_MOTO_UP_TIME_SET));
  moto_comm_buff[0] = moto_enum + 1;
  moto_comm_buff[4] = time>>8;
  moto_comm_buff[5] = time&0xFF;
  
  cal_crc=ModBus_CRC16_Calculate(moto_comm_buff , 6);
  moto_comm_buff[6]=cal_crc&0xFF;
  moto_comm_buff[7]=cal_crc>>8;
  FillUartTxBufN(moto_comm_buff,sizeof(MODBUS_MOTO_UP_TIME_SET),MOTO_COMM_PORT_ENUM);
}

void SetMotoSpeedDownTime(u8 moto_enum, u16 time)
{
  u16 cal_crc;
  memcpy(moto_comm_buff,MODBUS_MOTO_DOWN_TIME_SET,sizeof(MODBUS_MOTO_DOWN_TIME_SET));
  moto_comm_buff[0] = moto_enum + 1;
  moto_comm_buff[4] = time>>8;
  moto_comm_buff[5] = time&0xFF;
  
  cal_crc=ModBus_CRC16_Calculate(moto_comm_buff , 6);
  moto_comm_buff[6]=cal_crc&0xFF;
  moto_comm_buff[7]=cal_crc>>8;
  FillUartTxBufN(moto_comm_buff,sizeof(MODBUS_MOTO_DOWN_TIME_SET),MOTO_COMM_PORT_ENUM);
}

void SetMotoD01Rpm(u8 moto_enum,s16 d01rpm)
{
  u16 cal_crc;
  memcpy(moto_comm_buff,MODBUS_MOTO_RPM_SET,sizeof(MODBUS_MOTO_RPM_SET));
  moto_comm_buff[0] = moto_enum + 1;
  moto_comm_buff[4] = (d01rpm >> 8)&0xFF;
  moto_comm_buff[5] = d01rpm &0xFF;
  cal_crc=ModBus_CRC16_Calculate(moto_comm_buff , sizeof(MODBUS_MOTO_RPM_SET)-2);
  moto_comm_buff[6]=cal_crc&0xFF;
  moto_comm_buff[7]=cal_crc>>8;
  FillUartTxBufN(moto_comm_buff,sizeof(MODBUS_MOTO_RPM_SET), MOTO_COMM_PORT_ENUM);
}

//向电机驱动器发送指令，读取电机的转速，单位: 0.01RPM
void ReadMotoRpm(u8 moto_enum)
{
  u16 cal_crc;
  memcpy(moto_comm_buff,MODBUS_MOTO_RPM_READ,sizeof(MODBUS_MOTO_RPM_READ));  
  moto_comm_buff[0] = moto_enum + 1;
  cal_crc=ModBus_CRC16_Calculate(moto_comm_buff , sizeof(MODBUS_MOTO_RPM_READ)-2);
  moto_comm_buff[6]=cal_crc&0xFF;
  moto_comm_buff[7]=cal_crc>>8;  
  FillUartTxBufN(moto_comm_buff,sizeof(MODBUS_MOTO_RPM_READ), MOTO_COMM_PORT_ENUM);
}

/************************************
 ** 电机驱动器的控制任务
 ** 功能：
 **  1. 初始化，使能电机驱动器
 **  2. 设置电机的速度 rpm
 **  3. 电机速度为0后，持续[n]秒钟电机自由(失能电机驱动器)
 ***********************************/
void MOTO_SPEED_CONTROL_TASK(void)
{
  static s16 moto_speed_in_rpm_bk[MOTO_NUM] = {0,0};
  static u32 moto_disable_time_counter[MOTO_NUM] = {0,0};
  static u8 pro = 0;
  static u8 moto_enum = 0;
  static u8 InitIndex = 0;
  switch(pro)
  {
  case 0: // Init : [set_free]
    {
      if(MOTO_485COMM_Timeout == 0)
      {
        if(moto_enum < MOTO_NUM)
        {
          switch(InitIndex)
          {
          case 0:
            {
              SetMotoSpeedUpTime(moto_enum, MOTO_SPEED_UP_DOWN_DELAY_TIME); 
            }
            break;
          case 1:
            {
              SetMotoSpeedDownTime(moto_enum, MOTO_SPEED_UP_DOWN_DELAY_TIME);
            }
            break;
          case 2:
            {
              moto_enable_status[moto_enum] =0;
              Enable_Moto_RS485(moto_enum, moto_enable_status[moto_enum]);   
            }
            break;
          default: ;
          }
          MOTO_485COMM_Timeout = MOTO_CONTROL_CYCLE;
          moto_enum += 1;
        }
        else
        {
          moto_enum = 0;
          InitIndex += 1;
          if(InitIndex>=3) pro += 1;
        }
      }
    }
    break;
  case 1: // Init : [set_speed]
    {
      if(MOTO_485COMM_Timeout == 0)
      {
        if(moto_enum < MOTO_NUM)
        {
          MOTO_485COMM_Timeout = MOTO_CONTROL_CYCLE;
          moto_speed_in_rpm_bk[moto_enum] = 0;
          moto_speed_in_rpm[moto_enum] =0;
          SetMotoD01Rpm(moto_enum, moto_speed_in_rpm_bk[moto_enum]);
          SetMotoRpmTimes[moto_enum] += 1;
          moto_enum += 1;
        }
        else
        {
          moto_enum = 0;
          pro += 1;
        }
      }
    }
    break;    
  case 2: //nomal : [set_enable] + [set_speed]
    {
      if(MOTO_485COMM_Timeout == 0)
      {
        if(moto_enum < MOTO_NUM)
        {
          if(moto_speed_in_rpm_bk[moto_enum] != moto_speed_in_rpm[moto_enum])
          {
            if((moto_speed_in_rpm[moto_enum] != 0)&&(moto_enable_status[moto_enum] == 0))
            {
              MOTO_485COMM_Timeout = MOTO_CONTROL_CYCLE;
              moto_enable_status[moto_enum] = 1;
              Enable_Moto_RS485(moto_enum, moto_enable_status[moto_enum]);
              break;
            }
            MOTO_485COMM_Timeout = MOTO_CONTROL_CYCLE;
            moto_speed_in_rpm_bk[moto_enum] = moto_speed_in_rpm[moto_enum];
            SetMotoD01Rpm(moto_enum, moto_speed_in_rpm_bk[moto_enum]);
            SetMotoRpmTimes[moto_enum] += 1; 
          }
          moto_enum += 1;
        }
        else
        {
          moto_enum = 0;
          pro += 1;
        }        
      }
    }
    break;
  case 3: // nomal : [set_free]
    {
      if(MOTO_485COMM_Timeout == 0)
      {
        if(moto_enum < MOTO_NUM)
        {
          if(moto_enable_status_change_flag[moto_enum] != 0)
          {
            MOTO_485COMM_Timeout = MOTO_CONTROL_CYCLE;
            moto_enable_status_change_flag[moto_enum] = 0;
            Enable_Moto_RS485(moto_enum, moto_enable_status[moto_enum]);     
          }
          moto_enum += 1;
        }
        else
        {
          moto_enum = 0;
          pro += 1;
        }
      }
    }
    break;
  case 4: // nomal : [Read RPM]
    {
      if(MOTO_485COMM_Timeout == 0)
      {
        if(moto_enum < MOTO_NUM)
        {
            if(MOTO_READ_RPM_Timeout[moto_enum] == 0)
            {
              MOTO_485COMM_Timeout = MOTO_CONTROL_CYCLE;
              MOTO_READ_RPM_Timeout[moto_enum] = DEFAULT_MOTO_READ_RPM_TIME_OUT;
              ReadMotoRpm(moto_enum);
              ReadMotoRpmTimes[moto_enum] += 1;
            }
            moto_enum += 1;
        }
        else
        {
          moto_enum = 0;
          pro += 1; //pro = 2;
        }        
      }
    }
    break;
  case 5: // set speed up down time
    {
      if(MOTO_485COMM_Timeout == 0)
      {
        if(moto_enum < MOTO_NUM)
        {
            if(moto_reset_speed_up_down_time_flag[moto_enum] != 0)
            {
              MOTO_485COMM_Timeout = MOTO_CONTROL_CYCLE;
              SetMotoSpeedUpTime(moto_enum, MOTO_SPEED_UP_DOWN_DELAY_TIME);
            }
            moto_enum += 1;
        }
        else
        {
          moto_enum = 0;
          pro += 1;
        }        
      }
    }
    break;
  case 6: // set speed down time
    {
      if(MOTO_485COMM_Timeout == 0)
      {
        if(moto_enum < MOTO_NUM)
        {
            if(moto_reset_speed_up_down_time_flag[moto_enum] != 0)
            {
              moto_reset_speed_up_down_time_flag[moto_enum] = 0;
              MOTO_485COMM_Timeout = MOTO_CONTROL_CYCLE;
              SetMotoSpeedDownTime(moto_enum, MOTO_SPEED_UP_DOWN_DELAY_TIME);
            }
            moto_enum += 1;
        }
        else
        {
          moto_enum = 0;
          pro = 2;
        }        
      }
    }
    break;    
  case 7://useless
    {
      static u8 IM_STOP_Flag[MOTO_NUM]={0,0};
      static u8 IM_STOP_Status[MOTO_NUM]={0,0};
      if(MOTO_485COMM_Timeout == 0)
      {
        if(moto_enum < MOTO_NUM)
        {
            if(IM_STOP_Flag[moto_enum] != 0)
            {
              MOTO_485COMM_Timeout = MOTO_CONTROL_CYCLE;
              IM_STOP_Flag[moto_enum] = 0;
              SetMotoStop(moto_enum, IM_STOP_Status[moto_enum]);
            }
            moto_enum += 1;
        }
        else
        {
          moto_enum = 0;
          pro = 2;
        }        
      }
    }
    break;
  default: pro = 0;
  }
  
  //电机速度为0后，持续[n]秒钟电机自由
  if(1)
  {
    static u32 NumOfSysTickIntBk = 0;
    u8 i;
    if(NumOfSysTickInt!=NumOfSysTickIntBk)
    {
      NumOfSysTickIntBk = NumOfSysTickInt;
      for(i = 0; i < MOTO_NUM; i++)
      {
        if(moto_speed_in_rpm_bk[i] == 0)
        {        
          if(moto_enable_status[i] != 0)
          {
            moto_disable_time_counter[i] += 1;
            if(moto_disable_time_counter[i] >= MOTO_ZERO_FREE_TIME_IN_MS)
            {
              moto_disable_time_counter[i] = 0;
              moto_enable_status[i] = 0;
              
              moto_enable_status_change_flag[i] = 1;
              //Enable_Moto_RS485(i,moto_enable_status[i]);
              //SetBeep(1,1000,50);
            }
          }
        }
        else
        {
          moto_disable_time_counter[i] = 0;
        }
      }
    }
  }
}

// return : 0 - stop_status , 1 - Runing
u8 SLOW_DOWN_EX_Task(u8* reset, s16 distance_cm)
{
#define MAX_SLOW_DOWN_ACC_CMPSS  50.0   // 加速度50.0cm/ss
  static s16 slow_value_every_time[MOTO_NUM] = {0, 0};
  static s16 Speed_bk[MOTO_NUM];
  u8 status = 1;
  u8 i;
  if(PID_TimeOut==0)
  {
    PID_TimeOut=100;
    if(*reset)
    {
      *reset=0;
      float D1Rpm = 0.0; 
      float speed_mms;

      /*
      if(time_in_ms<100) time_in_ms=100;
      for(i = 0; i < MOTO_NUM; i++)
      {
        Speed_bk[i] = RealRpm[i];
        slow_value_every_time[i] = (s32)RealRpm[i] / ((s32)time_in_ms/100);
        if(slow_value_every_time[i] == 0) slow_value_every_time[i] = RealRpm[i];
      }     
      */
    }
    else
    {
      if((RealRpm[LEFT_MOTO_INDEX] != 0) || (RealRpm[RIGHT_MOTO_INDEX] != 0))    
      {
        for(i = 0; i < MOTO_NUM; i++)
        {
          if(abs(Speed_bk[i]) <= abs(slow_value_every_time[i]))
          {
            Speed_bk[i] = 0;
          }
          else 
          {
            Speed_bk[i] -= slow_value_every_time[i];
          }
          SetD1Rpm((MOTO_INDEX_ENUM)i, Speed_bk[i]);
        }   
      }
      else
      {
        for(i = 0;i < MOTO_NUM; i++)
        {
          SetD1Rpm((MOTO_INDEX_ENUM)i, 0);
        }    
        status = 0;
      }
    }
  }
  return status;
}