/* Includes ------------------------------------------------------------------*/
#include "User\user_inc.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

char test_buffer[256];
static u16 AUTO_FOLLOW_LINE_timers = 0;

int main(void)
{
  RCC_Configuration();
  NVIC_Configuration();
  GPIO_Configuration();
  FLASH_READOUT_Protect();//harris 20160522
  BackupAccessEnable();
  
  MovementListInit();
  //BUTTON_Init();
  BUZZER_Init();
  //RELAY_Init();
  //LedDispInit();
  Adc_init();
  SysTick_Init(1);
  REMOTE_Init();//遥控部分
  Usart1_Init();
  Usart2_Init();
  Usart3_Init();
  Usart4_Init();
  Usart5_Init();
  GetFlashModBusData(&MOD_BUS_Reg);
  MOTO_Init();
  LASER_INFOR_Init();
  CAN1_Init(CAN_BOUND_250, CAN_Mode_Normal);
  WK2124_Init();
 
  //SetBeep(3,100,200);//test use
  if(LOG_Level <= LEVEL_INFO) printf("---- AGV7 Mining ! ----\n");
  if(LOG_Level <= LEVEL_INFO) printf("Build Time: %s, %s\n", __DATE__, __TIME__);

  while(1)
  {
    UART_Task();
    MODBUS_READ_HALL_SERSOR_TASK();
    //CHECK_BUTTON_TASK();
    CheckBatteryVolt_TASK();
    JOYSTICK_SCAN_TASK();
    CHECK_REMOTE_ENABLE_TASK();
    
    BEEP_TASK();
    
    AGV_RUN_Task();
    READ_RFID_BLOCK_Task();
    MOD_BUS_REG_MODIFY_Check();
    
    MOTO_SPEED_CONTROL_TASK();
    VOICE_PLAY_TASK();
    Laser_Task();
    Check_DIDO_TASK();
    WK2124_TransTask();
    BMS_Task();
    CHARGE_Task();
    
    //-- 自動巡航循環 --//
    if(MOD_BUS_Reg.FOLLOW_LOOP_TIME_IN_MS != 0)
    {
      if((FOLLOW_LOOP_Timeout == 0) 
        && (AGV_RUN_Pro == AGV_STATUS_IDLE))
      {
        if(StartFollowLine())
        {
          FOLLOW_LOOP_Timeout = MOD_BUS_Reg.FOLLOW_LOOP_TIME_IN_MS;
          if(LOG_Level <= LEVEL_INFO) printf("++++Start FOLLOW %d times\n", ++AUTO_FOLLOW_LINE_timers);
        }         
      }
    }
    
    TimeoutJump();
    FeedDog();     
    //if(0)
    if((debug_show) && (LOG_Level <= LEVEL_DEBUG))
    {
      static u8 FollowLineLoop = 0;
      static s8 o_index = 0;
      debug_show=0;
      if(USART_BYTE == 'z')
      {
        USART_BYTE =0;
        printf("Build Time: %s, %s\n", __DATE__, __TIME__);
      }
      
      if(USART_BYTE == 'C')
      {
        printf("[CHARGE] Vol: %d, Cur:%d, Cap:%d, Tim:%d, Comm:%d Counter:%d\n", CHARGE_St.Voltage, \
           CHARGE_St.Current, \
           CHARGE_St.Cap,     \
           CHARGE_St.Time,    \
           CHARGE_St.RxNum,   \
           CHARGE_COMM_Counter);  
      }        
      
      if(USART_BYTE == 'T')
      {
        //USART_BYTE = 0;
        printf("PACK_Vol = %d, PACK_Current = %d, PACK_Left = %d, PACK_Cap = %d, Temp = %d, M_BAT_Precent = %d [%d, %d]\n", PACK_ANALOG_Infor.PACK_Vol, \
          *(s16*)&PACK_ANALOG_Infor.PACK_Current, \
          PACK_ANALOG_Infor.PACK_Left, \
          PACK_ANALOG_Infor.PACK_TotalCap, \
          PACK_ANALOG_Infor.TEMP_Value[0], \
          M_BAT_Precent, \
          PACK_ANALOG_Infor.COMM_Num, \
          PACK_WARN_Infor.COMM_Num);
      }         

      if(USART_BYTE == 'Q')
      {
        USART_BYTE = 0;
        printf("pro = %d \n", AGV_RUN_Pro);
      }      
      
      if(USART_BYTE == 's')
      {
        USART_BYTE = 0;
        FollowLineLoop = 1;
        printf("FollowLineLoop_1\n");
      }
      if(USART_BYTE == 't')
      {
        USART_BYTE = 0;
        FollowLineLoop = 0;
        printf("FollowLineLoop_0\n");
      }
      
      if(FollowLineLoop != 0)  
      {
        static u16 cycle_s = 0; // 600
        if(cycle_s == 0)
        {
          if(AGV_RUN_Pro == AGV_STATUS_IDLE)
          {
            if(StartFollowLine())
            {
              cycle_s = 600;
              printf("++Start FOLLOW %d times\n", ++AUTO_FOLLOW_LINE_timers);
            }          
          }
        }
        else
        {
          cycle_s -= 2;
        }
      }
      
      if(USART_BYTE == 'S')
      {
        USART_BYTE = 0;

        if(StartFollowLine())
        {
          printf("Start FOLLOW ONCE\n");
        }
      }      
      
      if(USART_BYTE == 'b')
      {
        USART_BYTE = 0;      
        SetBeep(3,100,200);
      }
      
      if(USART_BYTE == 'f')
      {
        USART_BYTE = 0;      
        SET_DIDO_Relay(DO_Fan_1, 0);
        SET_DIDO_Relay(DO_Fan_2, 0);
      }   
      
      if(USART_BYTE == 'F')
      {
        USART_BYTE = 0;      
        SET_DIDO_Relay(DO_Fan_1, 1);
        SET_DIDO_Relay(DO_Fan_2, 1);        
      }       
      
      if(USART_BYTE == 'y')
      {
        USART_BYTE = 0;
        printf("DIDO %d Off\n", o_index);
        SET_DIDO_Relay(o_index--, 0);
        if(o_index < 0) o_index = 7;
        else if(o_index > 7) o_index = 0;
      }        
      if(USART_BYTE == 'Y')
      {
        USART_BYTE = 0;
        printf("DIDO %d On\n", o_index);
        o_index++;
        SET_DIDO_Relay(o_index++, 1);
        if(o_index < 0) o_index = 7;
        else if(o_index > 7) o_index = 0;   
      }      
      if(1)// 电机测试
      {
        if(USART_BYTE == 'Z')
        {
          //USART_BYTE = 0;
          printf("DIDO ack_num : %d, DI = %02X\n",
                 MODBUS_Dido.read_success_num, 
                 DIDO_INPUT_Status.LightStatus );        
        }           
        if(USART_BYTE == 'A')
        {
          sprintf(test_buffer,"Speed_mmps: [ %d %d ], L_001rpm: [ %d %d ] \n",
                  MONITOR_St[LEFT_MOTO_INDEX].real_mms, MONITOR_St[RIGHT_MOTO_INDEX].real_mms,
                  MONITOR_St[LEFT_MOTO_INDEX].real_rpm_reg, MONITOR_St[RIGHT_MOTO_INDEX].real_rpm_reg);
          FillUartTxBufN((u8*)test_buffer,strlen(test_buffer), 2);
        }
        if(USART_BYTE == 'a')
        {
          printf("speed_roller_rate : %d\n", Get_ANALOG_SD_Speed() / 5);   
        }
        
        
        else if(USART_BYTE == 'B')
        {
          sprintf(test_buffer,"RT: [ %d %d ] , RRT: [ %d %d ] , TOTAL: %d \n", // , %d
                  ReadMotoRpmTimes[LEFT_MOTO_INDEX] ,ReadMotoRpmTimes[RIGHT_MOTO_INDEX] ,
                  MONITOR_St[LEFT_MOTO_INDEX].counter ,MONITOR_St[RIGHT_MOTO_INDEX].counter , 
                  MODBUS_Monitor.read_success_num);// rx5
          FillUartTxBufN((u8*)test_buffer,strlen(test_buffer), 2);
        }
      }   
      if(USART_BYTE == 'h')
      {
        USART_BYTE = 0;
        HALL_DEBUG_Print();
        /*
        if(MODE_BUS_HALL_Addr == BACKWARD_MODE_BUS_HALL_ADDR)
        {
          MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR ;
        }
        else
        {
          MODE_BUS_HALL_Addr = BACKWARD_MODE_BUS_HALL_ADDR;
        }
        printf("MODE_BUS_HALL_Addr = %d\n", MODE_BUS_HALL_Addr);
        */
      }
      
      if(USART_BYTE == 'H')
      {
        //printf("485 data here!\n");
        printf("HALL [%d] : %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", MODBUS_HallSensor.read_success_num,
             HallValue[0],HallValue[1],HallValue[2],HallValue[3],
             HallValue[4],HallValue[5],HallValue[6],HallValue[7],
             HallValue[8],HallValue[9],HallValue[10],HallValue[11],
             HallValue[12],HallValue[13],HallValue[14],HallValue[15]
             );        
      }
      
      if(USART_BYTE == 'P')
      {
        static u8 v_index = 0;
        USART_BYTE = 0;
        Play_Warning(v_index);
        printf("Play_Warning %d\n", v_index);
        v_index += 1;
        if(v_index >= VOICE_NUM) v_index = 0;
      }
      
      if(USART_BYTE == 'R')
      {
        //USART_BYTE = 0;
        printf("RFID_ReadBlockSuccessTimes %d , rx = %d\n", RFID_ReadBlockSuccessTimes, rx3);
      }
      
      if(USART_BYTE == 'U')
      {
        USART_BYTE = 0;
        printf("Update Program!\n");   
        BKP_WriteBackupRegister(BKP_DR1, TO_BOOT_UPDATE_FLAG);
        SetTimeoutJump(200);
      }      

      if(USART_BYTE == 'L')
      {
        printf("%05d : [%d, %d] [%d, %d]\n", CAN_RX_IntNum, \
          LASER_Infor[0].heart_beat_num,  \
          LASER_Infor[0].data_infor_num,  \
          LASER_Infor[1].heart_beat_num,  \
          LASER_Infor[1].data_infor_num );
      }     
      if((USART_BYTE == '0') || (USART_BYTE == '1') || (USART_BYTE == '2'))
      {
        LaserSelect = USART_BYTE - '0';
        printf("LaserSelect %d\n", USART_BYTE - '0');
        USART_BYTE = 0;
      }
      if(USART_BYTE == '3')
      {
        Set_LASER(0x103, 1, 70, 200);
        Set_LASER(0x203, 1, 70, 200);
        printf("Laser all on\n");
        USART_BYTE = 0;
      }
      if(USART_BYTE == '4')
      {
        Set_LASER(0x103, 0, 70, 200);
        Set_LASER(0x203, 0, 70, 200);
        printf("Laser all off\n");
        USART_BYTE = 0;
      }      
    }
  }//end of while(1)
}








/*
#if (0)
  u8 i;
  SendBmsCommand(DEFAULT_BMS_ADDR, BMS_CID2_GET_PACK_ANALOG, 0x01); 
  SendBmsCommand(DEFAULT_BMS_ADDR, BMS_CID2_GET_PACK_WARNING, 0x01);
  SendBmsCommand(DEFAULT_BMS_ADDR, BMS_CID2_BAT_CHARGE_MOS_CONTROL, BMS_CHARGE_MOS_CHANGE_FLAG | BMS_CHARGE_MOS_CLOSE);
  BMS_CURRENT_Cmd = BMS_CID2_GET_PACK_ANALOG;
  for(i = 0; i < strlen(BMS_INFOR_ACK); i++)
  {
    Handle_BmsRx(BMS_INFOR_ACK[i]);
  }
  BMS_CURRENT_Cmd = BMS_CID2_GET_PACK_WARNING;
  for(i = 0; i < strlen(BMS_WARN_ACK); i++)
  {
    Handle_BmsRx(BMS_WARN_ACK[i]);
  }
#endif
*/


