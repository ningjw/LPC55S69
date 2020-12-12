/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	ota.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2019-07-31
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		OTA
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//OTA
#include "ota.h"

//�㷨
#include "base64.h"
#include "hmac_sha1.h"
#include "md5.h"

//����
#include "delay.h"
#include "usart.h"
#include "stmflash.h"

//�����豸
#include "net_device.h"

//C��
#include <stdio.h>
#include <string.h>


#define OTA_IP		"183.230.40.50"

#define OTA_PORT	"80"


OTA_INFO ota_info;


static iapfun jump2app;


__asm void MSR_MSP(unsigned int addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}

/*
************************************************************
*	�������ƣ�	OTA_Jump
*
*	�������ܣ�	��ת��Ӧ�ó����
*
*	��ڲ�����	appxaddr:�û�������ʼ��ַ
*
*	���ز�����	0-��		1-��
*
*	˵����		
************************************************************
*/
void OTA_Jump(unsigned int addr)
{

	if(((*(volatile unsigned int *)(addr + 4)) & 0xFF000000) == 0x08000000)	//�ж��Ƿ�Ϊ0X08XXXXXX
	{
		if(((*(volatile unsigned int *)addr) & 0x2FFE0000) == 0x20000000)	//���ջ����ַ�Ƿ�Ϸ�
		{
			jump2app = (iapfun)*(volatile unsigned int *)(addr + 4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)
			
			MSR_MSP(*(volatile unsigned int *)addr);						//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
			
			jump2app();
		}
	}

}

/*
************************************************************
*	�������ƣ�	OTA_UrlEncode
*
*	�������ܣ�	sign��Ҫ����URL����
*
*	��ڲ�����	sign�����ܽ��
*
*	���ز�����	0-�ɹ�	����-ʧ��
*
*	˵����		+			%2B
*				�ո�		%20
*				/			%2F
*				?			%3F
*				%			%25
*				#			%23
*				&			%26
*				=			%3D
************************************************************
*/
static unsigned char OTA_UrlEncode(char *sign)
{

	char sign_t[40];
	unsigned char i = 0, j = 0;
	unsigned char sign_len = strlen(sign);
	
	if(sign == (void *)0 || sign_len < 28)
		return 1;
	
	for(; i < sign_len; i++)
	{
		sign_t[i] = sign[i];
		sign[i] = 0;
	}
	sign_t[i] = 0;
	
	for(i = 0, j = 0; i < sign_len; i++)
	{
		switch(sign_t[i])
		{
			case '+':
				strcat(sign + j, "%2B");j += 3;
			break;
			
			case ' ':
				strcat(sign + j, "%20");j += 3;
			break;
			
			case '/':
				strcat(sign + j, "%2F");j += 3;
			break;
			
			case '?':
				strcat(sign + j, "%3F");j += 3;
			break;
			
			case '%':
				strcat(sign + j, "%25");j += 3;
			break;
			
			case '#':
				strcat(sign + j, "%23");j += 3;
			break;
			
			case '&':
				strcat(sign + j, "%26");j += 3;
			break;
			
			case '=':
				strcat(sign + j, "%3D");j += 3;
			break;
			
			default:
				sign[j] = sign_t[i];j++;
			break;
		}
	}
	
	sign[j] = 0;
	
	return 0;

}

