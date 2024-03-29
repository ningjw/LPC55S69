/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	ota.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2019-07-31
	*
	*	版本： 		V1.0
	*
	*	说明： 		OTA
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//OTA
#include "ota.h"

//算法
#include "base64.h"
#include "hmac_sha1.h"
#include "md5.h"

//驱动
#include "delay.h"
#include "usart.h"
#include "stmflash.h"

//网络设备
#include "net_device.h"

//C库
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
*	函数名称：	OTA_Jump
*
*	函数功能：	跳转到应用程序段
*
*	入口参数：	appxaddr:用户代码起始地址
*
*	返回参数：	0-有		1-无
*
*	说明：		
************************************************************
*/
void OTA_Jump(unsigned int addr)
{

	if(((*(volatile unsigned int *)(addr + 4)) & 0xFF000000) == 0x08000000)	//判断是否为0X08XXXXXX
	{
		if(((*(volatile unsigned int *)addr) & 0x2FFE0000) == 0x20000000)	//检查栈顶地址是否合法
		{
			jump2app = (iapfun)*(volatile unsigned int *)(addr + 4);		//用户代码区第二个字为程序开始地址(复位地址)
			
			MSR_MSP(*(volatile unsigned int *)addr);						//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
			
			jump2app();
		}
	}

}

/*
************************************************************
*	函数名称：	OTA_UrlEncode
*
*	函数功能：	sign需要进行URL编码
*
*	入口参数：	sign：加密结果
*
*	返回参数：	0-成功	其他-失败
*
*	说明：		+			%2B
*				空格		%20
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
*	函数名称：	OTA_Authorization
*
*	函数功能：	计算Authorization
*
*	入口参数：	ver：参数组版本号，日期格式，目前仅支持格式"2018-10-31"
*				res：产品id
*				et：过期时间，UTC秒值
*				access_key：访问密钥
*				authorization_buf：缓存token的指针
*				authorization_buf_len：缓存区长度(字节)
*
*	返回参数：	0-成功	其他-失败
*
*	说明：		当前仅支持sha1
************************************************************
*/
#define METHOD		"sha1"
unsigned char OTA_Authorization(char *ver, char *res, unsigned int et, char *access_key, char *authorization_buf, unsigned short authorization_buf_len)
{
	
	size_t olen = 0;
	
	char sign_buf[40];							//保存签名的Base64编码结果 和 URL编码结果
	char hmac_sha1_buf[40];						//保存签名
	char access_key_base64[40];					//保存access_key的Base64编码结合
	char string_for_signature[56];				//保存string_for_signature，这个是加密的key

//----------------------------------------------------参数合法性--------------------------------------------------------------------
	if(ver == (void *)0 || res == (void *)0 || et < 1564562581 || access_key == (void *)0
		|| authorization_buf == (void *)0 || authorization_buf_len < 120)
		return 1;
	
//----------------------------------------------------将access_key进行Base64解码----------------------------------------------------
	memset(access_key_base64, 0, sizeof(access_key_base64));
	BASE64_Decode((unsigned char *)access_key_base64, sizeof(access_key_base64), &olen, (unsigned char *)access_key, strlen(access_key));
	//UsartPrintf(USART_DEBUG, "access_key_base64: %s\r\n", access_key_base64);
	
//----------------------------------------------------计算string_for_signature-----------------------------------------------------
	memset(string_for_signature, 0, sizeof(string_for_signature));
	snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s\n%s", et, METHOD, res, ver);
	//UsartPrintf(USART_DEBUG, "string_for_signature: %s\r\n", string_for_signature);
	
//----------------------------------------------------加密-------------------------------------------------------------------------
	memset(hmac_sha1_buf, 0, sizeof(hmac_sha1_buf));
	
	hmac_sha1((unsigned char *)access_key_base64, strlen(access_key_base64),
				(unsigned char *)string_for_signature, strlen(string_for_signature),
				(unsigned char *)hmac_sha1_buf);
	
	//UsartPrintf(USART_DEBUG, "hmac_sha1_buf: %s\r\n", hmac_sha1_buf);
	
//----------------------------------------------------将加密结果进行Base64编码------------------------------------------------------
	olen = 0;
	memset(sign_buf, 0, sizeof(sign_buf));
	BASE64_Encode((unsigned char *)sign_buf, sizeof(sign_buf), &olen, (unsigned char *)hmac_sha1_buf, strlen(hmac_sha1_buf));

//----------------------------------------------------将Base64编码结果进行URL编码---------------------------------------------------
	OTA_UrlEncode(sign_buf);
	//UsartPrintf(USART_DEBUG, "sign_buf: %s\r\n", sign_buf);
	
//----------------------------------------------------计算Token--------------------------------------------------------------------
	snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s&et=%d&method=%s&sign=%s", ver, res, et, METHOD, sign_buf);
	//UsartPrintf(USART_DEBUG, "Token: %s\r\n", token_buf);
	
	return 0;

}

