#include "user_inc.h"
#include "string.h"

#define RFID_COMM               3

#define RFID_CMD_HEADER 0x7F
#define RFID_CMD_MIN_LENGTH 2
#define RFID_CMD_MAX_LENGTH 0x7E

u8 RfidMachineState=0;
u8 DATA_7F_Num=0;
u8 RFID_COMEIN_Flag=0;
u8 RFID_DATA_Buf[256];
u8 RFID_DATA_BufIndex=0;
//u16 RFID_ReadBlockDelay=0;
u32 RFID_ReadBlockTimes=0;
u32 RFID_ReadBlockSuccessTimes=0;
u32 RFID_ReadBlockUnSuccessTimes=0;
u8 RFID_ReadBlockFailFlag=0;
u8 RFID_ReadBlockRetryNum=0;
u16 RFID_ReadBlockTimeout=0;
//��Ƭ��Ϣ
u16 RFID_Type;
u8 RFID_CARD_ID[4];
u8 RFID_BLOCK_Data[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const u8 READ_BLOCK_ONE[11]={0x7F ,0x09 ,0x14 ,0x01 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0x1C};
//u16 RFID_ONLINE_Timeout=0;
//u16 RFID_ONLINE_Flag=0;
u16 PlaceId=0;

void Analysis_Receive_From_RFID(u8 data)
{
  switch(RfidMachineState)
  {
  case 0:
    {
    //ANA_Start:
      if(data==RFID_CMD_HEADER)
      {
        RfidMachineState+=1;
        RFID_DATA_BufIndex=0;
        RFID_DATA_Buf[RFID_DATA_BufIndex++]=data;
      }
    }
    break;
  case 1:
    {
      if((data>=RFID_CMD_MIN_LENGTH)&&(data<=RFID_CMD_MAX_LENGTH))
      {
        RFID_DATA_Buf[RFID_DATA_BufIndex++]=data;
        DATA_7F_Num=0;
        RfidMachineState+=1;
      }
      else
      {
        RfidMachineState=0;
      }
    }
    break;
  case 2:
    {
      if(data==0x7F)
      {
        DATA_7F_Num+=1;
        if(DATA_7F_Num==1) return;
        else DATA_7F_Num=0;
      }
      RFID_DATA_Buf[RFID_DATA_BufIndex++]=data;
      if(RFID_DATA_BufIndex>=(RFID_DATA_Buf[1]+2))
      {
        RfidMachineState=0;
        //��������
        HandlerRfidCmd(RFID_DATA_Buf ,RFID_DATA_BufIndex);
      }
    }
    break;
  }
}

u8 RFID_checkSum(u8* data, u8 length)
{
  u8 temp = 0;
  u8 i;
  for (i = 0; i < length; i++)
  {
    temp ^= data[i];
  }
  return temp; 
}

void HandlerRfidCmd(u8* data ,u8 length)
{
  if(length<4) return;
  if( RFID_checkSum(&data[1],data[1])
           ==data[length-1])
  {
    switch(data[2])
    {
    case 0x10://һ������
      {
        RFID_Type=(data[4]<<8)|data[5];
        memcpy(RFID_CARD_ID,&data[6],4);
        
#if (RFID_UPLOAD_MODE == RFID_UPLOAD_ID_AND_INFOR)    
        RFID_COMEIN_Flag|=1;
        RFID_ReadBlockRetryNum=3;//����ظ���3��
        RFID_ReadBlockTimes+=1;
        //RFID_ReadBlockDelay=100;
#else
        RFID_COMEIN_Flag=1;
        //����ȡ������Ϣ,ֻ����ID��
        memset(RFID_BLOCK_Data,0,16);
        memcpy(RFID_BLOCK_Data,RFID_CARD_ID,4);
        RFID_COMEIN_Flag|=2;        
        //RFID_ONLINE_Timeout=MOD_BUS_Reg.RFID_ONLINE_TIME_IN_MS;
        //RFID_ONLINE_Flag=1;        
#endif
      }
      break;
    case 0x14://һ����������
      {
        if(data[1]==0x19)
        {
          memcpy(RFID_BLOCK_Data,&data[10],16);
          RFID_COMEIN_Flag|=2;
          //RFID_ONLINE_Timeout=MOD_BUS_Reg.RFID_ONLINE_TIME_IN_MS;
          //RFID_ONLINE_Flag=1;
        }
        else
        {
          RFID_ReadBlockFailFlag=1; 
        }
      }
      break;
    }
  }
}

void ReadRfidBlock(void)
{
  FillUartTxBufN((u8*)READ_BLOCK_ONE,11,RFID_COMM);
}

void READ_RFID_BLOCK_Task(void)
{
  static u8 pro=0;
  switch(pro)
  {
  case 0:
    {
      if((RFID_ReadBlockRetryNum!=0)&&(RFID_ReadBlockTimeout==0))
      {
        RFID_ReadBlockRetryNum-=1;
        RFID_ReadBlockTimeout=10;
        RFID_COMEIN_Flag&=~(2+4);
        RFID_ReadBlockFailFlag=0;   
        pro+=1;
      }
    }
    break;
  case 1:
    {
      if(RFID_ReadBlockTimeout==0)
      {
        ReadRfidBlock();
        RFID_ReadBlockTimeout=200;//200ms����ʱ
        pro+=1;
      }
    }
    break;
  case 2:
    {
      if(RFID_ReadBlockTimeout!=0)
      {
        if(RFID_ReadBlockFailFlag!=0)
        {
          RFID_ReadBlockFailFlag=0;
          RFID_ReadBlockUnSuccessTimes+=1; 
          RFID_ReadBlockTimeout=50;
          pro=0;
        }
        if(RFID_COMEIN_Flag&2)
        {
          if((RFID_BLOCK_Data[0]==0x55) && (RFID_BLOCK_Data[3]==0xAA))
          {
            PlaceId = ((u16)RFID_BLOCK_Data[1]<<8)|RFID_BLOCK_Data[2];
            RFID_COMEIN_Flag |= 4;

            if(LOG_Level <= LEVEL_INFO) printf("RFID = %04X\n", PlaceId);    
          }
          
          RFID_ReadBlockSuccessTimes+=1;
          RFID_ReadBlockTimeout=2;
          RFID_ReadBlockRetryNum=0;
          pro=0;
        }
      }
      else
      {
        RFID_ReadBlockUnSuccessTimes+=1; 
        pro=0;
      }
    }
    break;
  default:
    {
      pro=0;
    }
  }

  /*
  if((RFID_ONLINE_Timeout==0)&&(RFID_ONLINE_Flag!=0))
  {
    RFID_ONLINE_Flag=0;
  }
  */
}