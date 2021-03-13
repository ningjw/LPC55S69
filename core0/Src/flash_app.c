#include "main.h"



extern rtc_datetime_t sampTime;

uint8_t s_buffer[512] = {0};

AdcInfoTotal adcInfoTotal;
AdcInfo adcInfo;
uint32_t inFlashBuf[128] = {0};
void Flash_SavePara(void)
{
	memory_erase(SYS_PARA_ADDR,PAGE_SIZE);
	memory_write(SYS_PARA_ADDR,(uint8_t *)&g_sys_flash_para, sizeof(SysFlashPara));
	g_sys_flash_para.batRemainPercentBak = g_sys_para.batRemainPercent;
    
    memory_erase(SAMPLE_PARA_ADDR, sizeof(g_sample_para));
	LPC55S69_FlashSaveData((uint8_t *)&g_sample_para, SAMPLE_PARA_ADDR, sizeof(g_sample_para));
}

void Flash_ReadPara(void)
{
	uint16_t i = 0;
	memory_read(SYS_PARA_ADDR, (uint8_t *)&g_sys_flash_para, sizeof(SysFlashPara));
	
    memory_read(SAMPLE_PARA_ADDR, (uint8_t *)&g_sample_para, sizeof(g_sample_para));
    
	//ǰ12�ֽڱ������ adcInfoTotal �ṹ��
	SPI_Flash_Read((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_ADDR, sizeof(adcInfoTotal));
	
	//�ж�Ϊ�״��ϵ�����
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > ADC_MAX_NUM){
		DEBUG_PRINTF("%s: First run, init adcInfoTotal**************\r\n",__func__);
		//����Ϊ��ʼ��Ϊ0
		adcInfoTotal.totalAdcInfo = 0;
		//���� AdcInfo �ṹ�屣���ַ
		adcInfoTotal.addrOfNewInfo = ADC_INFO_ADDR + sizeof(adcInfoTotal);
		//�������ݵĿ�ʼ��ַ
		adcInfoTotal.addrOfNewData = ADC_DATA_ADDR;
	}
	DEBUG_PRINTF("%s: total=%d, addrOfNewInfo=0x%x, addrOfNewData=0x%x\r\n",__func__,
	               adcInfoTotal.totalAdcInfo,adcInfoTotal.addrOfNewInfo,adcInfoTotal.addrOfNewData);
	
}


/*******************************************************************************
* ������  : Flash_WriteAdcData
* ����    : ��adc����д��оƬ�Դ�flash, �������fft����
* ����    : 
* ����ֵ  :
*******************************************************************************/
void Flash_WriteAdcData(uint8_t *buff, int len)
{
	memory_erase(FFT_ADC_ADDR, len);
	memory_write(FFT_ADC_ADDR,(uint8_t *)buff, len*4);
}


