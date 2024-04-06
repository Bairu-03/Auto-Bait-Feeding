#ifndef __DS18B20_H
#define __DS18B20_H

void DS18B20_Init(void);
uint8_t DS18B20_Reset(void);
float DS18B20_ReadTemp(void);

#endif
