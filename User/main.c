#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "Key.h"
#include "OLED.h"
#include "DS18B20.h"
#include "Serial.h"
#include "string.h"
#include "servo.h"
#include "MyRTC.h"

uint8_t UIpage = 0;      // 主界面
uint8_t BaitWarning = 0; // 饵料余量标志. 0:充足 | 1:不足
uint8_t WiFiState = 0;   // 网络连接状态标志. 0:未连接 | 1:已连接
uint8_t TempEnable = 0;  // 温度传感器使能标志. 0:启用 | 1:禁用

uint16_t *TempT; // 系统时间临时变量. 0:年 | 1:月 | 2:日 | 3:时 | 4:分 | 5:秒

uint8_t FeedInterval[3]; // 投饵间隔
uint8_t *TempFI;         // 投饵间隔临时变量
uint8_t Servoflag = 0;   // 投饵舵机启停标志. 0:停止 | 1:启动
uint8_t FeedCount = 0;   // 投饵计次

// "设置"界面的光标位置
uint8_t SetMenu_CurL, SetMenu_CurC;

/**
 * @brief  读取投饵间隔时间并设置RTC闹钟.
 *         从BKP寄存器2、3、4读取投饵间隔时间并转换成秒, 设定RTC闹钟
 * @param  无
 * @retval 无
 */
void MyRTC_SetAlarm(void)
{
    uint32_t FIsec;
    FeedInterval[0] = BKP_ReadBackupRegister(BKP_DR2);
    FeedInterval[1] = BKP_ReadBackupRegister(BKP_DR3);
    FeedInterval[2] = BKP_ReadBackupRegister(BKP_DR4);

    FIsec = FeedInterval[0] * 60 * 60 + FeedInterval[1] * 60 + FeedInterval[2] - 1;

    if (FIsec)
    {
        RTC_ITConfig(RTC_IT_ALR, ENABLE);
        RTC_EnterConfigMode();
        RTC_SetAlarm(RTC_GetCounter() + FIsec);
        RTC_WaitForLastTask();
        RTC_ExitConfigMode();
    }
    else
    {
        RTC_ITConfig(RTC_IT_ALR, DISABLE);
    }
}

/**
 * @brief  显示主界面
 * @param  SA_ST_M 投饵舵机状态/系统时间显示, 1:"正在投饵..." | 0:"时间:xx:xx:xx"
 *     @arg 1:饵料余量不足 | 0:饵料余量充足
 * @param  FI_M 投饵间隔时间, 数组, 时 分 秒
 * @param  BW_M 饵料不足警报标志
 *     @arg 1:饵料余量不足 | 0:饵料余量充足
 * @param WS_M WiFi连接状态标志
 *     @arg 1:WiFi已连接 | 0:WiFi未连接
 * @param TE_M 温度检测使能
 *     @arg 1:禁用温度检测 | 0:启用温度检测
 * @retval 无
 */