/*******************************************************************************
* ������  : LPC55S69_FlashSaveData
* ����    : ��ָ����ַ��ʼд��ָ�����ȵ�����
* ����    : WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)  pBuffer:����ָ��  NumToWrite:
* ����ֵ  :
*******************************************************************************/
void LPC55S69_FlashSaveData(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite)
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
		memory_erase(secpos*PAGE_SIZE, PAGE_SIZE);
        for(i=0;i<secremain;i++)	                    //����
        {
            FLEXSPI_BUF[i+secoff]=pBuffer[i];	  
        }
		memory_write(secpos*PAGE_SIZE, FLEXSPI_BUF, PAGE_SIZE);
		if(NumByteToWrite==secremain)
			break;//д�������
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		   	pBuffer+=secremain;  //ָ��ƫ��
			WriteAddr+=secremain;//д��ַƫ��	   
		   	NumByteToWrite-=secremain;				//�ֽ����ݼ�
			if(NumByteToWrite>PAGE_SIZE)
				secremain=PAGE_SIZE;	           //��һҳ����д����
			else 
				secremain=NumByteToWrite;			//��һҳ����д����
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
	char tempTime[20] = {0};
	//ǰ12�ֽڱ������ adcInfoTotal �ṹ��
	SPI_Flash_Read((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_ADDR, sizeof(adcInfoTotal));
	
	//�ж�Ϊ�״��ϵ�����
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > ADC_MAX_NUM){
		DEBUG_PRINTF("%s: init adcInfoTotal\r\n",__func__);
		//����Ϊ��ʼ��Ϊ0
		adcInfoTotal.totalAdcInfo = 0;
		//���� AdcInfo �ṹ�屣���ַ
		adcInfoTotal.addrOfNewInfo = ADC_INFO_ADDR + sizeof(adcInfoTotal);
		//�������ݵĿ�ʼ��ַ
		adcInfoTotal.addrOfNewData = ADC_DATA_ADDR;
	}
	
	DEBUG_PRINTF("%s: total=%d, addrOfNewInfo=0x%x, addrOfNewData=0x%x\r\n",__func__,
	               adcInfoTotal.totalAdcInfo,adcInfoTotal.addrOfNewInfo,adcInfoTotal.addrOfNewData);
	
	//��ʼ�� adcInfo �ṹ�� ����ʱ��
	sprintf(tempTime, "%d%02d%02d%02d%02d%02d", 
		                       sampTime.year%100, sampTime.month, sampTime.day, 
	                           sampTime.hour, sampTime.minute, sampTime.second);
	memcpy(adcInfo.AdcDataTime, tempTime, sizeof(adcInfo.AdcDataTime));
	
	//��ʼ�� adcInfo �ṹ�� ���ݳ���
	adcInfo.AdcDataLen = sizeof(SysSamplePara) + g_sample_para.shkCount*4 + spd_msg->len*4;
	//��ʼ�� adcInfo �ṹ�� ���ݵ�ַ
	adcInfo.AdcDataAddr = adcInfoTotal.addrOfNewData;
	if((adcInfo.AdcDataAddr % 4) != 0){//�жϵ�ַ�Ƿ����ֽڶ���
		adcInfo.AdcDataAddr = adcInfo.AdcDataAddr + (4 - (adcInfo.AdcDataAddr % 4));
	}
	if((adcInfo.AdcDataAddr + adcInfo.AdcDataLen) > SPI_FLASH_SIZE_BYTE){//�жϵ�ַ�Ƿ񳬹�flash��Χ
		adcInfo.AdcDataAddr = ADC_DATA_ADDR;
	}
	
	DEBUG_PRINTF("%s: AdcDataLen=%d, \r\n",__func__,adcInfo.AdcDataLen);
	
	//���� adcInfo �ṹ��
	SPI_Flash_Write((uint8_t *)&adcInfo.AdcDataAddr, adcInfoTotal.addrOfNewInfo, sizeof(adcInfo));
	SPI_Flash_Read((uint8_t *)&adcInfo.AdcDataAddr, adcInfoTotal.addrOfNewInfo, sizeof(adcInfo));
	
	//���� SysSamplePara �ṹ��
	SPI_Flash_Write((uint8_t *)&g_sample_para.DetectType, adcInfo.AdcDataAddr, sizeof(SysSamplePara));
	SPI_Flash_Read((uint8_t *)&adcInfo.AdcDataAddr, adcInfoTotal.addrOfNewInfo, sizeof(adcInfo));
	
	SPI_Flash_Read((uint8_t *)&g_sample_para.DetectType, adcInfo.AdcDataAddr, sizeof(SysSamplePara));
	SPI_Flash_Read((uint8_t *)&adcInfo.AdcDataAddr, adcInfoTotal.addrOfNewInfo, sizeof(adcInfo));
	
	//���� ������
	SPI_Flash_Write((uint8_t *)&ShakeADC[0], adcInfo.AdcDataAddr+sizeof(SysSamplePara), g_sample_para.shkCount*4);
	SPI_Flash_Read((uint8_t *)&ShakeADC[0], adcInfo.AdcDataAddr+sizeof(SysSamplePara), g_sample_para.shkCount*4);

	//���� ת������
	SPI_Flash_Write((uint8_t *)&spd_msg->spdData[0], adcInfo.AdcDataAddr+sizeof(SysSamplePara)+g_sample_para.shkCount*4, spd_msg->len*4);
	
	//���� adcInfoTotal �ṹ���е��ܲ�������
	adcInfoTotal.totalAdcInfo++;//���øú���,��ʾ��Ҫ����һ��adc��������
	if(adcInfoTotal.totalAdcInfo > ADC_MAX_NUM){//�жϳ��Ѵﵽ���ֵ
		adcInfoTotal.totalAdcInfo = ADC_MAX_NUM;
	}
	//���� adcInfoTotal �ṹ���е��´β�����Ϣ�����ַ
	adcInfoTotal.addrOfNewInfo = adcInfoTotal.addrOfNewInfo + sizeof(adcInfoTotal);
	if(adcInfoTotal.addrOfNewInfo >= SPI_FLASH_SIZE_BYTE){
		adcInfoTotal.addrOfNewInfo = ADC_INFO_ADDR + sizeof(AdcInfoTotal);
	}
	//���� adcInfoTotal �ṹ���е��´β������ݱ����ַ
	adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + adcInfo.AdcDataLen;
	if( adcInfoTotal.addrOfNewData % 4 != 0){//�ж��µ�ַ����4�ֽڶ����, ��Ҫ����4�ֽڶ���
		adcInfoTotal.addrOfNewData = adcInfoTotal.addrOfNewData + (4-adcInfoTotal.addrOfNewData%4);
	}
	if (adcInfoTotal.addrOfNewData > SPI_FLASH_SIZE_BYTE){//�жϳ��µ�ַ����flash��Χ,���ͷ��ʼ��¼
		adcInfoTotal.addrOfNewData = ADC_DATA_ADDR;
		adcInfoTotal.freeOfKb = 0;
	}else{
		adcInfoTotal.freeOfKb = (SPI_FLASH_SIZE_BYTE - adcInfoTotal.addrOfNewData)/1024;
	}
	
	//���� adcInfoTotal �ṹ��
	SPI_Flash_Write((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_ADDR, sizeof(AdcInfoTotal));
}

