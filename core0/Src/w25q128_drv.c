/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        : Flash�������ǣ�д����ֻ�ܽ�1дΪ0���������ݾ���д1
*                      �����������������ݵ�falsh��д���µ����ݣ�������Ȳ�����
*                      ����Flash�ڲ�����ʱ������������
*******************************************************************************/

#include "main.h"


#define FLASH_CS_LOW    GPIO_PinWrite(GPIO, BOARD_FLASH_CS_PORT ,BOARD_FLASH_CS_PIN ,0);
#define FLASH_CS_HIGH   GPIO_PinWrite(GPIO, BOARD_FLASH_CS_PORT ,BOARD_FLASH_CS_PIN ,1);
#define FLASH_CLK_Low   GPIO_PinWrite(GPIO, BOARD_FLASH_SCK_PORT,BOARD_FLASH_SCK_PIN,0);
#define FLASH_CLK_High  GPIO_PinWrite(GPIO, BOARD_FLASH_SCK_PORT,BOARD_FLASH_SCK_PIN,1);
#define FLASH_DI_Low    GPIO_PinWrite(GPIO, BOARD_FLASH_MOSI_PORT ,BOARD_FLASH_MOSI_PIN ,0);
#define FLASH_DI_High   GPIO_PinWrite(GPIO, BOARD_FLASH_MOSI_PORT ,BOARD_FLASH_MOSI_PIN ,1);
#define FLASH_WP_Low    GPIO_PinWrite(GPIO, BOARD_FLASH_WP_PORT ,BOARD_FLASH_WP_PIN ,0)
#define FLASH_WP_High	GPIO_PinWrite(GPIO, BOARD_FLASH_WP_PORT ,BOARD_FLASH_WP_PIN ,1)
#define FLASH_Read_DO   GPIO_PinRead(GPIO, BOARD_FLASH_MISO_PORT, BOARD_FLASH_MISO_PIN)
#define SPI_FLASH_PageSize  256

/*******************************************************************************
* Function Name  : MX25L128_BufferWrite
* Description    : Writes block of data to the FLASH. In this function, the
*                  number of WRITE cycles are reduced, using Page WRITE sequence.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void NorFLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

    Addr = WriteAddr % SPI_FLASH_PageSize;
    count = SPI_FLASH_PageSize - Addr;
    NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
    NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

    if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
    {
        if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
        {
            NorFLASH_Page_Write(pBuffer, WriteAddr, NumByteToWrite);
        }
        else /* NumByteToWrite > SPI_FLASH_PageSize */
        {
            while (NumOfPage--)
            {
                NorFLASH_Page_Write(pBuffer, WriteAddr,SPI_FLASH_PageSize);
                WriteAddr +=  SPI_FLASH_PageSize;
                pBuffer += SPI_FLASH_PageSize;
            }

            NorFLASH_Page_Write(pBuffer, WriteAddr, NumOfSingle);
        }
    }
    else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
    {
        if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
        {
            if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
            {
                temp = NumOfSingle - count;
                NorFLASH_Page_Write(pBuffer, WriteAddr, count);
                WriteAddr +=  count;
                pBuffer += count;

                NorFLASH_Page_Write(pBuffer, WriteAddr, temp);
            }
            else
            {
                NorFLASH_Page_Write(pBuffer, WriteAddr, NumByteToWrite);
            }
        }
        else /* NumByteToWrite > SPI_FLASH_PageSize */
        {
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
            NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

            NorFLASH_Page_Write(pBuffer, WriteAddr, count);
            WriteAddr +=  count;
            pBuffer += count;

            while (NumOfPage--)
            {
                NorFLASH_Page_Write(pBuffer, WriteAddr, SPI_FLASH_PageSize);
                WriteAddr +=  SPI_FLASH_PageSize;
                pBuffer += SPI_FLASH_PageSize;
            }

            if (NumOfSingle != 0)
            {
                NorFLASH_Page_Write(pBuffer, WriteAddr, NumOfSingle);
            }
        }
    }
}

