/* Includes ------------------------------------------------------------------*/
#include "User\user_inc.h"
#include "stdio.h"

#define MAIN_PRINTF_DEBUG   1

int main(void)
{
  RCC_Configuration();
  NVIC_Configuration();
  GPIO_Configuration();
  //FLASH_READOUT_Protect();//harris 20160522
  BackupAccessEnable();
  
  //MovementListInit();
  BUTTON_Init();
  BUZZER_Init();
  RELAY_Init();
  LedDispInit();
  Adc_init();
  MOTO_Init();
  SysTick_Init(1);
  REMOTE_Init();//Ò£¿Ø²¿·Ö
  Usart1_Init();
  Usart2_Init();
  Usart3_Init();
  Usart4_Init();
  Usart5_Init();
  LASER_INFOR_Init();
  CAN1_Init(CAN_BOUND_250, CAN_Mode_Normal);
  WK2124_Init();
  GetFlashModBusData(&MOD_BUS_Reg);
  SetBeep(3,100,200);//test use
  
#if (MAIN_PRINTF_DEBUG)
  printf("---- AGV7 Mining ! ----\n");
#endif
  
  while(1)
  {
    UART_Task();
    MODBUS_READ_SERSOR_BOARD_TASK();
    //CHECK_BUTTON_TASK();
    //CheckBatteryVolt_TASK();
    //JOYSTICK_SCAN_TASK();
    //CHECK_REMOTE_ENABLE_TASK();
    
    BEEP_TASK();
    
    //AGV_RUN_Task();
    //MOTO_FaultCheck_TASK();
    READ_RFID_BLOCK_Task();
    //MOD_BUS_REG_MODIFY_Check();
    
    MOTO_SPEED_CONTROL_TASK();
    VOICE_PLAY_TASK();
    //Check_UltraSonic_TASK();
    Laser_Task();
    Check_DIDO_TASK();
    WK2124_TransTask();
    
    TimeoutJump();
    FeedDog();     
    //if(0)
    if(debug_show)
    {
      //static u8 sta=0;
      debug_show=0;
      if(0)
      //if(1)
      {
        printf("485 data here!\n");
        printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
             HallValue[0],HallValue[1],HallValue[2],HallValue[3],
             HallValue[4],HallValue[5],HallValue[6],HallValue[7],
             HallValue[8],HallValue[9],HallValue[10],HallValue[11],
             HallValue[12],HallValue[13],HallValue[14],HallValue[15]
             );        
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

      if(0)
      {
        printf("pro = %d \n",AGV_RUN_Pro);
      }
      
      
      if(0)
      {
        printf("RFID_ReadBlockSuccessTimes %d\n",RFID_ReadBlockSuccessTimes);
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











