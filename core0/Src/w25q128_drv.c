/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        : 4KbytesΪһ��Sector,16������Ϊ1��Block,
*                      W25Q128����Ϊ16M�ֽ�,����256��Block,4096��Sector
*******************************************************************************/

#include "main.h"

#define SPI_FLASH_CS_HIGH GPIO_PinWrite(GPIO,BOARD_FLASH_CS_PORT,BOARD_FLASH_CS_PIN,1)
#define SPI_FLASH_CS_LOW  GPIO_PinWrite(GPIO,BOARD_FLASH_CS_PORT,BOARD_FLASH_CS_PIN,0)

//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
uint8_t SPI_ReadWriteByte(uint8_t TxData)
{
	#if 0
    while((FLEXCOMM6_PERIPHERAL->FIFOSTAT & SPI_FIFOSTAT_TXNOTFULL_MASK) == 1U);
	FLEXCOMM6_PERIPHERAL->FIFOWR = (0x7<<24) | TxData;		 //��������
	
	while((FLEXCOMM6_PERIPHERAL->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 1U);
	return FLEXCOMM6_PERIPHERAL->FIFORD;			//��ȡ����
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


//��ȡоƬID W25X16��ID:0XEF14
uint16_t SPI_Flash_ReadID(void)
{
    uint16_t Temp = 0;
    SPI_FLASH_CS_LOW;
    SPI_ReadWriteByte(0x90);//���Ͷ�ȡID����
    SPI_ReadWriteByte(0x00);
    SPI_ReadWriteByte(0x00);
    SPI_ReadWriteByte(0x00);
    Temp |= SPI_ReadWriteByte(0xFF)<<8;
    Temp |= SPI_ReadWriteByte(0xFF);
    SPI_FLASH_CS_HIGH;
    return Temp;
}

uint16_t SPI_FLASH_TYPE = W25Q128;//Ĭ�Ͼ���25Q128
//��ʼ��SPI FLASH��IO��
void SPI_Flash_Init(void)
{
    SPI_FLASH_TYPE = SPI_Flash_ReadID();//��ȡFLASH ID.
}

//��ȡSPI FLASH
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void SPI_Flash_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint32_t NumByteToRead)
{
    uint32_t i;
	__disable_irq();
    SPI_FLASH_CS_LOW;                            //ʹ������
    SPI_ReadWriteByte(W25X_ReadData);         //���Ͷ�ȡ����
    SPI_ReadWriteByte((uint8_t)((ReadAddr)>>16));  //����24bit��֕
    SPI_ReadWriteByte((uint8_t)((ReadAddr)>>8));
    SPI_ReadWriteByte((uint8_t)ReadAddr);
    for(i=0; i<NumByteToRead; i++)
    {
        pBuffer[i]=SPI_ReadWriteByte(0XFF);   //ѭ������
    }
    SPI_FLASH_CS_HIGH;                            //ȡ��Ƭѡ
	__enable_irq();
}
uint8_t SPI_Flash_ReadSR(void)
{
    uint8_t byte=0;
    SPI_FLASH_CS_LOW;                            //ʹ������
    SPI_ReadWriteByte(W25X_ReadStatusReg);    //���Ͷ�ȡ״̬�Ĵ�������
    byte=SPI_ReadWriteByte(0Xff);             //��ȡһ���ֽ�
    SPI_FLASH_CS_HIGH;                            //ȡ��Ƭѡ
    return byte;
}
//�ȴ�����
void SPI_Flash_Wait_Busy(void)
{
    while ((SPI_Flash_ReadSR()&0x01)==0x01);   // �ȴ�BUSYλ���
}
//SPI_FLASHдʹ��
//��WEL��λ
void SPI_FLASH_Write_Enable(void)
{
    SPI_FLASH_CS_LOW;                            //ʹ������
    SPI_ReadWriteByte(W25X_WriteEnable);      //����дʹ��
    SPI_FLASH_CS_HIGH;                            //ȡ��Ƭѡ
}
//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����֕��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�֕(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!
void SPI_Flash_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint32_t i;
    SPI_FLASH_Write_Enable();                  //SET WEL
    SPI_FLASH_CS_LOW;                            //ʹ������
    SPI_ReadWriteByte(W25X_PageProgram);      //����дҳ����
    SPI_ReadWriteByte((uint8_t)((WriteAddr)>>16)); //����24bit��֕
    SPI_ReadWriteByte((uint8_t)((WriteAddr)>>8));
    SPI_ReadWriteByte((uint8_t)WriteAddr);
    for(i=0; i<NumByteToWrite; i++)SPI_ReadWriteByte(pBuffer[i]); //ѭ��д��
    SPI_FLASH_CS_HIGH;                            //ȡ��Ƭѡ
    SPI_Flash_Wait_Busy();					   //�ȴ�д�����
}
//�޼���дSPI FLASH
//����ȕ����д�ĵ�֕��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڕ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ����
//��ָ����֕��ʼд��ָ�����ȵ�����,����Ҫȕ����֕��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�֕(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite)
{
    uint32_t pageremain;
    pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���
    if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//������256���ֽ�

    while(1)
    {
        SPI_Flash_Write_Page(pBuffer,WriteAddr,pageremain);
        if(NumByteToWrite==pageremain)break;//д�������
        else //NumByteToWrite>pageremain
        {
            pBuffer+=pageremain;
            WriteAddr+=pageremain;

            NumByteToWrite-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
            if(NumByteToWrite>256)pageremain=256; //һ�ο���д��256���ֽ�
            else pageremain=NumByteToWrite; 	  //����256���ֽ���
        }
    };

}


