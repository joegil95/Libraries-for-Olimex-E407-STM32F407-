/**
 * RTC Clock Library for STM32F4xx devices
 */

#include "stm32f4xx.h"

#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_exti.h"

#include "misc.h"


// RTC Clock sources
#define CLOCK_SOURCE_LSI    0       // Internal clock source
#define CLOCK_SOURCE_LSE    1       // External clock source

// RTC Init modes
#define CLOCK_CHECK   0       // check if init needed
#define CLOCK_FORCE   1       // init clock anyway

// function prototypes
void LibClock_Init(uint16_t clock, RTC_DateTypeDef * RTC_DateStructure, RTC_TimeTypeDef * RTC_TimeStructure, uint16_t flag);
void LibClock_GetTime(RTC_TimeTypeDef * RTC_TimeStructure);
void LibClock_SetTime(RTC_TimeTypeDef * RTC_TimeStructure);
void LibClock_GetDate(RTC_DateTypeDef * RTC_DateStructure);
void LibClock_SetDate(RTC_DateTypeDef * RTC_DateStructure);
void LibClock_GetAlarm(uint32_t RTC_Alarm, RTC_AlarmTypeDef * RTC_AlarmStructure);
void LibClock_SetAlarm(uint32_t RTC_Alarm, RTC_AlarmTypeDef * RTC_AlarmStructure);
void LibClock_CmdAlarm(uint32_t RTC_Alarm, FunctionalState state);
uint8_t DecToBcd(uint8_t c);
uint8_t BcdToDec(uint8_t c);
