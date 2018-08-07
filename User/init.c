#include "user_inc.h"
#include "math.h"

#define INIT_PRINTF_DEBUG  0
u8 JumpFlag=0;
u16 JumpTimer=0;


void RCC_Configuration(void)
{
  ErrorStatus HSEStartUpStatus;

  /* RCC system reset(for debug purpose) */
  RCC_DeInit();


  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  
  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);

    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1);

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

    /* ADCCLK = PCLK2/6 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);

    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* PLLCLK = 8MHz * 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
          
    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }

  
  /* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA 
                         | RCC_APB2Periph_GPIOB 
                         | RCC_APB2Periph_GPIOC 
                         | RCC_APB2Periph_GPIOD
                         | RCC_APB2Periph_GPIOE 
                         | RCC_APB2Periph_AFIO 
                         | RCC_APB2Periph_USART1 
                         | RCC_APB2Periph_ADC1 
                         | RCC_APB2Periph_TIM1 
                         | RCC_APB2Periph_TIM8 , ENABLE);

  /*使能定时器的时钟*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2
                         | RCC_APB1Periph_TIM3
                         | RCC_APB1Periph_TIM4
                         | RCC_APB1Periph_TIM5
                         | RCC_APB1Periph_CAN
                         | RCC_APB1Periph_USART2
                         | RCC_APB1Periph_USART3
                         | RCC_APB1Periph_UART4
                         | RCC_APB1Periph_SPI2  
                         | RCC_APB1Periph_UART5, ENABLE);

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
}


/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures the NVIC and Vector Table base address.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4000);
  /* 中断优先级模式 */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  
  
  //遥控器引进中断
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);    
  
  /* 串口1中断优先级 平时测试用*/
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
  
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);    
  
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);    
  
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);      
  
  NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);     
  
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN_RX0_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 11;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  

  /*AD转换的DMA中断*/
  NVIC_InitStructure.NVIC_IRQChannel = ADC_DMA_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 

}



void GPIO_Configuration(void)
{
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);
}





/*页大小定义*/

void FLASH_READOUT_Protect(void)
{
    if(FLASH_GetReadOutProtectionStatus() != SET)
    {
              FLASH_Unlock();
              FLASH_ReadOutProtection(ENABLE);
              FLASH_Lock();
    }
}



/*保存参数的地址*/
#define ConfigDataAddr      ((u32)(0x08010000-FLASH_PAGE_SIZE))



/*保存数据*/
FLASH_Status SaveFlashModBusData(MOD_BUS_REG* pMOD_BUS_Reg)
{   
    u8 i;
    u16 *ptr;
    
    FLASH_Status status = FLASH_COMPLETE;
    /* FLASH解锁*/
    FLASH_Unlock(); 

    /* 清除FLASH标志位 */
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	
    
    status = FLASH_ErasePage(ConfigDataAddr);/*擦出FLASH*/
    
    if(status!=FLASH_COMPLETE) 
    {
      return status;
    }
    
    ptr = (void*)pMOD_BUS_Reg;//
    for( i=0; i<sizeof(MOD_BUS_REG); i+=2 )
    {
      status = FLASH_ProgramHalfWord(ConfigDataAddr+i, *ptr++);
    }
    
    /*锁上FLASH*/
    FLASH_Lock();
    return status;
}

void GetFlashModBusData(MOD_BUS_REG* pMOD_BUS_Reg)
{
    u8 i;
    u16 *ptr;
    u8 retry=2;
LOAD_MOD_BUS_DATA:   
    ptr = (void*)pMOD_BUS_Reg;//
    for( i=0; i<sizeof(MOD_BUS_REG); i+=2 )
    {
        *ptr++=*(u16*)(ConfigDataAddr+i);
    }
    if(pMOD_BUS_Reg->MOD_REG_MAGIC_WORD!=MAGIC_WORD)
    {
      SaveFlashModBusData((void*)&DEFAULT_MOD_BUS_Reg);
#if (INIT_PRINTF_DEBUG)
      printf("LOAD DEFAULT MOD BUS REG TO FLASH!\r\n");
#endif
      if(retry--)
      {
        goto LOAD_MOD_BUS_DATA;
      }
      else
      {
#if (INIT_PRINTF_DEBUG)
        printf("LOAD_MOD_BUS_DATA ERROR!\r\n");
#endif
        memcopy((void*)&DEFAULT_MOD_BUS_Reg,(void*)ptr,sizeof(MOD_BUS_REG));
      }
    }
    else
    {
#if (INIT_PRINTF_DEBUG)
      printf("LOAD_MOD_BUS_DATA OK!\r\n");
#endif
      //配置MODBUS 波特率
      if(pMOD_BUS_Reg->COMM_BD<MOD_BUS_BD_LIST_LENGTH)
      {
        Usart2_Init_op(MOD_BUS_BD_LIST[pMOD_BUS_Reg->COMM_BD]);
        //printf("MOD BUS SPEED PARA %ld,N,8,1\r\n",MOD_BUS_BD_LIST[pMOD_BUS_Reg->COMM_BD]);
      }
    }
    //默认值，20160608
    //pMOD_BUS_Reg->AUTO_FOLLOW_SPEED=30;//30%巡线速度

    MOD_BUS_REG_Backup();
}

void BackupAccessEnable(void)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR  | RCC_APB1Periph_BKP,ENABLE);
  PWR_BackupAccessCmd(ENABLE);
  BKP_ClearFlag();
  
  BKP_WriteBackupRegister(BKP_DR1, BOOT_STANDARD_FLAG);//for user arear only
}

void SetTimeoutJump(u16 time)
{
  JumpTimer = time;
  JumpFlag = 1;
}

void TimeoutJump(void)
{
  if((JumpFlag==1)&&(JumpTimer==0))
  {
    JumpFlag=0;
#if (INIT_PRINTF_DEGUG)
    printf("JUMP TO BOOT\r\n");
#endif
    USART_Cmd(USART1, DISABLE);
    USART_Cmd(USART3, DISABLE);
    SysTick_ITConfig(DISABLE);
    SysTick_CounterCmd(SysTick_Counter_Disable); 
    ADC_Cmd(ADC1, DISABLE); 
    NVIC_GenerateSystemReset();/*软件复位到BOOTLOADER区,也起到了关闭看门狗的作用*/    
  }
}

void FeedDog(void)
{
  IWDG_ReloadCounter();
}