/*
************************************************************
*	�������ƣ�	OTA_Authorization
*
*	�������ܣ�	����Authorization
*
*	��ڲ�����	ver��������汾�ţ����ڸ�ʽ��Ŀǰ��֧�ָ�ʽ"2018-10-31"
*				res����Ʒid
*				et������ʱ�䣬UTC��ֵ
*				access_key��������Կ
*				authorization_buf������token��ָ��
*				authorization_buf_len������������(�ֽ�)
*
*	���ز�����	0-�ɹ�	����-ʧ��
*
*	˵����		��ǰ��֧��sha1
************************************************************
*/
#define METHOD		"sha1"
unsigned char OTA_Authorization(char *ver, char *res, unsigned int et, char *access_key, char *authorization_buf, unsigned short authorization_buf_len)
{
	
	size_t olen = 0;
	
	char sign_buf[40];							//����ǩ����Base64������ �� URL������
	char hmac_sha1_buf[40];						//����ǩ��
	char access_key_base64[40];					//����access_key��Base64������
	char string_for_signature[56];				//����string_for_signature������Ǽ��ܵ�key

//----------------------------------------------------�����Ϸ���--------------------------------------------------------------------
	if(ver == (void *)0 || res == (void *)0 || et < 1564562581 || access_key == (void *)0
		|| authorization_buf == (void *)0 || authorization_buf_len < 120)
		return 1;
	
//----------------------------------------------------��access_key����Base64����----------------------------------------------------
	memset(access_key_base64, 0, sizeof(access_key_base64));
	BASE64_Decode((unsigned char *)access_key_base64, sizeof(access_key_base64), &olen, (unsigned char *)access_key, strlen(access_key));
	//UsartPrintf(USART_DEBUG, "access_key_base64: %s\r\n", access_key_base64);
	
//----------------------------------------------------����string_for_signature-----------------------------------------------------
	memset(string_for_signature, 0, sizeof(string_for_signature));
	snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s\n%s", et, METHOD, res, ver);
	//UsartPrintf(USART_DEBUG, "string_for_signature: %s\r\n", string_for_signature);
	
//----------------------------------------------------����-------------------------------------------------------------------------
	memset(hmac_sha1_buf, 0, sizeof(hmac_sha1_buf));
	
	hmac_sha1((unsigned char *)access_key_base64, strlen(access_key_base64),
				(unsigned char *)string_for_signature, strlen(string_for_signature),
				(unsigned char *)hmac_sha1_buf);
	
	//UsartPrintf(USART_DEBUG, "hmac_sha1_buf: %s\r\n", hmac_sha1_buf);
	
//----------------------------------------------------�����ܽ������Base64����------------------------------------------------------
	olen = 0;
	memset(sign_buf, 0, sizeof(sign_buf));
	BASE64_Encode((unsigned char *)sign_buf, sizeof(sign_buf), &olen, (unsigned char *)hmac_sha1_buf, strlen(hmac_sha1_buf));

//----------------------------------------------------��Base64����������URL����---------------------------------------------------
	OTA_UrlEncode(sign_buf);
	//UsartPrintf(USART_DEBUG, "sign_buf: %s\r\n", sign_buf);
	
//----------------------------------------------------����Token--------------------------------------------------------------------
	snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s&et=%d&method=%s&sign=%s", ver, res, et, METHOD, sign_buf);
	//UsartPrintf(USART_DEBUG, "Token: %s\r\n", token_buf);
	
	return 0;

}

/*
************************************************************
*	�������ƣ�	OTA_Check
*
*	�������ܣ�	����Ƿ���Ҫ����
*
*	��ڲ�����	dev_id���豸ID
*				authorization��
*
*	���ز�����	0-�ɹ�	����-ʧ��
*
*	˵����		
************************************************************
*/
unsigned char OTA_Check(char *dev_id, char *authorization)
{
	
	unsigned char result = 255;
	char *data_ptr = NULL;
	char send_buf[280];
	unsigned char time_out = 200;

	if(!net_device_info.net_work)
		return result;
	
	NET_DEVICE_Close();
	
	if(NET_DEVICE_Connect("TCP", OTA_IP, OTA_PORT) == 0)
	{
		memset(send_buf, 0, sizeof(send_buf));
		snprintf(send_buf, sizeof(send_buf), "GET /ota/south/check?"
												"dev_id=%s&manuf=100&model=10001&type=2&version=1.0&cdn=false HTTP/1.1\r\n"
												"Authorization:%s\r\n"
												"Host:ota.heclouds.com\r\n\r\n",
												dev_id, authorization);
		
		net_device_info.cmd_ipd = NULL;
		
		NET_DEVICE_SendData((unsigned char *)send_buf, strlen(send_buf));
		
		while(--time_out)
		{
			if(net_device_info.cmd_ipd != NULL)
				break;
			
			RTOS_TimeDly(2);
		}
		
		if(time_out)
		{
			data_ptr = strstr(net_device_info.cmd_ipd, "target");
		}
		
		if(data_ptr != NULL)
		{
			//errno":0,"error":"succ","data":{"target":"1.0","token":"ota_e8J7tlNueb7RICV8p5Ij","size":3868,
			//"md5":"2fcaf36d96f1bdbc849eb7c034957cf3","signal":10,"power":50,"retry":0,"interval":0,"type":1}}
			
			UsartPrintf(USART_DEBUG, "Tips:	OTA_Check Ok\r\n");
			
			if(sscanf(data_ptr, "target\":\"%[^\"]\",\"token\":\"%[^\"]\",\"size\":%d,\"md5\":\"%[^\"]\",",
									ota_info.version, ota_info.token, &ota_info.size, ota_info.md5) == 4)
			{
				UsartPrintf(USART_DEBUG, "ver: %s, token: %s, size: %d, md5:%s\r\n",
											ota_info.version, ota_info.token, ota_info.size, ota_info.md5);
				
				result = 0;
			}
		}
		else
			UsartPrintf(USART_DEBUG, "Tips:	OTA_Check Err\r\n");
	}
	
	if(result)
		NET_DEVICE_Close();
	
	return result;

}

