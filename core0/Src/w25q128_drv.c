/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        : 4Kbytes为一个Sector,16个扇区为1个Block,
*                      W25Q128容量为16M字节,共有256个Block,4096个Sector
*******************************************************************************/

#include "main.h"

#define SPI_FLASH_CS_HIGH GPIO_PinWrite(GPIO,BOARD_FLASH_CS_PORT,BOARD_FLASH_CS_PIN,1)
#define SPI_FLASH_CS_LOW  GPIO_PinWrite(GPIO,BOARD_FLASH_CS_PORT,BOARD_FLASH_CS_PIN,0)

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
uint8_t SPI_ReadWriteByte(uint8_t TxData)
{
	#if 0
    while((FLEXCOMM6_PERIPHERAL->FIFOSTAT & SPI_FIFOSTAT_TXNOTFULL_MASK) == 1U);
	FLEXCOMM6_PERIPHERAL->FIFOWR = (0x7<<24) | TxData;		 //发送数据
	
	while((FLEXCOMM6_PERIPHERAL->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 1U);
	return FLEXCOMM6_PERIPHERAL->FIFORD;			//读取数据
	#else
	uint8_t rxData[0];
	spi_transfer_t flashXfer = 
	{
		.rxData = rxData,
		.configFlags = kSPI_FrameAssert,
		.dataSize = 1,
	};
	flashXfer.txData = &TxData,
	SPI_FlashMasterTransfer(FLEXCOMM6_PERIPHERAL, &flashXfer);
	return rxData[0];
	#endif
}


//读取芯片ID W25X16的ID:0XEF14
uint16_t SPI_Flash_ReadID(void)
{
    uint16_t Temp = 0;
    SPI_FLASH_CS_LOW;
    SPI_ReadWriteByte(0x90);//发送读取ID命令
    SPI_ReadWriteByte(0x00);
    SPI_ReadWriteByte(0x00);
    SPI_ReadWriteByte(0x00);
    Temp |= SPI_ReadWriteByte(0xFF)<<8;
    Temp |= SPI_ReadWriteByte(0xFF);
    SPI_FLASH_CS_HIGH;
    return Temp;
}

uint16_t SPI_FLASH_TYPE = W25Q128;//默认就是25Q128
//初始化SPI FLASH的IO口
void SPI_Flash_Init(void)
{
    SPI_FLASH_TYPE = SPI_Flash_ReadID();//读取FLASH ID.
}

//读取SPI FLASH
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void SPI_Flash_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint32_t NumByteToRead)
{
    uint32_t i;
	__disable_irq();
    SPI_FLASH_CS_LOW;                            //使能器件
    SPI_ReadWriteByte(W25X_ReadData);         //暍送读取命令
    SPI_ReadWriteByte((uint8_t)((ReadAddr)>>16));  //暍送24bit地謺
    SPI_ReadWriteByte((uint8_t)((ReadAddr)>>8));
    SPI_ReadWriteByte((uint8_t)ReadAddr);
    for(i=0; i<NumByteToRead; i++)
    {
        pBuffer[i]=SPI_ReadWriteByte(0XFF);   //循粫读数
    }
    SPI_FLASH_CS_HIGH;                            //取消片选
	__enable_irq();
}
uint8_t SPI_Flash_ReadSR(void)
{
    uint8_t byte=0;
    SPI_FLASH_CS_LOW;                            //使能器件
    SPI_ReadWriteByte(W25X_ReadStatusReg);    //暍送读取状态寄存器命令
    byte=SPI_ReadWriteByte(0Xff);             //读取一个字节
    SPI_FLASH_CS_HIGH;                            //取消片选
    return byte;
}
//等待空闲
void SPI_Flash_Wait_Busy(void)
{
    while ((SPI_Flash_ReadSR()&0x01)==0x01);   // 等待BUSY位清空
}
//SPI_FLASH写使能
//将WEL置位
void SPI_FLASH_Write_Enable(void)
{
    SPI_FLASH_CS_LOW;                            //使能器件
    SPI_ReadWriteByte(W25X_WriteEnable);      //暍送写使能
    SPI_FLASH_CS_HIGH;                            //取消片选
}
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地謺开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地謺(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
void SPI_Flash_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint32_t i;
    SPI_FLASH_Write_Enable();                  //SET WEL
    SPI_FLASH_CS_LOW;                            //使能器件
    SPI_ReadWriteByte(W25X_PageProgram);      //暍送写页命令
    SPI_ReadWriteByte((uint8_t)((WriteAddr)>>16)); //暍送24bit地謺
    SPI_ReadWriteByte((uint8_t)((WriteAddr)>>8));
    SPI_ReadWriteByte((uint8_t)WriteAddr);
    for(i=0; i<NumByteToWrite; i++)SPI_ReadWriteByte(pBuffer[i]); //循粫写数
    SPI_FLASH_CS_HIGH;                            //取消片选
    SPI_Flash_Wait_Busy();					   //等待写入结束
}
//无检验写SPI FLASH
//必须葧保所写的地謺暥围内的数据全部为0XFF,曬则在暻0XFF处写入的数据将失败!
//具有自动换页功能
//在指定地謺开始写入指定长度的数据,但是要葧保地謺不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地謺(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite)
{
    uint32_t pageremain;
    pageremain=256-WriteAddr%256; //单页剩余的字节数
    if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节

    while(1)
    {
        SPI_Flash_Write_Page(pBuffer,WriteAddr,pageremain);
        if(NumByteToWrite==pageremain)break;//写入结束了
        else //NumByteToWrite>pageremain
        {
            pBuffer+=pageremain;
            WriteAddr+=pageremain;

            NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
            if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
            else pageremain=NumByteToWrite; 	  //不够256个字节了
        }
    };

}


