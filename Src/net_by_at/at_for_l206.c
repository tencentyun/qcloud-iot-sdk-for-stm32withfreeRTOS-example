/*********************************************************************************
 *                   Copyright (c) 2018 - 2020,Tencent
 *                      All rights reserved.
 *
 * File Name:    at_for_l206.c
 *
 * Description:   net api based on at cmd for l206
 *
 * History:      <author>          <time>        <version>
 *                   yougaliu          2018-6-7        1.0
 * Desc:           ORG.
 ********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "at_driver.h"
#include "at_for_l206.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

#include "time.h"

/* External variables --------------------------------------------------------*/

void HexDump(uint8_t *pdata, int len)
{
	for(int i=0; i < len; i++)
	{
		if(0 == i%16)
		{
			printf("\n\r");
		}
		printf("%02X ", pdata[i]);
	}
}



/**
  * @brief 关闭回显
  */

HAL_StatusTypeDef atEchoOff(void)
{
	sAtCmd atCmd;	

	//关闭回显	
	init_send_cmd(&atCmd,  "ATE0\r\n", "OK", TIMOUT_3S);
	
	return exec_gsm_at(&atCmd);
}

/**
  * @brief 打开回显
  */

HAL_StatusTypeDef atEchoOn(void)
{
	sAtCmd atCmd;	

	//打开回显	
	init_send_cmd(&atCmd,  "ATE1\r\n", "OK", TIMOUT_1S);
	
	return exec_gsm_at(&atCmd);
}

/**
  * @brief AT 状态确认
  */

HAL_StatusTypeDef atAlive(void)
{
	sAtCmd atCmd;	

	init_send_cmd(&atCmd,  "AT\r\n", "OK", TIMOUT_1S);
	
	return exec_gsm_at(&atCmd);
}



int net_init_by_at(void)
{
	sAtCmd atCmd;	
	int Ret = HAL_ERROR;
	int count;

	HwInitL206();


	if(HAL_OK != atAlive())
	{
		AT_INFO_DEBUG("\n\rGsm module not ready");
		//goto end;
	}

	
	if(HAL_OK !=  atEchoOff())
	{
		AT_INFO_DEBUG("\n\ratEcho off fail");
	}
	
	init_send_cmd(&atCmd, "AT+CIPSHUT\r\n", "OK", TIMOUT_5S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		AT_INFO_DEBUG("\n\rmodule shut fail");
		goto end;
	}



	/*使能手动获取数据*/
	init_send_cmd(&atCmd, "AT+CIPRXGET=1\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		goto end;
	}

	
	init_send_cmd(&atCmd, "AT+CPIN?\r\n", "+CPIN: READY", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		goto end;
	}

	init_send_cmd(&atCmd, "AT+CSQ\r\n",  NULL, TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		goto end;
	}
	else
	{
		AT_INFO_DEBUG("\n\rGet CSQ:[%s]", atCmd.pAckBuf);
	}




	init_send_cmd(&atCmd, "AT+CREG?\r\n", "+CREG: 0,1", TIMOUT_2S);
	for(count = 0; count < (AT_CHECK_STAT_TIME + 1); count++)
	{
		if(HAL_OK ==  exec_gsm_at(&atCmd))
		{
			break;
		}
		else
		{
			if(AT_CHECK_STAT_TIME == count)
			{
				goto end;
			}
		}
	}



	init_send_cmd(&atCmd, "AT+CGATT?\r\n", "+CGATT: 1", TIMOUT_2S);

	for(count = 0; count < (AT_CHECK_STAT_TIME + 1); count++)
	{
		if(HAL_OK ==  exec_gsm_at(&atCmd))
		{
			break;
		}
		else
		{
			if(AT_CHECK_STAT_TIME == count)
			{
				goto end;
			}
		}
	}

	
	
	init_send_cmd(&atCmd, "AT+CIPMUX=0\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		goto end;
	}

	
//	init_send_cmd(&atCmd, "AT+CIPMODE=1\r\n", "OK", TIMOUT_1S);
//	if(HAL_OK !=  exec_gsm_at(&atCmd))
//	{
//		goto end;
//	}	

    init_send_cmd(&atCmd, "AT+CSTT=\"CMNET\"\r\n", "OK", TIMOUT_2S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		goto end;
	}


	init_send_cmd(&atCmd, "AT+CIICR\r\n", "OK", TIMOUT_5S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		goto end;
	}

	

	init_send_cmd(&atCmd, "AT+CIFSR\r\n", NULL, TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		goto end;
	}
	else
	{
		AT_INFO_DEBUG("\n\rGet LocalIp:[%s]", (char *)atCmd.pAckBuf);
	}	


	Ret = HAL_OK;

end:

	return Ret;	
}


