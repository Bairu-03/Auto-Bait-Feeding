#ifndef __MYRTC_H
#define __MYRTC_H

void MyRTC_Init(void);
void MyRTC_SetTime(uint16_t *MyRTC_Time);
uint16_t* MyRTC_ReadTime(void);

#endif