/*******************************************************************************
* Function Name  :
* Description    : flash һ��pageΪ256byte ,���øú���ʱҪȷ��д��ĵ�ַaddr���Ѿ���������
*******************************************************************************/
void NorFLASH_Page_Write(uint8_t* pBuffer,uint32_t addr,uint16_t NumByte)
{
    NorFLASH_WREN();
    FLASH_CS_LOW;
    NorFLASH_WriteByte(0x02);
    NorFLASH_WriteByte( (addr >> 16)&0xFF );
    NorFLASH_WriteByte( (addr >> 8)&0xFF );
    NorFLASH_WriteByte( addr&0xFF );
    while (NumByte--)
    {
        /* Send the current byte */
        NorFLASH_WriteByte(* pBuffer);
        /* Point on the next byte to be written */
        pBuffer++;
    }
    FLASH_CS_HIGH;
    NorFLASH_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NorFLASH_BufferRead(uint8_t* pBuffer,uint32_t addr,uint16_t NumByte)
{
    FLASH_CS_LOW;
    NorFLASH_WriteByte(0x03);
    NorFLASH_WriteByte( (addr >> 16)&0xFF );
    NorFLASH_WriteByte( (addr >> 8)&0xFF );
    NorFLASH_WriteByte( addr&0xFF );
    while (NumByte--) /* while there is data to be read */
    {
        * pBuffer = NorFLASH_ReadByte();
        pBuffer++;
    }
    FLASH_CS_HIGH;
}





/*******************************************************************************
* Function Name  : NorFLASH_Word_Write
* Description    : д��һ��word
* Input          : - addr : FLASH's internal address to write to.
                    -word :the data to be read to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void NorFLASH_Word_Write(uint32_t addr,uint32_t word)
{

    NorFLASH_WaitForWriteEnd();
    NorFLASH_WREN();
    FLASH_CS_LOW;
    NorFLASH_WriteByte(0x02);
    NorFLASH_WriteByte( (addr >> 16)&0xFF );
    NorFLASH_WriteByte( (addr >> 8)&0xFF );
    NorFLASH_WriteByte( addr&0xFF );

    NorFLASH_WriteByte((uint8_t)(word>>24));
    NorFLASH_WriteByte((uint8_t)(word>>16));
    NorFLASH_WriteByte((uint8_t)(word>>8));
    NorFLASH_WriteByte((uint8_t)word);

    FLASH_CS_HIGH;
    NorFLASH_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : NorFLASH_Word_read
* Description    : ��һ��word
* Input          : - addr : FLASH's internal address to read to.
* Output         : None
* Return         : TempData ��Flash�ж�ȡ��������
*******************************************************************************/
uint32_t NorFLASH_Word_read(uint32_t addr)
{
    uint8_t  TempDataArray[4];
    uint32_t TempData;
    NorFLASH_WaitForWriteEnd();
    FLASH_CS_LOW;
    NorFLASH_WriteByte(0x03);
    NorFLASH_WriteByte( (addr >> 16)&0xFF );
    NorFLASH_WriteByte( (addr >> 8)&0xFF );
    NorFLASH_WriteByte( addr&0xFF );

    TempDataArray[0] = NorFLASH_ReadByte();
    TempDataArray[1] = NorFLASH_ReadByte();
    TempDataArray[2] = NorFLASH_ReadByte();
    TempDataArray[3] = NorFLASH_ReadByte();
    FLASH_CS_HIGH;
    NorFLASH_WaitForWriteEnd();
    TempData = (TempDataArray[0]<<24)|(TempDataArray[1]<<16)|(TempDataArray[2]<<8)|(TempDataArray[3]);
    return TempData;
}


/*******************************************************************************
* Function Name  : NorFLASH_HalfWordRead
* Description    : ��һ��halfword
* Input          : - addr : FLASH's internal address to read to.
* Output         : None
* Return         : TempData ��Flash�ж�ȡ��������
*******************************************************************************/
uint16_t NorFLASH_HalfWordRead(uint32_t addr)
{
    uint8_t  TempDataArray[2];
    uint16_t TempData;

    NorFLASH_WaitForWriteEnd();
    FLASH_CS_LOW;
    NorFLASH_WriteByte(0x03);
    NorFLASH_WriteByte( (addr >> 16)&0xFF );
    NorFLASH_WriteByte( (addr >> 8)&0xFF );
    NorFLASH_WriteByte( addr&0xFF );

    TempDataArray[0] = NorFLASH_ReadByte();
    TempDataArray[1] = NorFLASH_ReadByte();

    FLASH_CS_HIGH;
    NorFLASH_WaitForWriteEnd();
    TempData = ( (TempDataArray[0]<<8) | TempDataArray[1] );
    return TempData;
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
uint32_t NorFLASH_ReadID(void)
{
    uint32_t TempID;

    FLASH_CS_LOW;
    NorFLASH_WriteByte(0x9F);
    TempID = NorFLASH_ReadByte();		            //Manufacturer Identification
    TempID = (TempID<<8)| NorFLASH_ReadByte();	//Device Identification
    TempID = (TempID<<8)| NorFLASH_ReadByte();
    FLASH_CS_HIGH;
    return TempID;
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void NorFLASH_WriteByte(unsigned char _dat)
{
    unsigned char i;//����ѭ��
    unsigned char Flag_Bit = 0x80;//����д������ݵĸ�λ��MSB����ʼ

    for(i = 0; i < 8; i++)
    {
        FLASH_CLK_Low;
        if( (_dat & Flag_Bit) == 0 )//�ж�_dat�ĵ�Flag_Bitλ�ϵ�����
        {
            FLASH_DI_Low;//����͵�ƽ
        } else
        {
            FLASH_DI_High;//����ߵ�ƽ
        }
        Flag_Bit >>= 1;
        FLASH_CLK_High;
    }
    FLASH_CLK_Low;
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
uint8_t NorFLASH_ReadByte(void)
{
    uint8_t i;
    uint8_t tmpData = 0;
    for(i = 0; i < 8; i++)
    {
        FLASH_CLK_Low;
        if( FLASH_Read_DO == 1 )
        {
            tmpData = (tmpData | (0x80>>i));
        }
        FLASH_CLK_High;
    }
    FLASH_CLK_Low;
    return tmpData;
}



/*******************************************************************************
* Function Name  :
* Description    : ��������оƬ��ȫ��д1.
*******************************************************************************/
void NorFLASH_Chip_Erase(void)
{
    NorFLASH_WREN();
    FLASH_CS_LOW;
    NorFLASH_WriteByte(0xC7);
    FLASH_CS_HIGH;
    NorFLASH_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : NorFLASH_Block_Erase
* Description    : ����һ��Block 64k
*******************************************************************************/
void NorFLASH_Block_Erase(unsigned int addr)
{
    NorFLASH_WREN();
    FLASH_CS_LOW;
    NorFLASH_WriteByte(0xD8);
    NorFLASH_WriteByte( (addr >> 16)&0xFF );
    NorFLASH_WriteByte( (addr >> 8)&0xFF );
    NorFLASH_WriteByte( addr&0xFF );
    FLASH_CS_HIGH;
    NorFLASH_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : NorFLASH_Sector_Erase
* Description    : flashһ��sector��СΪ4096��0xFFF������16��page���
*                  ���øú���ʱ��Ҫȷ����16��page����������û����Ѿ�����
*******************************************************************************/
void NorFLASH_Sector_Erase(unsigned int addr)
{
    NorFLASH_WREN();
    FLASH_CS_LOW;
    NorFLASH_WriteByte(0x20);
    NorFLASH_WriteByte( (addr >> 16)&0xFF );
    NorFLASH_WriteByte( (addr >> 8)&0xFF );
    NorFLASH_WriteByte( addr&0xFF );
    FLASH_CS_HIGH;
    NorFLASH_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : NorFLASH_WREN
* Description    :
*******************************************************************************/
void NorFLASH_WREN(void)
{
    FLASH_CS_LOW;
    NorFLASH_WriteByte(0x06);
    FLASH_CS_HIGH;
}


/*******************************************************************************
* Function Name  :
* Description    :
- ���¶�����ʹ�ñ�־λ�ĵĵ�һλWEL BIT����
- Write Disable (WRDI) command completion
- Write Status Register (WRSR) command completion
- Page Program (PP, 4PP) command completion
- Continuously Program mode (CP) instruction completion
- Sector Erase (SE) command completion
- Block Erase (BE, BE32K) command completion
- Chip Erase (CE) command completion
- Single Block Lock/Unlock (SBLK/SBULK) instruction completion
- Gang Block Lock/Unlock (GBLK/GBULK) instruction completion
*******************************************************************************/
void NorFLASH_WaitForWriteEnd(void)
{
    uint8_t FLASH_Status = 0;
    /* Select the FLASH: Chip Select low */
    FLASH_CS_LOW;
    /* Send "Read Status Register" instruction */
    NorFLASH_WriteByte(0x05);
    /* Loop as long as the memory is busy with a write cycle */
    do
    {
        FLASH_Status = NorFLASH_ReadByte();
    }
    while ((FLASH_Status & 0x01) == 1); /* Write in progress */
    /* Deselect the FLASH: Chip Select high */
    FLASH_CS_HIGH;
}
/****************************************END OF FILE*******************************************************/