//дSPI_FLASH״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
void SPI_FLASH_Write_SR(uint8_t sr)
{
    SPI_FLASH_CS_LOW;                            //ʹ������
    SPI_ReadWriteByte(W25X_WriteStatusReg);   //����дȡ״̬�Ĵ�������
    SPI_ReadWriteByte(sr);               //д��һ���ֽ�
    SPI_FLASH_CS_HIGH;                            //ȡ��Ƭѡ
}

//SPI_FLASHд��ֹ
//��WEL����
void SPI_FLASH_Write_Disable(void)
{
    SPI_FLASH_CS_LOW;                            //ʹ������
    SPI_ReadWriteByte(W25X_WriteDisable);     //����д��ָֹ��
    SPI_FLASH_CS_HIGH;                            //ȡ��Ƭѡ
}
//����һ������
//Dst_Addr:������֕ 0~511 for w25x16
//����һ��ɽ��������ʱ��:150ms
void SPI_Flash_Erase_Sector(uint32_t Dst_Addr)
{
    SPI_FLASH_Write_Enable();                  //SET WEL
    SPI_Flash_Wait_Busy();
	SPI_FLASH_CS_LOW;                            //ʹ������
    SPI_ReadWriteByte(W25X_SectorErase);      //������������ָ��
    SPI_ReadWriteByte((uint8_t)((Dst_Addr)>>16));  //����24bit��֕
    SPI_ReadWriteByte((uint8_t)((Dst_Addr)>>8));
    SPI_ReadWriteByte((uint8_t)Dst_Addr);
    SPI_FLASH_CS_HIGH;                            //ȡ��Ƭѡ
    SPI_Flash_Wait_Busy();   				   //�ȴ��������
}

//����һ������
//Dst_Addr:������֕ 0~511 for w25x16
//����һ��ɽ��������ʱ��:150ms
void SPI_Flash_Erase_Chip(void)
{
    SPI_FLASH_Write_Enable();                  //SET WEL
    SPI_Flash_Wait_Busy();
	SPI_FLASH_CS_LOW;                            //ʹ������
    SPI_ReadWriteByte(W25X_ChipErase);      //������������ָ��
    SPI_FLASH_CS_HIGH;                            //ȡ��Ƭѡ
    SPI_Flash_Wait_Busy();   				   //�ȴ��������
}


//дSPI FLASH
//��ָ����֕��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�֕(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
__ALIGNED(4) uint8_t SPI_FLASH_BUF[4096];
void SPI_Flash_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite)
{
    uint32_t secpos;
    uint32_t secoff;
    uint32_t secremain;
    uint32_t i;
	if(NumByteToWrite == 0) 
		return;
    secpos = WriteAddr/4096;//������֕ 0~511 for w25x16
    secoff = WriteAddr%4096;//�������ڵ�ƫ��
    secremain=4096-secoff;//����ʣ��ռ��С

    if(NumByteToWrite<=secremain)
		secremain=NumByteToWrite;//������4096���ֽ�
	
	__disable_irq();//�ر��ж�
    while(1)
    {
        SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//������������������
		SPI_Flash_Erase_Sector(WriteAddr);//�����������
		for(i=0; i<secremain; i++)	 //����
		{
			SPI_FLASH_BUF[i+secoff] = pBuffer[i];
		}
		SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//д����������

        if(NumByteToWrite==secremain)
			break;//д�������
        else//д��δ����
        {
            secpos++;//������֕��1
            secoff=0;//ƫ��λ��Ϊ0

            pBuffer+=secremain;  //ָ��ƫ��
            WriteAddr+=secremain;//д��֕ƫ��
            NumByteToWrite-=secremain;				//�ֽ����ݼ�
            if(NumByteToWrite>4096)secremain=4096;	//��һ����������д����
            else secremain=NumByteToWrite;			//��һ����������д����
        }
    };
	__enable_irq();
}
/****************************************END OF FILE*******************************************************/












