#include "stm32f10x.h"
#include "Delay.h"

#define DQ_H GPIO_SetBits(GPIOB, GPIO_Pin_0)
#define DQ_L GPIO_ResetBits(GPIOB, GPIO_Pin_0)
#define DQ_Get GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0)

/**
 * @brief  初始化DS18B20通信IO口。PB0
 * @param  无
 * @retval 无
 */
void DS18B20_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief  初始化通信口为输出模式。PB0
 * @param  无
 * @retval 无
 */
void DS18B20_Output(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief  初始化通信口为输入模式。PB0
 * @param  无
 * @retval 无
 */
void DS18B20_Input(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief  复位DS18B20。
 * @param  无
 * @retval Resetflag 复位标志，当DS18B20出现故障或未连接时返回值为1，反之为0
 */
uint8_t DS18B20_Reset(void)
{
	uint8_t Resetflag = 1;
	DS18B20_Output();
	DQ_H;
	Delay_us(5);

	DQ_L;
	Delay_us(480);
	DQ_H;
	Delay_us(60);
	DS18B20_Input();
	Resetflag = DQ_Get;
	Delay_us(480);
	DS18B20_Output();
	DQ_H;
	return Resetflag;
}

/**
 * @brief  向DS18B20写数据。
 * @param  data 数据
 * @retval 无
 */
void DS18B20_WriteData(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		DS18B20_Output();
		DQ_L;
		Delay_us(2);
		if (data & 0x01)
		{
			DQ_H;
		}
		else
		{
			DQ_L;
		}
		Delay_us(60);
		DQ_H;
		data = data >> 1;
	}
}

/**
 * @brief  读DS18B20寄存器数据。
 * @param  无
 * @retval data
 */
uint8_t DS18B20_ReadData(void)
{
	uint8_t data = 0;

	for (uint8_t i = 0; i < 8; i++)
	{
		data = data >> 1;
		DS18B20_Output();
		DQ_L;
		Delay_us(2);
		DQ_H;
		Delay_us(2);
		DS18B20_Input();
		if (DQ_Get)
			data |= 0x80;
		Delay_us(60);
	}
	return data;
}

/**
 * @brief  从DS18B20读取温度值。
 * @param  无
 * @retval Temperature 温度值，范围: -55℃到+125℃
 */
float DS18B20_ReadTemp(void)
{
	uint8_t DL, DH;
	uint16_t data;
	uint8_t Tflag = 0; // 正负温度标志. 0:正 | 1:负
	float Temperature = 0;
	DS18B20_Reset();		 // 复位
	DS18B20_WriteData(0xCC); // 跳过ROM检测
	DS18B20_WriteData(0x44); // 启动温度转换
	Delay_ms(750);			 // 延时，等待转换完成
	DS18B20_Reset();		 // 复位
	DS18B20_WriteData(0xCC); // 跳过ROM检测
	DS18B20_WriteData(0xBE); // 读取暂存器指令
	DL = DS18B20_ReadData(); // 读温度低位
	DH = DS18B20_ReadData(); // 读温度高位
	data = DH;
	data = data << 8;
	data |= DL;
	// 读取高五位判断温度的正负，高5位全为0表示为正，全为1表示负
	if ((data & 0xF800) == 0xF800) // 0xF800: 1111 1000
	{
		data = ~data + 0x01;
		Tflag = 1;
	}
	Temperature = data * 0.0625;
	if (Tflag)
	{
		Temperature = -Temperature;
	}
	return Temperature;
}
