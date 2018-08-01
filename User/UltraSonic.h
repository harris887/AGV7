#ifndef _UltraSonic_h
#define _UltraSonic_h

/*超声波模组只能使用MODBUS地址-0x01*/
#define DEFAULT_MODE_BUS_ULTRA_SONIC_ADDR       0x01
#define ULTRA_SONIC_REG_ADDR_OFFSET             0x60

#define ULTRA_SONIC_SENSOR_NUM                  4
#define DEFAULT_ULTRA_SONIC_CHECK_TIME_IN_MS    100
#define DEFAULT_ULTRA_SONIC_DISTANCE            0 //100
#define DEFAULT_ULTRA_SONIC_NO_DATA_TIME_OUT    500
#define DEFAULT_ULTRA_SONIC_CLOSE_DISTANCE      70 //50

extern u8 UltraSonicMachineState;
extern u16 US_Timeout;
extern u16 UltraSonicCheckTimer;

extern u16 UltraSonicDistance[ULTRA_SONIC_SENSOR_NUM];
extern u32 us_time;
extern u32 us_suc_time;

//---------------------------------------------------//
extern void Analysis_Receive_From_UltraSonic(u8 data);
extern void Check_UltraSonic_TASK(void);
//u8 Transit_UltarSonic(u8* pInfor,u8 length);
//u8 AckWriteCmdToMaster(u8 CMD_ModBus,u8 r_code);
//u8 AckReadCmdToMaster(u8* pDATA,u8 byte_length);












#endif