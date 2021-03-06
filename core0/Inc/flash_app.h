#ifndef __FLASH_APP_H
#define __FLASH_APP_H

extern uint8_t s_buffer[512];
void Flash_SavePara(void);
void Flash_ReadPara(void);
void Flash_WriteAdcData(uint8_t *buff, int len);
void LPC55S69_FlashSaveData(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite);
void W25Q128_AddAdcData(void);
char W25Q128_ReadAdcData(char *adcDataTime);
void W25Q128_ReadAdcInfo(int si, int num, char *buf);


#endif