/*
************************************************************
*	函数名称：	OTA_Check
*
*	函数功能：	检测是否需要升级
*
*	入口参数：	dev_id：设备ID
*				authorization：
*
*	返回参数：	0-成功	其他-失败
*
*	说明：		
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
*	函数名称：	OTA_CheckToken
*
*	函数功能：	校验Token
*
*	入口参数：	dev_id：设备ID
*				token：平台返回的Token
*				authorization：
*
*	返回参数：	0-成功	其他-失败
*
*	说明：		
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
*	函数名称：	OTA_Download_Range
*
*	函数功能：	分片下载固件
*
*	入口参数：	token：平台返回的Token
*				md5：平台返回的MD5
*				size：平台返回的固件大小(字节)
*				bytes_range：分片大小(字节)
*
*	返回参数：	0-成功	其他-失败
*
*	说明：		
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
	
//----------------------------------------------------MD5初始化、擦除闪存-----------------------------------------------------------
	MD5_Init(&md5_ctx);
	
	packet_num = size / FLASH_PAGE_SIZE + 1;
	FLASH_Erase(write_page_start, packet_num, FLASH_PAGE_SIZE == 2048 ? 1 : 0);
	
	while(bytes < size)
	{
//----------------------------------------------------发送报文下载一包数据----------------------------------------------------------
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
//----------------------------------------------------等待数据---------------------------------------------------------------------
			time_out = 200;
			while(--time_out)
			{
				if(net_device_info.cmd_ipd != NULL)
					break;
				
				RTOS_TimeDly(2);
			}
			
//----------------------------------------------------跳过HTTP报文头、找到固件数据--------------------------------------------------
			if(time_out)
			{
				data_ptr = strstr(net_device_info.cmd_ipd, "Range");
				data_ptr = strstr(data_ptr, "\r\n");
				data_ptr += 4;
			}
			
//----------------------------------------------------将固件数据写入缓存和闪存-----------------------------------------------------
			if(data_ptr != NULL)
			{
				if(bytes < size - bytes_range)																	//判断是否为最后一包。满足则不是
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
				if((bytes % FLASH_PAGE_SIZE) == 0)																//缓存是否保存了2KB的数据了
				{
					Flash_Write(write_page_cnt, (unsigned short *)flash_buf, FLASH_PAGE_SIZE >> 1, 1);
					write_page_cnt++;
				}
				
				if(bytes >= size)																				//最后一包数据
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
		
		RTOS_TimeDly(net_device_info.send_time + 60);															//为了通信的稳定
	}
	
//----------------------------------------------------MD校验比对------------------------------------------------------------------
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
	
	if(strcmp(md5_result, md5) == 0)																		//MD5校验对比
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
*	函数名称：	OTA_Report
*
*	函数功能：	升级状态上报
*
*	入口参数：	dev_id：设备ID
*				token：平台返回的Token
*				authorization：平台返回的固件大小(字节)
*				status：升级状态
*
*	返回参数：	0-成功	其他-失败
*
*	说明：		101---下载成功-升级包下载成功
*				102---下载失败-空间不足
*				103---下载失败-内存溢出
*				104---下载失败-下载请求超时
*				105---下载失败-电量不足
*				106---下载失败-信号不良
*				107---下载失败-未知异常
*
*				201---升级成功-升级成功
*				202---升级失败-电量不足
*				203---升级失败-内存溢出
*				204---升级失败-版本不一致
*				205---升级失败-MD5校验失败
*				206---升级失败-未知异常
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
*	函数名称：	OTA_Clear
*
*	函数功能：	清零化相关变量
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
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