/*
************************************************************
*	�������ƣ�	OTA_CheckToken
*
*	�������ܣ�	У��Token
*
*	��ڲ�����	dev_id���豸ID
*				token��ƽ̨���ص�Token
*				authorization��
*
*	���ز�����	0-�ɹ�	����-ʧ��
*
*	˵����		
************************************************************
*/
unsigned char OTA_CheckToken(char *dev_id, char *token, char *authorization)
{

	unsigned char result = 255;
	char *data_ptr = NULL;
	char send_buf[256];
	unsigned char time_out = 200;

	if(!net_device_info.net_work)
		return result;
	
	memset(send_buf, 0, sizeof(send_buf));
	snprintf(send_buf, sizeof(send_buf), "GET /ota/south/download/"
											"%s/check?dev_id=%s HTTP/1.1\r\n"
											"Authorization:%s\r\n"
											"Host:ota.heclouds.com\r\n\r\n",
											token, dev_id, authorization);
	
	net_device_info.cmd_ipd = NULL;
	
	NET_DEVICE_SendData((unsigned char *)send_buf, strlen(send_buf));
	
	while(--time_out)
	{
		if(net_device_info.cmd_ipd != NULL)
			break;
		
		RTOS_TimeDly(2);
	}
	
	if(time_out)
	{
		data_ptr = strstr(net_device_info.cmd_ipd, "succ");
	}
	
	if(data_ptr != NULL)
	{
		UsartPrintf(USART_DEBUG, "Tips:	OTA_CheckToken Ok\r\n");
		
		result = 0;
	}
	else
		UsartPrintf(USART_DEBUG, "Tips:	OTA_CheckToken Err\r\n");
	
	if(result)
		NET_DEVICE_Close();
	
	return result;

}

