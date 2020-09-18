#ifndef __FLASH_APP_H
#define __FLASH_APP_H

extern uint8_t s_buffer[512];
void Flash_SaveBatPercent();
void Flash_SaveUpgradePara(void);
void FLASH_SaveAppData(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite);
	
void W25Q128_AddAdcData(void);
char W25Q128_ReadAdcData(char *adcDataTime);
void W25Q128_ReadAdcInfo(int si, int num, char *buf);


#endif
