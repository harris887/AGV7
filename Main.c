/* Includes ------------------------------------------------------------------*/
#include "User\user_inc.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define MAIN_PRINTF_DEBUG   1

char test_buffer[256];
int main(void)
{
  RCC_Configuration();
  NVIC_Configuration();
  GPIO_Configuration();
  //FLASH_READOUT_Protect();//harris 20160522
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
  
  SetBeep(3,100,200);//test use
  
#if (MAIN_PRINTF_DEBUG)
  printf("---- AGV7 Mining ! ----\n");
#endif
  
  while(1)
  {
    UART_Task();
    MODBUS_READ_HALL_SERSOR_TASK();
    //CHECK_BUTTON_TASK();
    //CheckBatteryVolt_TASK();
    JOYSTICK_SCAN_TASK();
    CHECK_REMOTE_ENABLE_TASK();
    
    BEEP_TASK();
    
    AGV_RUN_Task();
    READ_RFID_BLOCK_Task();
    //MOD_BUS_REG_MODIFY_Check();
    
    MOTO_SPEED_CONTROL_TASK();
    VOICE_PLAY_TASK();
    Laser_Task();
    Check_DIDO_TASK();
    WK2124_TransTask();
    
    TimeoutJump();
    FeedDog();     
    //if(0)
    if(debug_show)
    {
      static s8 o_index = 0;
      //static u8 sta=0;
      debug_show=0;

      if(USART_BYTE == 'Q')
      {
        USART_BYTE = 0;
        printf("pro = %d \n", AGV_RUN_Pro);
      }      
      
      if(USART_BYTE == 'S')
      {
        USART_BYTE = 0;
        FollowLineEnable = 1;
        Run_Dir = DIR_FORWARD;
        MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR;
        MB_LINE_DIR_SELECT = 1;
        printf("Start FOLLOW\n");
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
          FillUartTxBufN((u8*)test_buffer,strlen(test_buffer),1);
        }
        else if(USART_BYTE == 'B')
        {
          sprintf(test_buffer,"RT: [ %d %d ] , RRT: [ %d %d ] , TOTAL: %d \n", // , %d
                  ReadMotoRpmTimes[LEFT_MOTO_INDEX] ,ReadMotoRpmTimes[RIGHT_MOTO_INDEX] ,
                  MONITOR_St[LEFT_MOTO_INDEX].counter ,MONITOR_St[RIGHT_MOTO_INDEX].counter , 
                  MODBUS_Monitor.read_success_num);// rx5
          FillUartTxBufN((u8*)test_buffer,strlen(test_buffer),1);
        }
        /*
        else if(USART_BYTE == 'C')
        {
          u8 bff[32];
          u8* bf;
          USART_BYTE = 0;
          memcpy(bff, UART5_Oprx.Buf, 32); 
          
          bf = bff;
          sprintf(test_buffer,"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n",
                  bf[0], bf[1], bf[2], bf[3], bf[4], bf[5], bf[6], bf[7], bf[8], bf[9], bf[10], bf[11], bf[12], bf[13], bf[14], bf[15]);
          FillUartTxBufN((u8*)test_buffer, strlen(test_buffer), 1);
          
          bf = bff + 16;
          sprintf(test_buffer,"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n",
                  bf[0], bf[1], bf[2], bf[3], bf[4], bf[5], bf[6], bf[7], bf[8], bf[9], bf[10], bf[11], bf[12], bf[13], bf[14], bf[15]);
          FillUartTxBufN((u8*)test_buffer, strlen(test_buffer), 1);
        }
        */
      }   
      if(USART_BYTE == 'h')
      {
        USART_BYTE = 0;
        if(MODE_BUS_HALL_Addr == BACKWARD_MODE_BUS_HALL_ADDR)
        {
          MODE_BUS_HALL_Addr = DEFAULT_MODE_BUS_HALL_ADDR ;
        }
        else
        {
          MODE_BUS_HALL_Addr = BACKWARD_MODE_BUS_HALL_ADDR;
        }
        printf("MODE_BUS_HALL_Addr = %d\n", MODE_BUS_HALL_Addr);
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
      if(0)
      {
        printf("batt_ad=%d , speed_ad=%d\n",adc_data[0],AD_Roller);
      }
      //printf("e=%d \n",hall_error_num);
      //printf("%d ,%d ,%d \n",(s32)hall_value-WONDER_MID_SENSOR_INDEX \
             ,pid_out_global,speed_step_g);
      if(0)
      {
        printf("%x %x",GET_BUTTON_VT_STATUS(), GetVt() );
      }


      
      
      if(USART_BYTE == 'R')
      {
        //USART_BYTE = 0;
        printf("RFID_ReadBlockSuccessTimes %d , rx = %d %d\n", RFID_ReadBlockSuccessTimes, rx2, rx3);
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











