#include "main.h"


void main(void)
{
	BOARD_BootClockRUN();
	BOARD_InitPins();
	BOARD_InitPeripherals();

	/* 初始化EventRecorder并开启*/
	EventRecorderInitialize(EventRecordAll, 1U);
	EventRecorderStart();
	printf("hello\r\n");
}

void PINT0_CallBack(pint_pin_int_t pintr, uint32_t pmatch_status)
{
	
}