/***************************************************************************************
  * @brief   ͨ��ʱ���ȡһ��adc����
  * @input   
  * @return  �������ݵ�ַ
***************************************************************************************/
char W25Q128_ReadAdcData(char *adcDataTime)
{
	uint32_t tempAddr;
	uint8_t  ret = false;
	
	//ǰ12�ֽڱ������ adcInfoTotal �ṹ��
	SPI_Flash_Read((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_ADDR, sizeof(adcInfoTotal));
	
	//flash�л�δ����������,ֱ�ӷ���
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > ADC_MAX_NUM){
		return ret;
	}
	
	for(uint32_t i = 0; i<adcInfoTotal.totalAdcInfo; i++){
		//��ǰ�����ļ�
		tempAddr = adcInfoTotal.addrOfNewInfo - (i+1)*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_ADDR + sizeof(AdcInfoTotal))){
			tempAddr = ADC_DATA_ADDR - sizeof(adcInfo);
		}
		//��ȡ���ݵ� adcInfo �ṹ��
		SPI_Flash_Read((uint8_t *)&adcInfo.AdcDataAddr, tempAddr, sizeof(adcInfo));
		
		//�ҵ��ļ�
		if( memcmp(adcDataTime, adcInfo.AdcDataTime, sizeof(adcInfo.AdcDataTime)) == 0){
			ret = true;
			break;
		}
	}
	if ( ret == true){
		//��ȡ SysSamplePara �ṹ������
		SPI_Flash_Read((uint8_t *)&g_sample_para.DetectType, adcInfo.AdcDataAddr, sizeof(SysSamplePara));
		
		//��ȡ ������
		SPI_Flash_Read((uint8_t *)ShakeADC, adcInfo.AdcDataAddr+sizeof(SysSamplePara), g_sample_para.shkCount);
		
		//��ȡ ת������
		if (spd_msg->len != 0 && spd_msg!= NULL){
			SPI_Flash_Read((uint8_t *)spd_msg->spdData, adcInfo.AdcDataAddr+sizeof(SysSamplePara)+g_sample_para.shkCount, spd_msg->len);
		}
	}
	return ret;
}

/***************************************************************************************
  * @brief   ��ȡ����������ݲɼ����ļ���
  * @input   
  * @return  �������ݵ�ַ
***************************************************************************************/
void W25Q128_ReadAdcInfo(int si, int num, char *buf)
{
	uint32_t tempAddr,ei;
	
	//ǰ12�ֽڱ������ adcInfoTotal �ṹ��
	SPI_Flash_Read((uint8_t *)&adcInfoTotal.totalAdcInfo, ADC_INFO_ADDR, sizeof(adcInfoTotal));
	
	//flash�л�δ����������,ֱ�ӷ���
	if(adcInfoTotal.totalAdcInfo == 0xFFFFFFFF || adcInfoTotal.totalAdcInfo > ADC_MAX_NUM){
		adcInfoTotal.totalAdcInfo = 0;
		adcInfoTotal.freeOfKb = (SPI_FLASH_SIZE_BYTE - adcInfoTotal.addrOfNewData)/1024;
		return;
	}
	
	if(si > ADC_MAX_NUM)si= ADC_MAX_NUM;
	if(si <= 0 ) si = 1;
	
	ei = num+si;
	if(ei > adcInfoTotal.totalAdcInfo+1) ei=adcInfoTotal.totalAdcInfo+1;
	
	for(uint32_t i = si; i<ei; i++){
		//��ǰ�����ļ�
		tempAddr = adcInfoTotal.addrOfNewInfo - i*sizeof(AdcInfo);
		if(tempAddr < (ADC_INFO_ADDR + sizeof(AdcInfoTotal))){
			tempAddr = ADC_DATA_ADDR - sizeof(adcInfo);
		}

		//��ȡ���ݵ� adcInfo �ṹ��
		SPI_Flash_Read((uint8_t *)&adcInfo.AdcDataAddr, tempAddr, sizeof(adcInfo));
		strncat(buf, adcInfo.AdcDataTime, 12);
		strcat(buf, ",");//��ӷָ���
	}
	if(strlen(buf) > 0){//ȥ�����һ���ָ���
		buf[strlen(buf)-1] = 0x00;
	}
}