//写SPI_FLASH状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void SPI_FLASH_Write_SR(uint8_t sr)
{
    SPI_FLASH_CS_LOW;                            //使能器件
    SPI_ReadWriteByte(W25X_WriteStatusReg);   //暍送写取状态寄存器命令
    SPI_ReadWriteByte(sr);               //写入一个字节
    SPI_FLASH_CS_HIGH;                            //取消片选
}

//SPI_FLASH写禁止
//将WEL清零
void SPI_FLASH_Write_Disable(void)
{
    SPI_FLASH_CS_LOW;                            //使能器件
    SPI_ReadWriteByte(W25X_WriteDisable);     //暍送写禁止指令
    SPI_FLASH_CS_HIGH;                            //取消片选
}
//擦除一个扇区
//Dst_Addr:扇区地謺 0~511 for w25x16
//擦除一个山区的最少时间:150ms
void SPI_Flash_Erase_Sector(uint32_t Dst_Addr)
{
    SPI_FLASH_Write_Enable();                  //SET WEL
    SPI_Flash_Wait_Busy();
	SPI_FLASH_CS_LOW;                            //使能器件
    SPI_ReadWriteByte(W25X_SectorErase);      //暍送扇区擦除指令
    SPI_ReadWriteByte((uint8_t)((Dst_Addr)>>16));  //暍送24bit地謺
    SPI_ReadWriteByte((uint8_t)((Dst_Addr)>>8));
    SPI_ReadWriteByte((uint8_t)Dst_Addr);
    SPI_FLASH_CS_HIGH;                            //取消片选
    SPI_Flash_Wait_Busy();   				   //等待擦除完成
}

//擦除一个扇区
//Dst_Addr:扇区地謺 0~511 for w25x16
//擦除一个山区的最少时间:150ms
void SPI_Flash_Erase_Chip(void)
{
    SPI_FLASH_Write_Enable();                  //SET WEL
    SPI_Flash_Wait_Busy();
	SPI_FLASH_CS_LOW;                            //使能器件
    SPI_ReadWriteByte(W25X_ChipErase);      //暍送扇区擦除指令
    SPI_FLASH_CS_HIGH;                            //取消片选
    SPI_Flash_Wait_Busy();   				   //等待擦除完成
}


//写SPI FLASH
//在指定地謺开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地謺(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
__ALIGNED(4) uint8_t SPI_FLASH_BUF[4096];
void SPI_Flash_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite)
{
    uint32_t secpos;
    uint32_t secoff;
    uint32_t secremain;
    uint32_t i;
	if(NumByteToWrite == 0) 
		return;
    secpos = WriteAddr/4096;//扇区地謺 0~511 for w25x16
    secoff = WriteAddr%4096;//在扇区内的偏移
    secremain=4096-secoff;//扇区剩余空间大小

    if(NumByteToWrite<=secremain)
		secremain=NumByteToWrite;//不大于4096个字节
	
	__disable_irq();//关闭中断
    while(1)
    {
        SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
		SPI_Flash_Erase_Sector(WriteAddr);//擦除这个扇区
		for(i=0; i<secremain; i++)	 //复制
		{
			SPI_FLASH_BUF[i+secoff] = pBuffer[i];
		}
		SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区

        if(NumByteToWrite==secremain)
			break;//写入结束了
        else//写入未结束
        {
            secpos++;//扇区地謺增1
            secoff=0;//偏移位置为0

            pBuffer+=secremain;  //指针偏移
            WriteAddr+=secremain;//写地謺偏移
            NumByteToWrite-=secremain;				//字节数递减
            if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
            else secremain=NumByteToWrite;			//下一个扇区可以写完了
        }
    };
	__enable_irq();
}
/****************************************END OF FILE*******************************************************/












