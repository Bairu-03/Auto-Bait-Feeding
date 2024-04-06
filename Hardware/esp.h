#ifndef __esp_H
#define __esp_H

uint8_t esp_Init(void);
uint8_t Esp_PUB(uint16_t Feedtimes, uint8_t Temperature, uint8_t F_ED, uint8_t *FeedInterval);
uint16_t *CommandAnalyse(void);

#endif