int net_connect_by_at(const char *host,  int port,  void* pdata)
{
	sAtCmd atCmd;	
	char ipCmdStr[IP_CMD_LEN];
	int Ret  = HAL_ERROR;

	memset(ipCmdStr, 0, IP_CMD_LEN);
	snprintf(ipCmdStr, IP_CMD_LEN, "AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r\n", host, port);
	AT_INFO_DEBUG("\n\ripCmdStr[%s]", ipCmdStr);
	
	init_send_cmd(&atCmd, ipCmdStr, "CONNECT OK", 2*TIMOUT_5S);
	
	AT_INFO_DEBUG("\n\rat_buf[%s]", atCmd.at_buf);
	
	if(HAL_OK ==  exec_gsm_at(&atCmd))
	{
		Ret = HAL_OK;
		AT_INFO_DEBUG("\n\rTCP/IP connect setup success");
	}
	else
	{
		Ret = HAL_ERROR;
		AT_INFO_DEBUG("\n\rTCP/IP connect setup fail");
	}

	return Ret;	
}

int net_disconnect_by_at(void* pdata)
{
	sAtCmd atCmd;	
	int Ret = HAL_ERROR;

	
	init_send_cmd(&atCmd, "AT+CIPCLOSE\r\n", "OK", TIMOUT_1S);
	if(HAL_OK ==  exec_gsm_at(&atCmd))
	{
		Ret = HAL_OK;
		AT_INFO_DEBUG("\n\rTCP/IP disconnect success");
	}
	else
	{
		Ret = HAL_ERROR;
		AT_INFO_DEBUG("\n\rTCP/IP disconnect fail");
	}
	
	return Ret;	
}


int net_connectState_by_at(void* pdata)
{
	sAtCmd atCmd;	
	int Ret  = HAL_ERROR;

	
	init_send_cmd(&atCmd, "AT+CIPSTATUS\r\n", "CONNECT OK", TIMOUT_5S);
	if(HAL_OK ==  exec_gsm_at(&atCmd))
	{
		Ret = HAL_OK;
		AT_INFO_DEBUG("\n\rTCP/IP connect status ok");
	}
	else
	{
		Ret = HAL_ERROR;
		AT_INFO_DEBUG("\n\rTCP/IP not connect");
	}

	return Ret;	
}

int net_destroy_by_at(void* pdata)
{
	sAtCmd atCmd;	
	int Ret = HAL_ERROR;

	
	init_send_cmd(&atCmd, "AT+CIPSHUT\r\n", "OK", TIMOUT_1S);
	if(HAL_OK ==  exec_gsm_at(&atCmd))
	{
		Ret = HAL_OK;
		AT_INFO_DEBUG("\n\rnet destroy success");
	}
	else
	{
		Ret = HAL_ERROR;
		AT_INFO_DEBUG("\n\rnet destroy fail");
	}
	
	return Ret;	
}


/*
*下发AT+CIPRXGET=2,len手动读取模组AT buf中的数据
* ackbuf中会得到格式如下的数据
   +CIPRXGET: 2,len,rest_len \n\r
   xxxxxx\n\r   (内容数据)
   OK
**函数从上述格式的atbuf中提取出指定长度数据
*/

static int copyDataToBuf(uint8_t *pFromBuf, uint8_t *pToBuf, uint16_t len)
{
	uint8_t *pPosition = NULL;
	uint8_t lenChar[RECV_LEN_NUM_MAX + 1] = {'\0'};
	int i = 0;
	int num;
	int readlen;
	int	offset = 0;
	bool startflag = false;
	int Ret = RET_ERR;
	int delayTime;

	if(!pFromBuf || !pToBuf || (len < 1))
	{
		return RET_ERR;
	}

	
	//HAL_Delay(500);

	delayTime = (len/100)*100 + 350;
	HAL_Delay(delayTime);

	pPosition = strstr(pFromBuf, READ_CMD_ACK_PATTEN);
	if(NULL != pPosition)
	{
		offset = strlen(READ_CMD_ACK_PATTEN);
		
		for(num = 0; num < RECV_LEN_NUM_MAX; num++)
		{
			if(NUM_END_PATTEN != pPosition[offset + num])
			{
				lenChar[num] = pPosition[offset + num];	
			}
			else
			{
				readlen = atoi(lenChar);
				len = (readlen > NET_READ_MAXLEN_ONCE)?len:readlen;				
			}							
		}
		
		
		for(i = 0; i < (AT_RECIVE_MAX_BUF_LEN - offset); i++)
		{
			if((END_PATTEN1 == pPosition[offset + i])||(END_PATTEN2 == pPosition[offset + i]))
			{
				if((END_PATTEN1 != pPosition[offset + i + 1])&&(END_PATTEN2 != pPosition[offset + i + 1]))
				{	
					startflag = true;
					break;
				}
			}		
		}

		if(true == startflag)
		{
			pPosition += offset + i + 1; 
			memcpy(pToBuf, pPosition, len);
			Ret = len;			
		}
	}

	return Ret;
}