void MainMenu(uint8_t SA_ST_M, uint8_t *FI_M, uint8_t BW_M, uint8_t WS_M, uint8_t TE_M)
{
    uint8_t TimeLine_Main = 1,
            IntervalLine_Main = 3,
            TmpLine = 5,
            BaitLine = 7;

    uint16_t *SysTime;
    float Temperature;

    if (!SA_ST_M)
    {
        // 显示RTC时间
        SysTime = MyRTC_ReadTime();
        // 时间:xx:xx:xx
        OLED_ShowCN(TimeLine_Main, 1, 8);
        OLED_ShowCN(TimeLine_Main, 17, 11);
        OLED_ShowChar(TimeLine_Main, 33, ':', 8);
        OLED_ShowNum(TimeLine_Main, 41, SysTime[3], 2, 8);
        OLED_ShowChar(TimeLine_Main, 57, ':', 8);
        OLED_ShowNum(TimeLine_Main, 65, SysTime[4], 2, 8);
        OLED_ShowChar(TimeLine_Main, 81, ':', 8);
        OLED_ShowNum(TimeLine_Main, 89, SysTime[5], 2, 8);
    }
    else
    {
        // "正在投饵..."
        OLED_ShowCN(1, 1, 20);
        OLED_ShowCN(1, 17, 21);
        OLED_ShowCN(1, 33, 9);
        OLED_ShowCN(1, 49, 10);
        OLED_ShowString(1, 65, "...   ", 8);
    }

    // 间隔:xx:xx:xx
    OLED_ShowCN(IntervalLine_Main, 1, 11);
    OLED_ShowCN(IntervalLine_Main, 17, 12);
    OLED_ShowChar(IntervalLine_Main, 33, ':', 8);
    OLED_ShowNum(IntervalLine_Main, 41, FI_M[0], 2, 8);
    OLED_ShowChar(IntervalLine_Main, 57, ':', 8);
    OLED_ShowNum(IntervalLine_Main, 65, FI_M[1], 2, 8);
    OLED_ShowChar(IntervalLine_Main, 81, ':', 8);
    OLED_ShowNum(IntervalLine_Main, 89, FI_M[2], 2, 8);

    // 饵料不足提醒
    if (BW_M)
    {
        // "饵料不足(xxx)". xxx为累计投饵次数
        OLED_ShowCN(BaitLine, 1, 10);
        OLED_ShowCN(BaitLine, 17, 15);
        OLED_ShowCN(BaitLine, 33, 16);
        OLED_ShowCN(BaitLine, 49, 17);
        OLED_ShowChar(BaitLine, 65, '(', 8);
        OLED_ShowNum(BaitLine, 73, FeedCount, 3, 8);
        OLED_ShowChar(BaitLine, 97, ')', 8);
    }
    else
    {
        // "计次:xxx"
        OLED_ShowCN(BaitLine, 1, 22);
        OLED_ShowCN(BaitLine, 17, 23);
        OLED_ShowChar(BaitLine, 33, ':', 8);
        OLED_ShowNum(BaitLine, 41, FeedCount, 3, 8);
        OLED_ShowString(BaitLine, 65, "     ", 8);
    }

    // WiFi连接状态图标
    if (WS_M)
        // WiFi已连接
        OLED_ShowCN(7, 112, 13);
    else
        // WiFi未连接
        OLED_ShowCN(7, 112, 14);

    // 温度检测
    if (!TE_M)
    {
        if (!DS18B20_Reset())
        {
            Temperature = DS18B20_ReadTemp();
            // 温度℃:
            OLED_ShowCN(TmpLine, 1, 0);
            OLED_ShowCN(TmpLine, 17, 1);
            OLED_ShowCN(TmpLine, 33, 2);
            OLED_ShowChar(TmpLine, 49, ':', 8);
            OLED_ShowString(TmpLine, 97, "  ", 8);
            if (Temperature >= 0)
                OLED_ShowFloat(TmpLine, 57, Temperature, 3, 1, 8);
            else
                OLED_ShowFloat(TmpLine, 57, Temperature, 2, 1, 8);
            Delay_ms(5);
        }
        else
        {
            // "温度传感器异常"
            OLED_ShowCN(TmpLine, 1, 0);
            OLED_ShowCN(TmpLine, 17, 1);
            OLED_ShowCN(TmpLine, 33, 3);
            OLED_ShowCN(TmpLine, 49, 4);
            OLED_ShowCN(TmpLine, 65, 5);
            OLED_ShowCN(TmpLine, 81, 6);
            OLED_ShowCN(TmpLine, 97, 7);
        }
    }
}

/**
 * @brief  显示设置界面
 * @param SysTime 显示的待修改时间值, 数组, 时 分 秒
 * @param FI_S 投饵间隔时间, 数组, 时 分 秒
 * @retval 无
 */
