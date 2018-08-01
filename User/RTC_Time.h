#ifndef _RTC_TIME_H_
#define _RTC_TIME_H_
#include <time.h>

//#define RTCClockOutput_Enable /*外部输出引脚*/

extern u8 TimeDisplay;

struct tm Time_ConvUnixToCalendar(time_t t);
time_t Time_ConvCalendarToUnix(struct tm t);
time_t Time_GetUnixTime(void);
struct tm Time_GetCalendarTime(void);
struct tm Time_GetCalendarTime_WithDayOffset(u16 day_offset);
void Time_SetUnixTime(time_t);
void Time_SetCalendarTime(struct tm t);

void RTC_Configuration(void);
void RTC_Config(void);
void RTC_IntHandler(void);


void RTC_Test(void);
#endif
