#include "main.h"


void main(void)
{
	BOARD_BootClockRUN();
	BOARD_InitPins();
	BOARD_InitPeripherals();

	/* ��ʼ��EventRecorder������*/
	EventRecorderInitialize(EventRecordAll, 1U);
	EventRecorderStart();
	printf("hello\r\n");
}