void SetMenu(uint16_t *SysTime, uint8_t *FI_S)
{
    uint8_t TimeLine_Set = 1;
    uint8_t IntervalLine_Set = 5;

    if ((SetMenu_CurL == 1) || (SetMenu_CurL == 5))
        OLED_ShowCN(SetMenu_CurL, SetMenu_CurC, 19);
    else
        OLED_ShowCN(SetMenu_CurL, SetMenu_CurC, 18);

    // 时间:xx:xx:xx
    OLED_ShowCN(TimeLine_Set, 1, 8);
    OLED_ShowCN(TimeLine_Set, 17, 11);
    OLED_ShowChar(TimeLine_Set, 33, ':', 8);
    OLED_ShowNum(TimeLine_Set, 41, SysTime[3], 2, 8);
    OLED_ShowChar(TimeLine_Set, 57, ':', 8);
    OLED_ShowNum(TimeLine_Set, 65, SysTime[4], 2, 8);
    OLED_ShowChar(TimeLine_Set, 81, ':', 8);
    OLED_ShowNum(TimeLine_Set, 89, SysTime[5], 2, 8);

    // 间隔:xx:xx:xx
    OLED_ShowCN(IntervalLine_Set, 1, 11);
    OLED_ShowCN(IntervalLine_Set, 17, 12);
    OLED_ShowChar(IntervalLine_Set, 33, ':', 8);
    OLED_ShowNum(IntervalLine_Set, 41, FI_S[0], 2, 8);
    OLED_ShowChar(IntervalLine_Set, 57, ':', 8);
    OLED_ShowNum(IntervalLine_Set, 65, FI_S[1], 2, 8);
    OLED_ShowChar(IntervalLine_Set, 81, ':', 8);
    OLED_ShowNum(IntervalLine_Set, 89, FI_S[2], 2, 8);
}

