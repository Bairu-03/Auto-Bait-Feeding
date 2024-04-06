#include "stm32f10x.h" // Device header
#include "Delay.h"

void Key_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource11);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);

    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line5 | EXTI_Line6 | EXTI_Line7 | EXTI_Line10 | EXTI_Line11 | EXTI_Line12;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStructure);
}

uint8_t KeyNum = 0;

/**
 * @brief  读取按键键码
 * @param  无
 * @retval 键码值。
 * KeyNum保存键码值, 无按键动作时为0。
 * PB5 菜单键、确认键, 键码:5 |
 * PB6 左方向键, 键码:4 |
 * PB7 右方向键, 键码:6 |
 * PB10 上方向键, 键码:8 |
 * PB11 下方向键, 键码:2 |
 * PB12 返回键, 键码:1
 */
int8_t GetKeyNum(void)
{
    uint8_t Temp;
    Temp = KeyNum;
    KeyNum = 0;
    return Temp;
}

void EXTI9_5_IRQHandler(void)
{
    // PB5 菜单键、确认键,键码:5
    if (EXTI_GetITStatus(EXTI_Line5) == SET)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0)
        {
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0)
                ;
            KeyNum = 5;
        }
        EXTI_ClearITPendingBit(EXTI_Line5);
    }

    // PB6 左方向键,键码:4
    if (EXTI_GetITStatus(EXTI_Line6) == SET)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0)
        {
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0)
                ;
            KeyNum = 4;
        }
        EXTI_ClearITPendingBit(EXTI_Line6);
    }

    // PB7 右方向键,键码:6
    if (EXTI_GetITStatus(EXTI_Line7) == SET)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7) == 0)
        {
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7) == 0)
                ;
            KeyNum = 6;
        }
        EXTI_ClearITPendingBit(EXTI_Line7);
    }
}

void EXTI15_10_IRQHandler(void)
{
    // PB10 上方向键,键码:8
    if (EXTI_GetITStatus(EXTI_Line10) == SET)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0)
        {
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0)
                ;
            KeyNum = 8;
        }
        EXTI_ClearITPendingBit(EXTI_Line10);
    }

    // PB11 下方向键,键码:2
    if (EXTI_GetITStatus(EXTI_Line11) == SET)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
        {
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
                ;
            KeyNum = 2;
        }
        EXTI_ClearITPendingBit(EXTI_Line11);
    }

    // PB12 返回键,键码:1
    if (EXTI_GetITStatus(EXTI_Line12) == SET)
    {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0)
        {
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0)
                ;
            KeyNum = 1;
        }
        EXTI_ClearITPendingBit(EXTI_Line12);
    }
}
