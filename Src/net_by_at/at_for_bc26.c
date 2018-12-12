/*********************************************************************************
 *                   Copyright (c) 2018 - 2020,Tencent
 *                      All rights reserved.
 *
 * File Name:    at_for_bc28.c
 *
 * Description:   net api based on at cmd for l206
 *
 * History:      <author>          <time>        		<version>
 *                   yougaliu          2018-11-26             1.0
 * Desc:           ORG.
 ********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "at_driver.h"
#include "at_for_bc26.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

#include "time.h"



static uint8_t g_SocketBitMap = 0;

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

	init_send_cmd(&atCmd,  "AT\r\n", "OK", TIMOUT_2S);
	
	return exec_gsm_at(&atCmd);
}



HAL_StatusTypeDef atReset(void)
{
	sAtCmd atCmd;	

	init_send_cmd(&atCmd,  "AT+QRST=1\r\n", NULL, TIMOUT_1S);
	
	return exec_gsm_at(&atCmd);
}

void atSocketCloseAll(void)
{
	sAtCmd atCmd;	
	char CmdStr[IP_CMD_LEN];
	int Ret = HAL_ERROR;
	int fd;

	for(fd = 0; fd < BC26_MAX_SOCKETS_NUM; fd++)
	{
		memset(CmdStr, 0, IP_CMD_LEN);
		snprintf(CmdStr, IP_CMD_LEN, "AT+QICLOSE=%d\r\n", fd);
		init_send_cmd(&atCmd, CmdStr, NULL, TIMOUT_1S);
		exec_gsm_at(&atCmd);		
	}

	AT_INFO_DEBUG("\n\rAll socket closed");	
	return; 
}




int net_init_by_at(void)
{
	sAtCmd atCmd;	
	int Ret = HAL_ERROR;
	int count;

	for(count = 0; count < AT_REPEAT_TIME; count++)
	{
		if(HAL_OK == atAlive())
		{	
			AT_INFO_DEBUG("\n\rBC26 ready");
			//break;
		}
	}

	
	if(HAL_OK !=  atEchoOff())
	{
		AT_INFO_DEBUG("\n\ratEcho off fail");
	}

#if 0//Get AT FW information
	init_send_cmd(&atCmd, "ATI\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		AT_INFO_DEBUG("\n\rATI fail");
		//goto end;
	}

	init_send_cmd(&atCmd, "AT+CGMR\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		AT_INFO_DEBUG("\n\rCGMR fail");
		//goto end;
	}

	init_send_cmd(&atCmd, "AT+CGDCONT?\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		AT_INFO_DEBUG("\n\rCGDCONT fail");
		//goto end;
	}


	init_send_cmd(&atCmd, "AT+QICFG=\"showlength\",1\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		AT_INFO_DEBUG("\n\rQICFG fail");
		//goto end;
	}

	init_send_cmd(&atCmd, "AT+QICFG=\"dataformat\",1,1\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		AT_INFO_DEBUG("\n\rdataformat fail");
		//goto end;
	}
#endif	

	/*获取IMI  号*/
	init_send_cmd(&atCmd, "AT+CIMI\r\n", NULL, TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		goto end;
	}
	else
	{
		AT_INFO_DEBUG("\n\rGet IMI:[%s]", atCmd.pAckBuf);
	}

//	init_send_cmd(&atCmd, "AT+CPSMS=0\r\n", "OK", TIMOUT_1S);
//	if(HAL_OK !=  exec_gsm_at(&atCmd))
//	{
//		goto end;
//	}