int net_read_by_at(uint8_t *pRecvbuf, int len, uint16_t timeOut)
{
	sAtCmd atCmd;		
	char readCmdStr[IP_CMD_LEN];
	uint8_t count = 0;
	int readLen;
	int retlen = 0;
	int Ret = HAL_ERROR;
	uint32_t ulStartTick;
	uint32_t ulEndTick;	

	HAL_Delay(300);

	ulStartTick = HAL_GetTick();
	ulEndTick = ulStartTick + timeOut;

	while((len > 0)&&(ulEndTick > HAL_GetTick()))
	{		
		if(len > NET_READ_MAXLEN_ONCE)
		{
			readLen = NET_READ_MAXLEN_ONCE;	
			len -= NET_READ_MAXLEN_ONCE;
			count++;
		}
		else
		{
			readLen = len;
			len = 0;
		}

		memset(readCmdStr, 0, IP_CMD_LEN);
		snprintf(readCmdStr, IP_CMD_LEN, "AT+CIPRXGET=2,%d\r\n", readLen);	
		init_send_cmd(&atCmd, readCmdStr, READ_CMD_ACK_PATTEN, TIMOUT_5S);
		
		if(HAL_OK ==  exec_gsm_at(&atCmd))
		{
			retlen = copyDataToBuf((uint8_t *)atCmd.pAckBuf, (pRecvbuf + count*NET_READ_MAXLEN_ONCE) , readLen);
			if(RET_ERR ==  retlen)
			{			
				AT_INFO_DEBUG("\n\r[net_read_by_at]copyDataToBuf err");	
				Ret = HAL_ERROR;
				break;
			}
			else if(retlen < readLen)
			{
				AT_INFO_DEBUG("\n\r[net_read_by_at]copyDataToBuf data less than read needed");	
				Ret = count*NET_READ_MAXLEN_ONCE + retlen;
				break;
				
			}else if(0 == len)
			{
				Ret = count*NET_READ_MAXLEN_ONCE + retlen;
			}
		}
		else
		{
			Ret = HAL_ERROR;
			break;
		}		
	}	

	AT_INFO_DEBUG("\n\r[net_read_by_at] return [%d]", Ret);	

	return Ret;	
}


int net_write_by_at(uint8_t *pSendbuf, int len, uint16_t timeOut)
{
	sAtCmd atCmd;		
	char writeCmdStr[IP_CMD_LEN];
	uint8_t count = 0;
	int sendLen;
	int Ret = HAL_ERROR;
	uint32_t ulStartTick;
	uint32_t ulEndTick;	

	ulStartTick = HAL_GetTick();
	ulEndTick = ulStartTick + timeOut;

	while((len > 0)&&(ulEndTick > HAL_GetTick()))
	{		
		if(len > NET_SEND_MAXLEN_ONCE)
		{
			sendLen = NET_SEND_MAXLEN_ONCE;	
			len -= NET_SEND_MAXLEN_ONCE;
			count++;
		}
		else
		{
			sendLen = len;
			len = 0;
		}

		memset(writeCmdStr, 0, IP_CMD_LEN);
		snprintf(writeCmdStr, IP_CMD_LEN, "AT+CIPSEND=%d\r\n", sendLen);			
		init_send_cmd(&atCmd, writeCmdStr, ">", TIMOUT_3S);
		
		if(HAL_OK ==  exec_gsm_at(&atCmd))
		{
			if(HAL_ERROR == (at_send_data(pSendbuf + count*NET_SEND_MAXLEN_ONCE, sendLen)))
			{
				AT_INFO_DEBUG("\n\r[net_write_by_at]at_send_data err");	
				Ret = HAL_ERROR;
				
			}else if(0 == len)
			{
				Ret = HAL_OK;
			}
		}
		else
		{
			Ret = HAL_ERROR;
			break;
		}
	}

	if(HAL_OK == WaitForAck("SEND OK",  TIMOUT_5S))
	{
		if(HAL_OK == Ret)
		{
			Ret = count*NET_SEND_MAXLEN_ONCE + sendLen;
		}
	}	
	else
	{
		AT_INFO_DEBUG("\n\r[WaitForAck]timeout");		
	}

	return Ret;	
}



