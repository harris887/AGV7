/*预处理部分*/
#ifndef __init_h_
#define __init_h_
//#include "Protocol.h"





#define FLASH_PAGE_SIZE ((u32)0x800)
#define Id_addr 0x1FFFF7E8
#define ID_SIZE 12
extern u8 stm32ID[ID_SIZE];
extern u8 stm32ID_Convert[ID_SIZE];

void RCC_Configuration(void);
void NVIC_Configuration(void);
void GPIO_Configuration(void);

void AC_ZeroHandler(void);
void AC_DecInit(void);
extern u16 VoltList[1000];
extern void GetVoltList(void);
extern u16 Tim3IntNumBk;
extern u16 time_to_open_heat;
extern u16 JumpTimer;

void GetConfigData(u16* pInitData);
FLASH_Status SaveConfigData(u16* pInitData);
u16 memcompare(u8* a,u8* b,u16 num);
void JumpToAddr(u32 programAddr);
void memcopy(u8* src,u8* dst,u16 num);
u8 Reload_FactoryProgram(u32 src_addr,u32 dst_addr,u32 byte_length);
typedef struct
{
        u16	AdxlZero;
	u16	GyroZero;
	u16	RockerZero;
	u16 Checksum;
}INITIALIZE_DEF;

extern INITIALIZE_DEF InitPara;
extern INITIALIZE_DEF InitData;

extern u8 ID_OK_Flag;
extern void InitializeTask( void );
extern void InitializePara( void );
extern void InitializeInit( void );

void init();
void CheckID(void);

void FeedDog(void);
void SetTimeoutJump(u16 time);
void TimeoutJump(void);
void BackupAccessEnable(void);
u8 EraseAndPro_FLASH_Less1PAGE(u32* src_buf,u32 WORD_LEN,u32 flash_addr);
//void LoadBalancePara(void);
//void SaveBalancePara(void);
u8 RecheckSensorTask(void);
void GET_stm32ID(void);
void GET_stm32ID_Convert(void);
FLASH_Status SavePassWord(void);
void GetPassword(void);
void FLASH_READOUT_Protect(void);
void STORE_CAR_STATUS_Task(void);


#define STAND_BOOT_ALWAYS_FLAG    0x1234
#define TO_BOOT_UPDATE_FLAG       0x4321
#define BOOT_STANDARD_FLAG	  0x0000






#endif

