#include "stm32f10x.h" // Device header
#include <time.h>

void MyRTC_SetTime(uint16_t *MyRTC_Time);

/**
 * @brief  RTC初始化, 默认时间:2024.1.1 00:00:00
 * @param  无
 * @retval 无
 */
void MyRTC_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);

    PWR_BackupAccessCmd(ENABLE);

    if (BKP_ReadBackupRegister(BKP_DR1) != 0xFEFE) // BKP_DR1寄存器内无标记值(VBAT断电), 执行RTC初始化
    {
        uint16_t Default_Time[] = {2024, 1, 1, 0, 0, 0};

        RCC_LSEConfig(RCC_LSE_ON);
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET)
            ;
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        RCC_RTCCLKCmd(ENABLE);

        RTC_WaitForSynchro();
        RTC_WaitForLastTask();

        RTC_SetPrescaler(32768 - 1);
        RTC_WaitForLastTask();

        MyRTC_SetTime(Default_Time);

        BKP_WriteBackupRegister(BKP_DR1, 0xFEFE);
    }
    else
    {
        RTC_WaitForSynchro();
        RTC_WaitForLastTask();
    }

    RTC_ITConfig(RTC_IT_SEC,ENABLE); // 使能RTC秒中断

    // 配置RTC中断
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief  设置RTC时间
 * @param  MyRTC_Time 时间值数组首地址, 长度:6, 0:年 | 1:月 | 2:日 | 3:时 | 4:分 | 5:秒
 * @retval 无
 */
void MyRTC_SetTime(uint16_t *MyRTC_Time)
{
    time_t time_cnt;
    struct tm time_date;

    time_date.tm_year = MyRTC_Time[0] - 1900;
    time_date.tm_mon = MyRTC_Time[1] - 1;
    time_date.tm_mday = MyRTC_Time[2];
    time_date.tm_hour = MyRTC_Time[3];
    time_date.tm_min = MyRTC_Time[4];
    time_date.tm_sec = MyRTC_Time[5];

    time_cnt = mktime(&time_date) - 8 * 60 * 60;

    RTC_SetCounter(time_cnt);
    RTC_WaitForLastTask();
}

/**
 * @brief  读取RTC时间
 * @param  无
 * @retval 时间值数组首地址, 长度:6, 0:年 | 1:月 | 2:日 | 3:时 | 4:分 | 5:秒
 */
uint16_t *MyRTC_ReadTime(void)
{
    time_t time_cnt;
    struct tm time_date;
    static uint16_t Read_Time[6];

    time_cnt = RTC_GetCounter() + 8 * 60 * 60;

    time_date = *localtime(&time_cnt);

    Read_Time[0] = time_date.tm_year + 1900;
    Read_Time[1] = time_date.tm_mon + 1;
    Read_Time[2] = time_date.tm_mday;
    Read_Time[3] = time_date.tm_hour;
    Read_Time[4] = time_date.tm_min;
    Read_Time[5] = time_date.tm_sec;

    return Read_Time;
}
