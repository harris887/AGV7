#include "user_inc.h"

#define DEFAULT_CHARGE_MODULE_ADDR  0x01
#define CHARGE_COMM                 1

const u8 CMD_CHARGE_START[8] = {0x01, 0x05 ,0x00 ,0x00 ,0x00 ,0x01 ,0x0C ,0x0A};  // ����
const u8 CMD_CHARGE_STOP[8] = {0x01, 0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,0xCD ,0xCA};   // ֹͣ


MODBUS_SAMPLE MODBUS_Charge = {
  .MachineState = 0,
  .read_success_num = 0,
  .write_success_num = 0,
};

void Analysis_Receive_From_Charge(u8 data,MODBUS_SAMPLE* pMODBUS)
{
    switch(pMODBUS->MachineState)//��ʼ�� Ĭ�� Ϊ 00;
    {
    case 0:
      {
        if(data == DEFAULT_CHARGE_MODULE_ADDR)//�ӻ���ַ
        {
          pMODBUS->MachineState = 0x01;
          pMODBUS->BufIndex = 0;
          pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        }
        else
        {
          pMODBUS->MachineState = 0x0B;//����������������Ҫ�������м�����Ϊ01������Ϊ��Ҫ�ӻ���ַ��
          pMODBUS->BufIndex = 0;
        }  
      }
      break;
      case 0x01:
      {	 
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        if(data == CMD_ModBus_ReadEx) //ִ�ж�ȡ���������Ĵ�������  0x01 
        {
          pMODBUS->MachineState = 0x02; 
          pMODBUS->ModBus_CMD = data;
          pMODBUS->read_receive_timer = 0;
        }
        else if(data == CMD_ModBus_Wire_Write)
        {
          pMODBUS->MachineState = 0x04; 
          pMODBUS->ModBus_CMD = data;
          pMODBUS->read_receive_timer = 0;
        }
        else
        { 
          pMODBUS->MachineState = 0x0B;
          pMODBUS->BufIndex = 0;
        }
      }
      break;
      case 0x02: //read part 00
      {    
        //���յ������ܵ��ֽ���
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        pMODBUS->read_receive_timer++;
        if(pMODBUS->read_receive_timer == 1 )
        {
          pMODBUS->Read_Register_Num = pMODBUS->DataBuf[pMODBUS->BufIndex-1];
          if(pMODBUS->Read_Register_Num <= 16)//�����޶�
          {
            pMODBUS->MachineState = 0x03;
          }
          else
          {
            pMODBUS->MachineState = 0x00;
          }
          pMODBUS->read_receive_timer = 0;
        } 
      }
      break;
      case 0x03: //read part 01
      {   
        pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
        pMODBUS->read_receive_timer++;
        if(pMODBUS->read_receive_timer >= (pMODBUS->Read_Register_Num+2))
        {
          u16 cal_crc;
          cal_crc=ModBus_CRC16_Calculate(pMODBUS->DataBuf,pMODBUS->Read_Register_Num+3);
              
          pMODBUS->receive_CRC_L = pMODBUS->DataBuf[pMODBUS->BufIndex-2];
          pMODBUS->receive_CRC_H = pMODBUS->DataBuf[pMODBUS->BufIndex-1];
          if(((cal_crc>>8) == pMODBUS->receive_CRC_H) && ((cal_crc&0xFF) == pMODBUS->receive_CRC_L))
          {
            pMODBUS->err_state = 0x00;//CRCУ����ȷ 
            pMODBUS->read_success_num += 1;
            ///////
          }    
          else	  
          {
             pMODBUS->err_state = 0x04;
          }   
           pMODBUS->BufIndex = 0;  
           pMODBUS->read_receive_timer = 0;  
           pMODBUS->MachineState = 0x00;                
          }
        }
        break;
        case 0x04: //write
          {
            pMODBUS->DataBuf[pMODBUS->BufIndex++] = data;
            pMODBUS->read_receive_timer++;
            if( pMODBUS->read_receive_timer == 6 )
            {
              u16 cal_crc;
              cal_crc=ModBus_CRC16_Calculate(pMODBUS->DataBuf,6);
              
              pMODBUS->receive_CRC_L = pMODBUS->DataBuf[pMODBUS->BufIndex-2];
              pMODBUS->receive_CRC_H = pMODBUS->DataBuf[pMODBUS->BufIndex-1];
              if(((cal_crc>>8) == pMODBUS->receive_CRC_H) && ((cal_crc&0xFF) == pMODBUS->receive_CRC_L))
              {
                pMODBUS->err_state = 0x00;//CRCУ����ȷ 
                pMODBUS->write_success_num += 1;
              }    
              else	  
              {
                pMODBUS->err_state = 0x04;
              }   
              pMODBUS->BufIndex = 0;  
              pMODBUS->read_receive_timer = 0;  
              pMODBUS->MachineState = 0;   
            }
          }
        break;
      case 0xb:
        {
        
        }
        break;  
      default:
        {
          pMODBUS->MachineState=0;
        }
    }
}

void SET_Charge(u8 on_off)
{
  if(on_off)
    FillUartTxBufN((u8*)CMD_CHARGE_START, sizeof(CMD_CHARGE_START), CHARGE_COMM);
  else
    FillUartTxBufN((u8*)CMD_CHARGE_STOP, sizeof(CMD_CHARGE_STOP), CHARGE_COMM);
}