/*
*获取localtime，动态token获取需要时间戳
*比对 http://gz.auth-device-iot.tencentcloudapi.com/time
*/
int GetLocalTimeByAt(uint8_t * strTime)
{
	sAtCmd atCmd;	 
	int Ret = HAL_ERROR;
	
	
	init_send_cmd(&atCmd, "AT+CCLK?\r\n", "OK", TIMOUT_2S);
	if(HAL_OK ==  exec_gsm_at(&atCmd))
	{
		Ret = HAL_OK;	
		sscanf((uint8_t *)atCmd.pAckBuf, "%*s %s %*s", strTime);  
		AT_INFO_DEBUG("\n\rGetLocalTime [%s]", atCmd.pAckBuf);
	}
	else
	{
		Ret = HAL_ERROR;
		AT_INFO_DEBUG("\n\rGetLocalTimeByGsm fail");
	}
	
	return Ret; 
}


/*
* strTime: string time input,  "18/06/19,17:35:55+32"
* 将本地时间转换为相对UTC 的秒时间
*/
long GetTimeStampByAt(void *zone)
{
	char strTime[TIME_STR_LEN];
	char YYtimeChar[TIME_STR_LEN];
	char HHtimeChar[TIME_STR_LEN];
	uint8_t *pHH = NULL;
	int yy, mm, dd, hh, mimi, ss;
	struct tm tm_time;
	long unixtime;
	int zoneTime;

	
	memset(strTime, 0, TIME_STR_LEN);
	memset(YYtimeChar, 0, TIME_STR_LEN);
	memset(HHtimeChar, 0, TIME_STR_LEN);

	if(HAL_ERROR == GetLocalTimeByAt(strTime))
	{
		unixtime = 0;
		goto end;
	}
	
	
	sscanf(strTime + 1, "%[^,]", YYtimeChar);  	
	pHH = strchr(strTime, ',');
	sscanf(pHH + 1, "%[^+]%", HHtimeChar); 

	
	sscanf(YYtimeChar, "%d/%d/%d", &yy, &mm, &dd);	
	sscanf(HHtimeChar, "%d:%d:%d", &hh, &mimi, &ss);

	tm_time.tm_year = yy + TIME_BASE_YEAR - 1900;
	tm_time.tm_mon = mm - 1;
	tm_time.tm_mday = dd;
	tm_time.tm_hour = hh;
	tm_time.tm_min = mimi;
	tm_time.tm_sec = ss;

	if(NULL == zone)
	{
		zoneTime = ZONE_FOR_BEIJING;
	}
	
	unixtime = mktime(&tm_time);
	unixtime -= zoneTime*3600;   
	
	AT_INFO_DEBUG("\n\rYYtime[%s] HHtime[%s]",YYtimeChar, HHtimeChar);
	AT_INFO_DEBUG("\n\r[NewTime]%d-%d-%d %d:%d:%d", yy + TIME_BASE_YEAR, mm, dd, hh, mimi, ss);
	AT_INFO_DEBUG("\n\rUTC [%d]s", unixtime);


end:

	return unixtime;
}






/**
* @ 上电时序要求参见gsm 模组要求
*/
void HwInitL206(void)
{
	int count;

	for(count = 0; count < AT_REPEAT_TIME; count++)
	{
		if(HAL_OK == atAlive())
		{	
			AT_INFO_DEBUG("\n\rAT Ready");
			break;
		}
	}

#if (BOARD_TYPE == BOARD_TYPE_SENSOR)
	if(count > (AT_REPEAT_TIME - 1))
	{	
		AT_INFO_DEBUG("\n\rAT not Ready");
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
//		HAL_Delay(POWER_ON_TIME);
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
//		HAL_Delay(POWER_ON_TIME/2);
	}
#else
	#error unknow board type
#endif
}

/***********************END OF FILE**************************/
