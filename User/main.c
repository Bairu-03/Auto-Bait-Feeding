#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "Key.h"
#include "OLED.h"
#include "DS18B20.h"
#include "Serial.h"
#include "string.h"
#include "Timer.h"
#include "servo.h"
#include "MyRTC.h"

uint8_t UIpage = 0;
uint8_t FeedInterval=10;
uint8_t BaitWarning=0;
uint8_t WiFiState=0;
uint8_t TempEnable=0;

/**
 * @brief  主界面
 * @param  SysTime RTC时间数组地址
 * @param  FeedInterval 投饵间隔时间
 *     @arg 单位:分钟
 * @param  BaitWarning 饵料不足警报标志
 *     @arg 1:饵料余量不足 | 0:饵料余量充足
 * @param WiFiState WiFi连接状态标志
 *     @arg 1:WiFi已连接 | 0:WiFi未连接
 * @param TempEnable 温度检测使能
 *     @arg 1:禁用温度检测 | 0:启用温度检测
 * @retval 无
 */
void MainMenu(uint16_t *SysTime, uint8_t FeedInterval, uint8_t BaitWarning, uint8_t WiFiState, uint8_t TempEnable)
{
    float Temp;
    uint8_t TimeLine_Main = 1;
    uint8_t IntervalLine_Main = 3;
    uint8_t TmpLine = 5;
    uint8_t BaitLine = 7;

    // 显示RTC时间
    MyRTC_ReadTime();
    OLED_ShowCN(TimeLine_Main, 1, 8);
    OLED_ShowCN(TimeLine_Main, 17, 11);
    OLED_ShowChar(TimeLine_Main, 33, ':', 8);
    OLED_ShowNum(TimeLine_Main, 41, SysTime[3], 2, 8);
    OLED_ShowChar(TimeLine_Main, 57, ':', 8);
    OLED_ShowNum(TimeLine_Main, 65, SysTime[4], 2, 8);
    OLED_ShowChar(TimeLine_Main, 81, ':', 8);
    OLED_ShowNum(TimeLine_Main, 89, SysTime[5], 2, 8);

    // 显示投饵间隔时间
    OLED_ShowCN(IntervalLine_Main, 1, 11);
    OLED_ShowCN(IntervalLine_Main, 17, 12);
    OLED_ShowString(IntervalLine_Main + 1, 33, "(min)", 6);
    OLED_ShowChar(IntervalLine_Main, 63, ':', 8);
    OLED_ShowNum(IntervalLine_Main, 71, FeedInterval, 4, 8);

    // 饵料不足提醒
    if (BaitWarning)
    {
        // "饵料不足!"
        OLED_ShowCN(BaitLine, 1, 10);
        OLED_ShowCN(BaitLine, 17, 15);
        OLED_ShowCN(BaitLine, 33, 16);
        OLED_ShowCN(BaitLine, 49, 17);
        OLED_ShowChar(BaitLine, 65, '!', 8);
    }

    // WiFi连接状态图标
    if (WiFiState)
        // WiFi已连接
        OLED_ShowCN(7, 112, 13);
    else
        // WiFi未连接
        OLED_ShowCN(7, 112, 14);

    // 温度检测
    if (!TempEnable)
    {
        if (!DS18B20_Reset())
        {
            Temp = DS18B20_ReadTemp();
            OLED_ShowCN(TmpLine, 1, 0);
            OLED_ShowCN(TmpLine, 17, 1);
            OLED_ShowCN(TmpLine, 33, 2);
            OLED_ShowChar(TmpLine, 49, ':', 8);
            OLED_ShowString(TmpLine, 97, "  ", 8);
            if (Temp >= 0)
            {
                OLED_ShowFloat(TmpLine, 57, Temp, 3, 1, 8);
            }
            else
            {
                OLED_ShowFloat(TmpLine, 57, Temp, 2, 1, 8);
            }
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
 * @brief  设置界面
 * @param 无
 * @retval 无
 */
void SetMenu(void)
{
    uint8_t TimeLine_Set = 1;
    uint8_t IntervalLine_Set = 5;

    // 系统时间
    OLED_ShowCN(TimeLine_Set, 1, 8);
    OLED_ShowCN(TimeLine_Set, 17, 11);
    OLED_ShowChar(TimeLine_Set, 33, ':', 8);
    OLED_ShowNum(TimeLine_Set, 41, MyRTC_Time[3], 2, 8);
    OLED_ShowChar(TimeLine_Set, 57, ':', 8);
    OLED_ShowNum(TimeLine_Set, 65, MyRTC_Time[4], 2, 8);
    OLED_ShowChar(TimeLine_Set, 81, ':', 8);
    OLED_ShowNum(TimeLine_Set, 89, MyRTC_Time[5], 2, 8);

    // 投饵间隔时间
    OLED_ShowCN(IntervalLine_Set, 1, 11);
    OLED_ShowCN(IntervalLine_Set, 17, 12);
    OLED_ShowString(IntervalLine_Set + 1, 33, "(min)", 6);
    OLED_ShowChar(IntervalLine_Set, 63, ':', 8);
    OLED_ShowNum(IntervalLine_Set, 71, FeedInterval, 4, 8);
}

void KeySwitch(void)
{
    uint8_t TempT[3];    // 时间, 0:时 | 1:分 | 2:秒
    uint8_t TempFI;     // 投饵间隔
    switch (KeyNum)
    {
    case 5:  // 菜单、确定按键
        if (!UIpage)    // 主界面
        {
            OLED_Clear();
            SetMenu();
            UIpage = 1;
            KeyNum = 0;
        }
        else    // 设置界面
        {
            OLED_Clear();
            MyRTC_Time[3] = TempT[0];
            MyRTC_Time[4] = TempT[1];
            MyRTC_Time[5] = TempT[2];
            MyRTC_SetTime();
            FeedInterval = TempFI;
            MainMenu(MyRTC_Time, FeedInterval, BaitWarning, WiFiState, TempEnable);
            UIpage = 0;
            KeyNum = 0;
        }
        break;
    case 4:

        break;
    case 6:

        break;
    case 8:

        break;
    case 2:

        break;
    case 1:

        break;
    default:
        if(!UIpage)
            MainMenu(MyRTC_Time, FeedInterval, BaitWarning, WiFiState, TempEnable);
        else
            SetMenu();
        break;
    }
}

uint32_t a = 0;
int main(void)
{
    // Serial_Init();
    OLED_Init();
    Key_Init();
    DS18B20_Init();
    Timer_Init();
    MyRTC_Init();
    // Servo_Init();

    // Servo_SetAngle(0);

    while (1)
    {
        KeySwitch();
        // 首次上电设置系统时间和投饵间隔时间
        // if (BKP_ReadBackupRegister(BKP_DR2) == 0x00)
        // {
        // }
        // else
        // {
        // }
    }
}

// 定时器中断，每秒一次
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        // Serial_SendString("OK\r\n");
        a++;
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
