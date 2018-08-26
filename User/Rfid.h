#ifndef _RFID_H_
#define _RFID_H_

extern u8 RfidMachineState;
extern u8 RFID_COMEIN_Flag;
extern u8 RFID_DATA_Buf[256];
extern u8 RFID_BLOCK_Data[16];
extern u16 RFID_ReadBlockTimeout;
extern u32 RFID_ReadBlockTimes;
extern u32 RFID_ReadBlockSuccessTimes;
extern u16 RFID_ONLINE_Timeout;
extern u16 RFID_ONLINE_Flag;
extern u16 PlaceId;

u8 RFID_checkSum(u8* data, u8 length);
void Analysis_Receive_From_RFID(u8 data);
void HandlerRfidCmd(u8* data ,u8 length);
void ReadRfidBlock(void);
void READ_RFID_BLOCK_Task(void);


#define RFID_UPLOAD_ONLY_ID       0
#define RFID_UPLOAD_ID_AND_INFOR  1

#define RFID_UPLOAD_MODE  RFID_UPLOAD_ID_AND_INFOR //RFID_UPLOAD_ID_AND_INFOR



#endif