//	init_send_cmd(&atCmd, "AT+QSCLK=0\r\n", "OK", TIMOUT_1S);
//	if(HAL_OK !=  exec_gsm_at(&atCmd))
//	{
//		goto end;
//	}


	
	init_send_cmd(&atCmd, "AT+QICFG=\"showlength\",1\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		AT_INFO_DEBUG("\n\rShowlength set fail");
		//goto end;
	}

	init_send_cmd(&atCmd, "AT+QICFG=\"dataformat\",1,1\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		AT_INFO_DEBUG("\n\rDataformat set fail");
		//goto end;
	}


	init_send_cmd(&atCmd, "AT+QCGDEFCONT=\"IP\"\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		goto end;
	}

	init_send_cmd(&atCmd, "AT+CGDCONT?\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		AT_INFO_DEBUG("\n\rCGDCONT fail");
		//goto end;
	}
	

	init_send_cmd(&atCmd, "AT+SM=LOCK\r\n", "OK", TIMOUT_1S);
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		goto end;
	}
	

	init_send_cmd(&atCmd, "AT+CFUN?\r\n", "+CFUN: 1", TIMOUT_1S);		
	if(HAL_OK !=  exec_gsm_at(&atCmd))
	{
		AT_INFO_DEBUG("\n\rNot full  functionality");
	}
		
	atSocketCloseAll();

	init_send_cmd(&atCmd, "AT+CSQ\r\n",  NULL, TIMOUT_1S);
	for(count = 0; count < (AT_CHECK_STAT_TIME + 1); count++)
	{
		if(HAL_OK !=  exec_gsm_at(&atCmd))
		{
			goto end;
		}
		else
		{		
			AT_INFO_DEBUG("\n\rGet CSQ:[%s]", atCmd.pAckBuf);
			if((strstr(atCmd.pAckBuf,"+CSQ: 0,0")) || (strstr(atCmd.pAckBuf,"+CSQ: 99,99")))
			{
				AT_INFO_DEBUG("\n\rCSQ ERROR");
				continue;
			}
			else
			{
				break;
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
	
	Ret = HAL_OK;

end:

	return Ret;	
}

int alloc_socket(void)
{
	int fd;
	uint8_t i;

	for(i = 0; i < BC26_MAX_SOCKETS_NUM; i++)
	{
		if(0 == ((g_SocketBitMap>>i) & 0x01))
		{
			g_SocketBitMap |= 1<<i;
			break;
		}
	}

	(i < BC26_MAX_SOCKETS_NUM)?(fd = i):(fd = -1);

	return fd;	
}

int net_connect_by_at(const char *host,  int port, int *pfd,  eNetProto protocol, void* pdata)
{
	sAtCmd atCmd;	
	char CmdStr[IP_CMD_LEN];
	char ConAck[CONNECT_ACK_LEN];
	int Ret  = HAL_ERROR;
	

	/*Get fd*/	
	*pfd = alloc_socket();

	if(*pfd < 0)
	{
		AT_INFO_DEBUG("\n\rSocket exhausted");
		return HAL_ERROR;
	}

	AT_INFO_DEBUG("\n\rSocket[%d] alloced", *pfd);
	
	/*set up connect*/	
	memset(CmdStr, 0, IP_CMD_LEN);
	if(eUDP == protocol)
	{
		snprintf(CmdStr, IP_CMD_LEN, "AT+QIOPEN=1,%d,\"UDP\",\"%s\",%d,0,0\r\n", *pfd, host, port);
	}
	else
	{
		snprintf(CmdStr, IP_CMD_LEN, "AT+QIOPEN=1,%d,\"TCP\",\"%s\",%d,0,0\r\n", *pfd, host, port);
	}
	
	AT_INFO_DEBUG("\n\rCmdStr[%s]", CmdStr);


	memset(ConAck, 0, CONNECT_ACK_LEN);
	snprintf(ConAck, CONNECT_ACK_LEN, "+QIOPEN: %d,0", *pfd);
	init_send_cmd(&atCmd, CmdStr, ConAck, 2*TIMOUT_5S);
	
	AT_INFO_DEBUG("\n\rat_buf[%s]", atCmd.at_buf);
	
	if(HAL_OK ==  exec_gsm_at(&atCmd))
	{
		Ret = HAL_OK;
		AT_INFO_DEBUG("\n\rnet connect setup success");
	}
	else
	{

		/*It seems that QIOPEN ok ack sometimes missed, workaround temporarily*/
		if((NULL != strstr(atCmd.pAckBuf, "+QIOPEN: 0,566"))||(NULL != strstr(atCmd.pAckBuf, "+QIOPEN: 0,565")))
		{
			Ret = HAL_ERROR;
			net_disconnect_by_at(*pfd, NULL);
			AT_INFO_DEBUG("\n\rnet connect setup fail, ackstr{%s}",atCmd.pAckBuf);
		}
		else		
		{
			Ret = HAL_OK;
			AT_INFO_DEBUG("\n\rnet connect setup as workaound");
		}	
	}

	return Ret;	
}

int net_disconnect_by_at(uint32_t fd, void* pdata)
{
	sAtCmd atCmd;	
	char CmdStr[IP_CMD_LEN];
	int Ret = HAL_ERROR;

	memset(CmdStr, 0, IP_CMD_LEN);
	snprintf(CmdStr, IP_CMD_LEN, "AT+QICLOSE=%d\r\n", fd);

	
	init_send_cmd(&atCmd, CmdStr, "OK", TIMOUT_1S);
	if(HAL_OK ==  exec_gsm_at(&atCmd))
	{
		g_SocketBitMap &= ~(1<<fd);
		Ret = HAL_OK;
		AT_INFO_DEBUG("\n\rnet disconnect success");
	}
	else
	{
		Ret = HAL_ERROR;
		AT_INFO_DEBUG("\n\rnet disconnect fail");
	}
	
	return Ret;	
}


int net_connectState_by_at(uint32_t fd, void* pdata)
{
	return g_SocketBitMap;	
}

int net_destroy_by_at(uint32_t fd,void* pdata)
{
	return HAL_OK;	
}



//Hex to string 
void HexToStr(uint8_t *source, uint8_t *dest,  uint16_t len)
{

    uint16_t i;
	uint8_t HighVal;
	uint8_t	LowVal;
	
    for(i = 0; i < len; i++)
    {
        HighVal = source[i]>>4;
        LowVal = source[i] & 0x0f;
		dest[i*2] = (HighVal > 9)?(HighVal - 10 + 'a'):(HighVal + '0');
		dest[i*2+1] = (LowVal > 9)?(LowVal - 10 + 'a'):(LowVal + '0');
    } 
    
}

 
//String to Hex 
void StrToHex(const char* source, unsigned char* dest, int sourceLen)
{
    uint16_t i;
    unsigned char highByte, lowByte;
    
    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper(source[i]);
        lowByte  = toupper(source[i + 1]);
 
        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;
 
        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;
 
        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return ;
}


/*
*下发AT+CIPRXGET=2,len手动读取模组AT buf中的数据
* ackbuf中会得到格式如下的数据
   +CIPRXGET: 2,len,rest_len \n\r
   xxxxxx\n\r   (内容数据)
   OK
**函数从上述格式的atbuf中提取出指定长度数据
*/

static int copyDataToBuf(uint8_t *pSource, uint8_t *pDest, uint16_t len)
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

	if(!pSource || !pDest || (len < 1))
	{
		return RET_ERR;
	}

		
	if(NULL != strstr(pSource, READ_CMD_ACK_NULL))
	{
		return 0;  //nothing read
	}

	

	delayTime = (len > 250)?1000:600;
	HAL_Delay(delayTime);

	

	pPosition = strstr(pSource, READ_CMD_ACK_PATTEN);
	if(NULL != pPosition)
	{
		offset = strlen(READ_CMD_ACK_PATTEN);

		if('0' == pPosition[offset])
		{	
			return 0;	 //nothing read
		}
		
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
				AT_INFO_DEBUG("\n\rrecv_len:[%s][%d]", lenChar, len);
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

				
			StrToHex(pPosition, pDest, 2*len);
			//memcpy(pDest, pPosition, len);
			Ret = len;			
		}
		else
		{
			Ret = 0;
		}
	}

	return Ret;
}