/*
************************************************************
*	�������ƣ�	OTA_Download_Range
*
*	�������ܣ�	��Ƭ���ع̼�
*
*	��ڲ�����	token��ƽ̨���ص�Token
*				md5��ƽ̨���ص�MD5
*				size��ƽ̨���صĹ̼���С(�ֽ�)
*				bytes_range����Ƭ��С(�ֽ�)
*
*	���ز�����	0-�ɹ�	����-ʧ��
*
*	˵����		
************************************************************
*/
#define FLASH_PAGE_SIZE		2048
unsigned char OTA_Download_Range(char *token, char *md5, unsigned int size, const unsigned short bytes_range)
{
	
	MD5_CTX md5_ctx;
	unsigned char i = 0, md5_t[16];
	char md5_result[40];
	char md5_t1[4] = {0, 0, 0, 0};

	unsigned char result = 255;
	char send_buf[256];
	char *data_ptr = NULL;
	unsigned char time_out = 200;
	unsigned int bytes = 0;
	unsigned char err_cnt = 0;
	
	unsigned char *flash_buf = (void *)0;
	unsigned int packet_num = 0;
	unsigned int write_page_start = 50;
	unsigned int write_page_cnt = write_page_start;
	
	flash_buf = NET_MallocBuffer(FLASH_PAGE_SIZE);
	if(!flash_buf)
	{
		UsartPrintf(USART_DEBUG, "WARN:	flash_buf malloc failed\r\n");
		
		return result;
	}
	
//----------------------------------------------------MD5��ʼ������������-----------------------------------------------------------
	MD5_Init(&md5_ctx);
	
	packet_num = size / FLASH_PAGE_SIZE + 1;
	FLASH_Erase(write_page_start, packet_num, FLASH_PAGE_SIZE == 2048 ? 1 : 0);
	
	while(bytes < size)
	{
//----------------------------------------------------���ͱ�������һ������----------------------------------------------------------
		memset(send_buf, 0, sizeof(send_buf));
		snprintf(send_buf, sizeof(send_buf), "GET /ota/south/download/"
												"%s HTTP/1.1\r\n"
												"Range:bytes=%d-%d\r\n"
												"Host:ota.heclouds.com\r\n\r\n",
												token, bytes, bytes + bytes_range - 1);
		
		net_device_info.cmd_ipd = NULL;
		
		if(NET_DEVICE_SendData((unsigned char *)send_buf, strlen(send_buf)) == 0)
		{
			err_cnt = 0;
//----------------------------------------------------�ȴ�����---------------------------------------------------------------------
			time_out = 200;
			while(--time_out)
			{
				if(net_device_info.cmd_ipd != NULL)
					break;
				
				RTOS_TimeDly(2);
			}
			
//----------------------------------------------------����HTTP����ͷ���ҵ��̼�����--------------------------------------------------
			if(time_out)
			{
				data_ptr = strstr(net_device_info.cmd_ipd, "Range");
				data_ptr = strstr(data_ptr, "\r\n");
				data_ptr += 4;
			}
			
//----------------------------------------------------���̼�����д�뻺�������-----------------------------------------------------
			if(data_ptr != NULL)
			{
				if(bytes < size - bytes_range)																	//�ж��Ƿ�Ϊ���һ������������
				{
					memcpy(flash_buf + (bytes % FLASH_PAGE_SIZE), data_ptr, bytes_range);
					MD5_Update(&md5_ctx, (unsigned char *)data_ptr, bytes_range);
				}
				else
				{
					memcpy(flash_buf + (bytes % FLASH_PAGE_SIZE), data_ptr, size - bytes);
					MD5_Update(&md5_ctx, (unsigned char *)data_ptr, size - bytes);
				}
				
				RTOS_ENTER_CRITICAL();
				
				bytes += bytes_range;
				if((bytes % FLASH_PAGE_SIZE) == 0)																//�����Ƿ񱣴���2KB��������
				{
					Flash_Write(write_page_cnt, (unsigned short *)flash_buf, FLASH_PAGE_SIZE >> 1, 1);
					write_page_cnt++;
				}
				
				if(bytes >= size)																				//���һ������
					Flash_Write(write_page_cnt, (unsigned short *)flash_buf, (size % FLASH_PAGE_SIZE) >> 1, 1);
				
				RTOS_EXIT_CRITICAL();
				
				if(bytes >= size)
					UsartPrintf(USART_DEBUG, "Update %d / %d   Bytes, 100%%\r\n", size, size);
				else
					UsartPrintf(USART_DEBUG, "Update %d / %d   Bytes, %0.2f%%\r\n", bytes, size, (float)bytes / size * 100);
			}
		}
		else
		{
			if(++err_cnt >= 3)
			{
				err_cnt = 0;
				
				NET_DEVICE_Close();
				
				while(NET_DEVICE_Connect("TCP", OTA_IP, OTA_PORT))
					RTOS_TimeDly(400);
			}
		}
		
		RTOS_TimeDly(net_device_info.send_time + 60);															//Ϊ��ͨ�ŵ��ȶ�
	}
	
//----------------------------------------------------MDУ��ȶ�------------------------------------------------------------------
	memset(md5_result, 0, sizeof(md5_result));
	MD5_Final(&md5_ctx, md5_t);
	for(i = 0; i < 16; i++)
	{
		if(md5_t[i] <= 0x0f)
			sprintf(md5_t1, "0%x", md5_t[i]);
		else
			sprintf(md5_t1, "%x", md5_t[i]);
		
		strcat(md5_result, md5_t1);
	}
	
	UsartPrintf(USART_DEBUG, "Tips:	MD5: %s\r\n", md5_result);
	
	if(strcmp(md5_result, md5) == 0)																		//MD5У��Ա�
	{
		UsartPrintf(USART_DEBUG, "Tips:	MD5 Successful Matches\r\n");
		
		//OTA_Jump(FLASH_GetStartAddr(write_page_start, FLASH_PAGE_SIZE == 2048 ? 1 : 0));
		
		ota_info.ota_download_ok = 1;
		ota_info.addr = FLASH_GetStartAddr(write_page_start, FLASH_PAGE_SIZE == 2048 ? 1 : 0);
		result = 0;
	}
	
	NET_FreeBuffer(flash_buf);
	
	return result;

}

