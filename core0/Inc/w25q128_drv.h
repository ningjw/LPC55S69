#ifndef __W25Q128_DRV_H
#define __W25Q128_DRV_H

void NorFLASH_WREN(void);
void NorFLASH_Chip_Erase(void);
void NorFLASH_Block_Erase(unsigned int addr);
void NorFLASH_Sector_Erase(unsigned int addr);
void NorFLASH_BufferWrite(unsigned char* pBuffer,unsigned int WriteAddr,unsigned short int NumByteToWrite);
void NorFLASH_Page_Write(unsigned char* pBuffer, unsigned int addr,unsigned short int NumByte);
void NorFLASH_BufferRead(unsigned char* pBuffer ,unsigned int addr,unsigned short int NumByte);
void NorFLASH_Word_Write(unsigned int addr,unsigned int word);
uint32_t  NorFLASH_Word_read(unsigned int addr);
uint16_t NorFLASH_HalfWordRead(uint32_t addr);
void NorFLASH_WriteByte(unsigned char _dat);
unsigned char NorFLASH_ReadByte(void);
unsigned int  NorFLASH_ReadID(void);
void FLASH_Test(void);
void NorFLASH_WaitForWriteEnd(void);



#endif