int net_read_by_at(uint32_t fd, uint8_t *pRecvbuf, int len, uint16_t timeOut)
{
	sAtCmd atCmd;		
	char readCmdStr[IP_CMD_LEN];
	uint8_t count = 0;
	int readLen;
	int retlen = 0;
	int origilen = len; 
	int Ret = 0;
	uint32_t ulStartTick;
	uint32_t ulEndTick;	

	//HAL_Delay(300);

	ulStartTick = HAL_GetTick();
	ulEndTick = ulStartTick + timeOut;

	while((len > 0)&&(ulEndTick > HAL_GetTick()))
	{		

		readLen = (len > NET_READ_MAXLEN_ONCE)?(NET_READ_MAXLEN_ONCE):len;
		memset(readCmdStr, 0, IP_CMD_LEN);
		snprintf(readCmdStr, IP_CMD_LEN, "AT+QIRD=%d,%d\r\n", fd, readLen);	
		init_send_cmd(&atCmd, readCmdStr, READ_CMD_ACK_PATTEN, TIMOUT_5S);
		
		if(HAL_OK ==  exec_gsm_at(&atCmd))
		{
			retlen = copyDataToBuf((uint8_t *)atCmd.pAckBuf, (pRecvbuf + Ret) , readLen);
			if(RET_ERR ==  retlen)
			{			
				AT_INFO_DEBUG("\n\r[net_read_by_at]copyDataToBuf err");	
				Ret = HAL_ERROR;
				break;
			}
			else 
			{								
				Ret += retlen;		
				len -= retlen;

				
				if((0 == retlen)||(len < 1))
				{
					break;
				}
			}
		}
		else
		{
			Ret = HAL_ERROR;
			break;
		}		
	}	

	if(len > 0)
	{	
		/*Timeout*/
		AT_INFO_DEBUG("\n\rnet_read_by_at read data less than needed");	
	}

	AT_INFO_DEBUG("\n\r[net_read_by_at] return [%d]", Ret);	

	return Ret;	
}

