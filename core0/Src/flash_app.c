#include "main.h"


uint8_t s_buffer[512] = {0};
AdcInfoTotal adcInfoTotal;
AdcInfo adcInfo;

/***************************************************************************************
  * @brief   ����ص�����Ϣд��Flash
  * @input   
  * @return  
***************************************************************************************/
void Flash_SaveBatPercent()
{
	FLASH_Erase(&flashInstance, BAT_INFO_ADDR, PAGE_SIZE, kFLASH_ApiEraseKey);
	FLASH_Program(&flashInstance, BAT_INFO_ADDR, (uint8_t *)&g_sys_para.batRegAC, 4);
	g_sys_para.batRemainPercentBak = g_sys_para.batRemainPercent;
}


/***************************************************************************************
  * @brief   ����̼���������,�ڽ��ܵ������Ĺ̼�����,���øú���
  * @input   
  * @return  
***************************************************************************************/
void Flash_SaveUpgradePara(void)
{
	FLASH_Erase(&flashInstance, IAP_INFO_ADDR, PAGE_SIZE, kFLASH_ApiEraseKey);
    FLASH_Program(&flashInstance, BAT_INFO_ADDR, &g_sys_para.firmUpdate, 20);
}

/*******************************************************************************
* ������  : STMFLASH_Write
* ����    : ��ָ����ַ��ʼд��ָ�����ȵ�����
* ����    : WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)  pBuffer:����ָ��  NumToWrite:
* ����ֵ  :
*******************************************************************************/
void FLASH_SaveAppData(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite)
{
	uint32_t secpos=0;
	uint16_t secoff=0;
	uint16_t secremain=0;	   
 	uint16_t i=0;    
	uint8_t *FLEXSPI_BUF;
    if(NumByteToWrite == 0) return;
   	FLEXSPI_BUF = s_buffer;	     
 	secpos = WriteAddr/PAGE_SIZE;//������ַ  
	secoff = WriteAddr%PAGE_SIZE;//�������ڵ�ƫ��
	secremain = PAGE_SIZE-secoff;//����ʣ��ռ��С
	
 	if(NumByteToWrite<=secremain)
		secremain=NumByteToWrite;//������4096���ֽ�
	__disable_irq();//�ر��ж�
    while(1) 
	{
		memory_read(secpos*PAGE_SIZE, (uint8_t *)FLEXSPI_BUF, PAGE_SIZE);
		FLASH_Erase(&flashInstance, secpos*PAGE_SIZE, PAGE_SIZE, kFLASH_ApiEraseKey);
        for(i=0;i<secremain;i++)	                    //����
        {
            FLEXSPI_BUF[i+secoff]=pBuffer[i];	  
        }
		FLASH_Program(&flashInstance, secpos*PAGE_SIZE, FLEXSPI_BUF, PAGE_SIZE);
		if(NumByteToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		   	pBuffer+=secremain;  //ָ��ƫ��
			WriteAddr+=secremain;//д��ַƫ��	   
		   	NumByteToWrite-=secremain;				//�ֽ����ݼ�
			if(NumByteToWrite>4096)secremain=4096;	//��һ����������д����
			else secremain=NumByteToWrite;			//��һ����������д����
		}	 
	};
	__enable_irq();
}

/***************************************************************************************
  * @brief   ����һ��ADC����
  * @input   
  * @return  
***************************************************************************************/
void W25Q128_AddAdcData(void)
{
	
}

/***************************************************************************************
  * @brief   ͨ��ʱ���ȡһ��adc����
  * @input   
  * @return  �������ݵ�ַ
***************************************************************************************/
char W25Q128_ReadAdcData(char *adcDataTime)
{
	
}

/***************************************************************************************
  * @brief   ��ȡ����������ݲɼ����ļ���
  * @input   
  * @return  �������ݵ�ַ
***************************************************************************************/
void W25Q148_ReadAdcInfo(int si, int num, char *buf)
{
	
}