/*
************************************************************
*	�������ƣ�	OTA_Report
*
*	�������ܣ�	����״̬�ϱ�
*
*	��ڲ�����	dev_id���豸ID
*				token��ƽ̨���ص�Token
*				authorization��ƽ̨���صĹ̼���С(�ֽ�)
*				status������״̬
*
*	���ز�����	0-�ɹ�	����-ʧ��
*
*	˵����		101---���سɹ�-���������سɹ�
*				102---����ʧ��-�ռ䲻��
*				103---����ʧ��-�ڴ����
*				104---����ʧ��-��������ʱ
*				105---����ʧ��-��������
*				106---����ʧ��-�źŲ���
*				107---����ʧ��-δ֪�쳣
*
*				201---�����ɹ�-�����ɹ�
*				202---����ʧ��-��������
*				203---����ʧ��-�ڴ����
*				204---����ʧ��-�汾��һ��
*				205---����ʧ��-MD5У��ʧ��
*				206---����ʧ��-δ֪�쳣
************************************************************
*/
unsigned char OTA_Report(char *dev_id, char *token, char *authorization, int status)
{

	unsigned char result = 255;
	char *data_ptr = NULL;
	char send_buf[296];
	char status_str[4];
	unsigned char time_out = 200;

	if(!net_device_info.net_work)
		return result;
	
	snprintf(status_str, sizeof(status_str), "%d", status);
	memset(send_buf, 0, sizeof(send_buf));
	snprintf(send_buf, sizeof(send_buf), "POST /ota/south/report?dev_id=%s&token=%s HTTP/1.1\r\n"
											"Authorization:%s\r\n"
											"Host:ota.heclouds.com\r\n"
											"Content-Type:application/json\r\n"
											"Content-Length:%d\r\n\r\n"
											"{\"result\":%s}",
											dev_id, token, authorization, 14, status_str);
	
	net_device_info.cmd_ipd = NULL;
	
	NET_DEVICE_SendData((unsigned char *)send_buf, strlen(send_buf));
	
	while(--time_out)
	{
		if(net_device_info.cmd_ipd != NULL)
			break;
		
		RTOS_TimeDly(2);
	}
	
	if(time_out)
	{
		data_ptr = strstr(net_device_info.cmd_ipd, "succ");
	}
	
	if(data_ptr != NULL)
	{
		UsartPrintf(USART_DEBUG, "Tips:	OTA Report Status Ok\r\n");
			
		ota_info.ota_report_ok = 1;
		
		result = 0;
	}
	else
		UsartPrintf(USART_DEBUG, "Tips:	OTA Report Status Err\r\n");
	
	if(result)
		NET_DEVICE_Close();
	
	return result;

}

/*
************************************************************
*	�������ƣ�	OTA_Clear
*
*	�������ܣ�	���㻯��ر���
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void OTA_Clear(void)
{

	ota_info.addr = 0;
	ota_info.ota_check = 0;
	ota_info.ota_download_ok = 0;
	ota_info.ota_report_ok = 0;
	ota_info.ota_start = 0;
	ota_info.size = 0;
	
	memset(ota_info.version, 0, sizeof(ota_info.version));
	memset(ota_info.token, 0, sizeof(ota_info.token));
	memset(ota_info.md5, 0, sizeof(ota_info.md5));

}