int net_write_by_at(uint32_t fd, uint8_t *pSendbuf, int len, uint16_t timeOut)
{
	sAtCmd atCmd;		
	char writeCmdStr[IP_CMD_LEN];
	uint8_t sendBuff[2*NET_SEND_MAXLEN_ONCE]; /*sendBuff malloc is proposed if avaliable*/
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

		FlushDataBuff();
		memset(sendBuff, 0, 2*NET_SEND_MAXLEN_ONCE);
		HexToStr(pSendbuf + count*NET_SEND_MAXLEN_ONCE, sendBuff, sendLen); 
		memset(writeCmdStr, 0, IP_CMD_LEN);
		snprintf(writeCmdStr, IP_CMD_LEN, "AT+QISENDEX=%d,%d,", fd, sendLen);	
		AT_INFO_DEBUG("\n\r[writeCmdStr]%s", writeCmdStr);	
		Ret  = at_send_data(writeCmdStr, strlen(writeCmdStr));
		Ret |= at_send_data(sendBuff, 2*sendLen);
		Ret |= at_send_data(END_PATTEN, strlen(END_PATTEN));

		if(HAL_OK != Ret)
		{
			AT_INFO_DEBUG("\n\r at_send_data fail,Ret:%d", Ret);
			goto end;
		}
		
		if(HAL_OK == WaitForAck("SEND OK",	2*TIMOUT_5S))
		{
			if(HAL_OK == Ret)
			{
				Ret = count*NET_SEND_MAXLEN_ONCE + sendLen;
			}
		}	
		else
		{
			AT_INFO_DEBUG("\n\r[WaitForAck]timeout");
			Ret = HAL_ERROR;
			break;
		}
		
	}
		
end:

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



int NetInit(void)
{
	int rc;

	
	if(HAL_OK != net_init_by_at())
	{
		if(HAL_OK != net_init_by_at())
		{
			AT_INFO_DEBUG("\n\rNet init Fail"); 
			rc = HAL_ERROR;
		}
		else
		{				
			AT_INFO_DEBUG("\n\rNet 2th init OK");	
			rc = HAL_OK;
		}
	}
	else
	{		
		AT_INFO_DEBUG("\n\rNet init OK");	
		rc = HAL_OK;
	}

	return rc;
}

/**
* @ 上电时序要求参见模组要求
*/
void HwInitBc26(void)
{	
#if (BOARD_TYPE == BOARD_TYPE_CARD)	
	/*power on*/
	AT_INFO_DEBUG("\n\rBC26 Power on"); 		
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_Delay(POWER_ON_TIME);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
#else
	#error unknow board type
#endif
}

/***********************END OF FILE**************************/
