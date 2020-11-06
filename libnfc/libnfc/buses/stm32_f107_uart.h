
#ifndef __CORX_STM32_F107_UART__
#define __CORX_STM32_F107_UART__

#include <stm32f10x.h>
#include <stm32f10x_it.h>

void InitUart();
void PutToLogBuffer(u8 ch);
int ReadByte(u32 timeout);
int SendByte(u8 byte, u32 timeout);

#endif//__CORX_STM32_F107_UART__

