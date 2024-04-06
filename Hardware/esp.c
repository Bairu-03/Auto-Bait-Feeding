#include "stm32f10x.h" // Device header
#include "MyUSART.h"
#include <stdio.h>
#include <string.h>
#include "Delay.h"

extern char RECS[250];
extern char Feed_ED;
extern uint8_t FeedInterval[3];

const char *WIFI = "vivo";
const char *WIFIASSWORD = "12345678";

int fputc(int ch, FILE *f) // printf重定向
{
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        ;
    return ch;
}

/**
 * @brief  ESP8266初始化
 * @param  无
 * @retval 错误码
 * 0:初始化成功 |
 * 1:关闭回显失败 |
 * 2:切换混合模式失败 |
 * 3:联网失败 |
 * 4:时区校准失败 |
 * 5:上传用户配置信息失败 |
 * 6:上传MQTT标识符失败 |
 * 7:连接MQTT Broker失败 |
 * 8:订阅消息失败
 */
uint8_t esp_Init(void)
{
    memset(RECS, 0, sizeof(RECS));
    printf("AT+RST\r\n"); // 重启
    Delay_ms(2000);

    memset(RECS, 0, sizeof(RECS));
    printf("ATE0\r\n"); // 关闭回显
    Delay_ms(10);
    if (strcmp(RECS, "OK"))
        return 1;

    printf("AT+CWMODE=3\r\n"); // 混合模式
    Delay_ms(500);
    if (strcmp(RECS, "OK"))
        return 2;

    memset(RECS, 0, sizeof(RECS));
    printf("AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI, WIFIASSWORD); // 连接热点
    Delay_ms(2000);
    if (strcmp(RECS, "OK"))
        return 3;

    memset(RECS, 0, sizeof(RECS));
    printf("AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"\r\n"); // 校准时区
    Delay_ms(200);
    if (strcmp(RECS, "OK"))
        return 4;

    memset(RECS, 0, sizeof(RECS)); // 用户信息配置
    printf("AT+MQTTUSERCFG=0,1,\"NULL\",\"tyma110&a1IZ6nPksSi\",\"BA2ECBA29B0FDD0C4E244399920A5551D24E0D55\",0,0,\"\"\r\n");
    Delay_ms(1000);
    if (strcmp(RECS, "OK"))
        return 5;

    memset(RECS, 0, sizeof(RECS)); // 上传MQTT标识符
    printf("AT+MQTTCLIENTID=0,\"1234|securemode=3\\,signmethod=hmacsha1|\"\r\n");
    Delay_ms(200);
    if (strcmp(RECS, "OK"))
        return 6;

    memset(RECS, 0, sizeof(RECS)); // 连接 MQTT Broker
    printf("AT+MQTTCONN=0,\"a1IZ6nPksSi.iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883,1\r\n+MQTTCONNECTED:0,1,\"a1IZ6nPksSi.iot-as-mqtt.cn-shanghai.aliyuncs.com\",\"1883\",\"\",1\r\n");
    Delay_ms(1500);
    if (strcmp(RECS, "OK"))
        return 7;

    printf("AT+MQTTSUB=0,\"/sys/a1IZ6nPksSi/tyma110/thing/service/property/set\",1\r\n"); // 订阅消息
    Delay_ms(1000);
    if (strcmp(RECS, "OK"))
        return 8;
    memset(RECS, 0, sizeof(RECS));
    return 0;
}

/**
 * @brief  经ESP上传数据
 * @param  Feedtimes 投饵计次
 * @param  Temperature 温度
 * @param  F_ED 自动投饵开关. '1':启用 | '0':禁用
 * @param  FeedInterval 投饵间隔
 * @retval 发送状态. 1:发送失败 | 0:发送成功
 */
uint8_t Esp_PUB(uint16_t Feedtimes, uint8_t Temperature, uint8_t F_ED, uint8_t *FeedInterval)
{
    if (F_ED == '1')
        F_ED = 1;
    if (F_ED == '0')
        F_ED = 0;
    memset(RECS, 0, sizeof(RECS));
    printf("AT+MQTTPUB=0,\"/sys/a1IZ6nPksSi/tyma110/thing/event/property/post\",\"{\\\"method\\\":\\\"thing.event.property.post\\\"\\,\\\"params\\\":{\\\"Feedtimes\\\":%d\\,\\\"Temperature\\\":%d\\,\\\"Feed_ED\\\":%d\\,\\\"FeedInterval_h\\\":%d\\,\\\"FeedInterval_m\\\":%d\\,\\\"FeedInterval_s\\\":%d}}\",0,0\r\n", Feedtimes, Temperature, F_ED, FeedInterval[0], FeedInterval[1], FeedInterval[2]);
    Delay_ms(1000); // 延时等待数据接收完成
    if (strcmp(RECS, "ERROR") == 0)
        return 1;
    return 0;
}

/**
 * @brief  平台回传信息解析
 * @param  无
 * @retval ESPparam 数组 [0]:投饵间隔 | [1]:自动投饵开关状态
 */
void CommandAnalyse(void)
{
    if (strncmp(RECS, "+MQTTSUBRECV:0", 14) == 0)
    {
        uint8_t i = 0;
        uint8_t FIh[2], FIm[2], FIs[2];
        uint8_t j = 0;
        while (RECS[i++] != '\0')
        {
            if (strncmp((RECS + i), "Feed_ED", 7) == 0)
            {
                while (RECS[i++] != ':')
                    ;
                Feed_ED = RECS[i];
            }

            if (strncmp((RECS + i), "FeedInterval_h", 14) == 0)
            {
                while (RECS[i++] != ':')
                    ;
                do
                {
                    FIh[j++] = RECS[i];
                } while (RECS[i++] != '}');
            }

            if (strncmp((RECS + i), "FeedInterval_m", 14) == 0)
            {
                while (RECS[i++] != ':')
                    do
                    {
                        FIm[j++] = RECS[i];
                    } while (RECS[i++] != '}');
            }

            if (strncmp((RECS + i), "FeedInterval_s", 14) == 0)
            {
                while (RECS[i++] != ':')
                    do
                    {
                        FIs[j++] = RECS[i];
                    } while (RECS[i++] != '}');
            }
        }
    }
}
