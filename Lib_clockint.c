/**
 * Copyright (C) JG 2016
 */


#include "lib_clockint.h"


/**
  * @brief  Configure RTC peripheral
  * @param  clock = CLOCK_SOURCE_LSI (internal) or CLOCK_SOURCE_LSE (external)
  * @param  RTC_DateStructure = date structure for clock initilization
  * @param  RTC_TimeStructure = time structure for clock initilization
  * @param  flag = CLOCK_CHECK (check if initialized yet) or CLOCK_FORCE (initialize)
  * @retval None
  */
void LibClock_Init(uint16_t clock, RTC_DateTypeDef * RTC_DateStructure, RTC_TimeTypeDef * RTC_TimeStructure, uint16_t flag)
    {
    RTC_InitTypeDef RTC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // Enable the PWR clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    // Allow access to RTC
    PWR_BackupAccessCmd(ENABLE);

    if (clock == CLOCK_SOURCE_LSE)
        {
        // Enable the LSE OSC
        RCC_LSEConfig(RCC_LSE_ON);

        // Wait till LSE is ready
        while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

        // Select external RTC Clock Source (32768 Hz)
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        }
    else
        {
        // RTC Clock may varie due to LSI frequency dispersion
        // Enable LSI OSC
        RCC_LSICmd(ENABLE);

        // Wait till LSI is ready
        while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

        // Select internal RTC Clock Source
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
        }

    // Enable RTC Clock
    RCC_RTCCLKCmd(ENABLE);

    // Wait for RTC APB registers synchronisation
    RTC_WaitForSynchro();

    // Configure RTC data register and RTC prescaler
    // ck_spre(1Hz) = RTCCLK(LSE) /(AsynchPrediv + 1)*(SynchPrediv + 1)
    RTC_InitStructure.RTC_AsynchPrediv= 0x7F;
    RTC_InitStructure.RTC_SynchPrediv= 0xFF;
    RTC_InitStructure.RTC_HourFormat= RTC_HourFormat_24;

    RTC_Init(&RTC_InitStructure);

    // Set current date and time if need be
    if  (((flag == CLOCK_CHECK) && (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x32F4))
        || (flag == CLOCK_FORCE))
        {
        RTC_SetDate(RTC_Format_BCD, RTC_DateStructure);
        RTC_SetTime(RTC_Format_BCD, RTC_TimeStructure);
        }

    // Enable RTC Alarm A & B Interrupts
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);
    RTC_ITConfig(RTC_IT_ALRB, ENABLE);

    // Clear RTC Alarm Flags
    RTC_ClearFlag(RTC_FLAG_ALRAF);
    RTC_ClearFlag(RTC_FLAG_ALRBF);

    // Clear any EXTI Line 17 Pending interrupt (Connected internally to RTC Alarm)
    EXTI_ClearITPendingBit(EXTI_Line17);

    // EXTI Line 17 configuration
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // Enable RTC Alarm Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);

    // Configuration done
    RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F4);
    }


/**
  * @brief  Get current time.
  * @param  RTC_TimeStructure = time structure filled by function
  * @retval None
  */
void LibClock_GetTime(RTC_TimeTypeDef * RTC_TimeStructure)
    {
    // Get the current Time
    RTC_GetTime(RTC_Format_BCD, RTC_TimeStructure);
    }


/**
  * @brief  Set current time.
  * @param  RTC_TimeStructure = time structure passed to function
  * @retval None
  */
void LibClock_SetTime(RTC_TimeTypeDef * RTC_TimeStructure)
    {
    RTC_SetTime(RTC_Format_BCD, RTC_TimeStructure);
    }


/**
  * @brief  Get current date.
  * @param  RTC_DateStructure = date structure filled by function
  * @retval None
  */
void LibClock_GetDate(RTC_DateTypeDef * RTC_DateStructure)
    {
    // Get the current Time
    RTC_GetDate(RTC_Format_BCD, RTC_DateStructure);
    }


/**
  * @brief  Set current date.
  * @param  RTC_DateStructure = date structure passed to function
  * @retval None
  */
void LibClock_SetDate(RTC_DateTypeDef * RTC_DateStructure)
    {
    RTC_SetDate(RTC_Format_BCD, RTC_DateStructure);
    }


/**
  * @brief  Get current alarm time
  * @param  RTC_Alarm = RTC_Alarm_A or RTC_Alarm_B
  * @param  RTC_AlarmStructure = alarme structure filled by function
  * @retval None
  */
void LibClock_GetAlarm(uint32_t RTC_Alarm, RTC_AlarmTypeDef * RTC_AlarmStructure)
    {
    // Get the current Alarm A or B
    RTC_GetAlarm(RTC_Format_BCD, RTC_Alarm, RTC_AlarmStructure);
    }


/**
  * @brief  Set current alarm time
  * @param  RTC_Alarm = RTC_Alarm_A or RTC_Alarm_B
  * @param  RTC_AlarmStructure = alarme structure passed to function
  * @retval None
  */
void LibClock_SetAlarm(uint32_t RTC_Alarm, RTC_AlarmTypeDef * RTC_AlarmStructure)
    {
    // Pour pouvoir modifier une alarme il faut la desactiver
    RTC_AlarmCmd(RTC_Alarm, DISABLE);

    // Set the alarm in BCD
    RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm, RTC_AlarmStructure);

    // Enable alarm
    RTC_AlarmCmd(RTC_Alarm, ENABLE);
    }


/**
  * @brief  Activate or deactivate alarm
  * @param  RTC_Alarm = RTC_Alarm_A or RTC_Alarm_B
  * @param  state= ENABLE or DISABLE
  * @retval None
  */
void LibClock_CmdAlarm(uint32_t RTC_Alarm, FunctionalState state)
    {
    RTC_AlarmCmd(RTC_Alarm, state);
    }


/**
 * @brief  Convert values from decimal to BCD format
 * @param  c = decimal value to convert
 * @retval Converted BCD value
 */
uint8_t DecToBcd(uint8_t c)
	{
	uint8_t bcd;

	bcd= ((c/10) << 4) + (c%10);
	return bcd;
	}

/**
 * @brief  Convert values from BCD to decimal format
 * @param  c = BCD value to convert
 * @retval Converted decimal value
 */
uint8_t BcdToDec(uint8_t c)
    {
	uint8_t dec;

	dec= 10*(c >> 4) + (c&0x0F);
	return dec;
    }