int main(void)
{
    Serial_Init();
    OLED_Init();
    Key_Init();
    DS18B20_Init();
    MyRTC_Init();

    Servo_Init();
    Servo_SetAngle(0); // 舵机复位(接料位置)

    MyRTC_SetAlarm();

    while (1)
    {
        // 执行投饵
        if (Servoflag)
        {
            RTC_ITConfig(RTC_IT_ALR, DISABLE);
            MainMenu(Servoflag, FeedInterval, BaitWarning, WiFiState, TempEnable);
            Servo_SetAngle(180);
            Delay_s(2);
            Servo_SetAngle(0);
            Delay_s(1);

            Servoflag = 0;
            MyRTC_SetAlarm();
        }

        // 饵料不足
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
        {
            BaitWarning = 1;
            RTC_ITConfig(RTC_IT_ALR, DISABLE);
        }
        else
        {
            // 饵料由不足转充足,重置投饵计次,启用定时投饵
            if (BaitWarning)
            {
                MyRTC_SetAlarm();
                FeedCount = 0;
            }
            BaitWarning = 0;
        }

        switch (KeyNum)
        {
        case 5: // 菜单、确定键
            OLED_Clear();
            if (!UIpage) // 主界面 -> 设置界面
            {
                RTC_ITConfig(RTC_IT_ALR, DISABLE);
                TempT = MyRTC_ReadTime();
                TempFI = FeedInterval;
                SetMenu_CurL = 1;
                SetMenu_CurC = 112;
                SetMenu(TempT, TempFI);
                UIpage = 1;
            }
            else // 保存改动, 设置界面 -> 主界面
            {
                MyRTC_SetTime(TempT);
                for (uint8_t j = 0x00, i = 0; i <= 2; i++, j += 0x04)
                {
                    FeedInterval[i] = TempFI[i];
                    BKP_WriteBackupRegister(BKP_DR2 + j, FeedInterval[i]);
                }
                MyRTC_SetAlarm();
                MainMenu(Servoflag, FeedInterval, BaitWarning, WiFiState, TempEnable);
                UIpage = 0;
            }
            KeyNum = 0;
            break;
        case 4: // Left键
            if (UIpage)
            {
                OLED_Clear();
                if (SetMenu_CurL == 1)
                {
                    SetMenu_CurL = 3;
                    SetMenu_CurC = 89;
                }
                else if ((SetMenu_CurL == 3) || (SetMenu_CurL == 7))
                {
                    SetMenu_CurC -= 24;
                    if (SetMenu_CurC <= 41)
                        SetMenu_CurC = 41;
                }
                else if (SetMenu_CurL == 5)
                {
                    SetMenu_CurL = 7;
                    SetMenu_CurC = 89;
                }
            }
            KeyNum = 0;
            break;
        case 6: // Right键
            if (UIpage)
            {
                OLED_Clear();
                if (SetMenu_CurL == 3)
                {
                    SetMenu_CurC += 24;
                    if (SetMenu_CurC > 97)
                    {
                        SetMenu_CurL = 1;
                        SetMenu_CurC = 112;
                    }
                }
                else if (SetMenu_CurL == 7)
                {
                    SetMenu_CurC += 24;
                    if (SetMenu_CurC > 97)
                    {
                        SetMenu_CurL = 5;
                        SetMenu_CurC = 112;
                    }
                }
            }
            KeyNum = 0;
            break;
        case 8: // Up键
            if (UIpage)
            {
                OLED_Clear();
                if (SetMenu_CurC == 112)
                {
                    SetMenu_CurL = 1;
                }
                else if (SetMenu_CurL == 3)
                {
                    if (SetMenu_CurC == 89)
                        if (TempT[5] < 59)
                            TempT[5] += 1;
                        else
                            TempT[5] = 0;
                    if (SetMenu_CurC == 65)
                        if (TempT[4] < 59)
                            TempT[4] += 1;
                        else
                            TempT[4] = 0;
                    if (SetMenu_CurC == 41)
                        if (TempT[3] < 23)
                            TempT[3] += 1;
                        else
                            TempT[3] = 0;
                }
                else if (SetMenu_CurL == 7)
                {
                    if (SetMenu_CurC == 89)
                        if (TempFI[2] < 59)
                            TempFI[2] += 1;
                        else
                            TempFI[2] = 0;
                    if (SetMenu_CurC == 65)
                        if (TempFI[1] < 59)
                            TempFI[1] += 1;
                        else
                            TempFI[1] = 0;
                    if (SetMenu_CurC == 41)
                        if (TempFI[0] < 23)
                            TempFI[0] += 1;
                        else
                            TempFI[0] = 0;
                }
            }
            KeyNum = 0;
            break;
        case 2: // Down键
            if (UIpage)
            {
                OLED_Clear();
                if (SetMenu_CurC == 112)
                {
                    SetMenu_CurL = 5;
                }
                else if (SetMenu_CurL == 3)
                {
                    if (SetMenu_CurC == 89)
                        if (TempT[5] > 0)
                            TempT[5] -= 1;
                        else
                            TempT[5] = 59;
                    if (SetMenu_CurC == 65)
                        if (TempT[4] > 0)
                            TempT[4] -= 1;
                        else
                            TempT[4] = 59;
                    if (SetMenu_CurC == 41)
                        if (TempT[3] > 0)
                            TempT[3] -= 1;
                        else
                            TempT[3] = 23;
                }
                else if (SetMenu_CurL == 7)
                {
                    if (SetMenu_CurC == 89)
                        if (TempFI[2] > 0)
                            TempFI[2] -= 1;
                        else
                            TempFI[2] = 59;
                    if (SetMenu_CurC == 65)
                        if (TempFI[1] > 0)
                            TempFI[1] -= 1;
                        else
                            TempFI[1] = 59;
                    if (SetMenu_CurC == 41)
                        if (TempFI[0] > 0)
                            TempFI[0] -= 1;
                        else
                            TempFI[0] = 23;
                }
            }
            KeyNum = 0;
            break;
        case 1: // 返回键
            OLED_Clear();
            MyRTC_SetAlarm();
            MainMenu(Servoflag, FeedInterval, BaitWarning, WiFiState, TempEnable);
            UIpage = 0;
            KeyNum = 0;
            break;
        default: // 保持当前界面
            if (!UIpage)
                MainMenu(Servoflag, FeedInterval, BaitWarning, WiFiState, TempEnable);
            else
                SetMenu(TempT, TempFI);
            break;
        }
    }
}

// RTC闹钟中断, 投饵间隔时间触发
void RTCAlarm_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_ALR) != RESET)
    {
        FeedCount++;
        Servoflag = 1;
        MyRTC_SetAlarm();
    }
    RTC_ClearITPendingBit(RTC_IT_ALR);
    RTC_WaitForLastTask();
    EXTI_ClearITPendingBit(EXTI_Line17);
    RTC_WaitForLastTask();
}
