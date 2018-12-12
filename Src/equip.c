/**
  ******************************************************************************
  * @file	equip.c
  * @author	yougaliu	
  * @date	2018-12-06
  ******************************************************************************
  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>  
#include <stdbool.h> 
#include <stdarg.h>
#include "equip.h"
#include "version.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_it.h"
#include "peripheral_drv.h"

//AT server not transplant for just TWO at cmd

extern UART_HandleTypeDef huart3;

#if BOARD_TYPE == BOARD_TYPE_CARD
	static UART_HandleTypeDef *pEquipUart = &huart3;
#else
	#error unknow board type
#endif

static sEquipRecvData g_EuipRecvData = {{0,}, 0, eRECV_ABLE};
static char g_last_ch;
static bool g_CmdWaitDealSem = false;

/*The cmd list that is supported*/
sEuipCmd g_Cmd_List[]= {
							{"AT+TEST", eAT_TEST},
							{"AT+PARAW", eSYS_PARA_WRITE},
							{"AT+PARAR?", eSYS_PARA_READ},
							{"AT+VER?", eFW_CHECK},
					  };


/*
*@brief 装备串口接收中断处理函数
*/
void EQUIP_UART_IRQHandler(void)
{ 
	if(__HAL_UART_GET_FLAG(pEquipUart, UART_FLAG_RXNE) == SET)
	{	
		if(false == g_CmdWaitDealSem)
		{
			if(g_EuipRecvData.pos  > (EQUIP_RECIVE_MAX_BUF_LEN - 1))
			{
				g_EuipRecvData.status = eFULL;
				g_CmdWaitDealSem = true;
			}
			else
			{
				g_EuipRecvData.recv_buf[g_EuipRecvData.pos] = (uint8_t) READ_REG(pEquipUart->Instance->DR)&0xFF;					
			}


			// \r\n means a new cmd
			if((AT_CMD_LF == g_EuipRecvData.recv_buf[g_EuipRecvData.pos]) && (AT_CMD_CR == g_last_ch))
			{
				g_EuipRecvData.recv_buf[g_EuipRecvData.pos + 1]='\0';
				g_CmdWaitDealSem = true;
			}
			else
			{
				g_last_ch = g_EuipRecvData.recv_buf[g_EuipRecvData.pos];
				g_EuipRecvData.pos++;								
			}
		}
		//__HAL_UART_CLEAR_FLAG(pAtUart, UART_IT_RXNE);				
	}
	__HAL_UART_CLEAR_PEFLAG(pEquipUart);
}

bool atCmdCome(void)
{
	return g_CmdWaitDealSem;
}


/*
*@brief 装备串口数据发送函数，支持可变参数
*/
void EuipDataSend(const char* fmt,...)
{
	va_list ap;
    char str[256];   

    str[255]='\0';
    va_start(ap, fmt);
    vsprintf(str, fmt, ap);
    va_end(ap);
	
	HAL_UART_Transmit(pEquipUart, (uint8_t *)str, strlen(str), 0xFFFF);
}


/*
*@brief 装备测试命令列表显示
*/
void equip_menu_show(void)
{
	EuipDataSend("****************************************\n\r");
	EuipDataSend("********* 装备测试命令列表 *************\n\r");
	EuipDataSend("****************************************\n\r");
	EuipDataSend("AT+TEST: OK will be returned\n\r");
	EuipDataSend("AT+PARAW: =<Product_id>,<Device_Name>,<Device_srec> ,Device information written\n\r");
	EuipDataSend("AT+PARAR?: <Product_id>,<Device_Name>,<Device_srec>, Device information readback\n\r");
	EuipDataSend("AT+VER?: FirmWare version information return\n\r");
}

/*
*@brief 清空AT 命令buf
*/
static void ResetCmdBuf(void)
{
	memset((uint8_t *)&g_EuipRecvData, 0x0, sizeof(sEquipRecvData));
	g_CmdWaitDealSem = false;	
}


/*
*@brief  解析AT 命令
*/
static eCmdType ParseCmdType(void)
{
	uint8_t  i;
	uint8_t  cmd_num;
	eCmdType Ret = eDefault;
	
	cmd_num = sizeof(g_Cmd_List) / sizeof(g_Cmd_List[0]);
	for(i = 0; i < cmd_num; i++)
	{
		if(strstr(g_EuipRecvData.recv_buf, g_Cmd_List[i].at_buf) != NULL)
		{
			Ret = g_Cmd_List[i].type;
			break;
		}
	}	

	return Ret;
}


void dumpCmdlist(void)
{
	uint8_t  i;
	uint8_t  cmd_num;

	
	cmd_num = sizeof(g_Cmd_List) / sizeof(g_Cmd_List[0]);
	for(i = 0; i < cmd_num; i++)
	{
		EuipDataSend("\n\r%dth Cmd:%s", i, g_Cmd_List[i].at_buf);
	}	

	return;
}

/**
 * at_sscanf - Unformat a buffer into a list of arguments, rewrite sscanf
 * @buf:	input buffer
 * @fmt:	format of buffer
 * @args:	arguments
 */
static int at_sscanf(const char * buf, const char * fmt, va_list args)
{
	const char *str = buf;
	char *next;
	int num = 0;
	int qualifier;
	int base;
	int field_width = -1;
	int is_sign = 0;

	while(*fmt && *str) {
		/* skip any white space in format */
		/* white space in format matchs any amount of
		 * white space, including none, in the input.
		 */
		if (isspace(*fmt)) {
			while (isspace(*fmt))
				++fmt;
			while (isspace(*str))
				++str;
		}

		/* anything that is not a conversion must match exactly */
		if (*fmt != '%' && *fmt) {
			if (*fmt++ != *str++)
				break;
			continue;
		}

		if (!*fmt)
			break;
		++fmt;
		
		/* skip this conversion.
		 * advance both strings to next white space
		 */
		if (*fmt == '*') {
			while (!isspace(*fmt) && *fmt)
				fmt++;
			while (!isspace(*str) && *str)
				str++;
			continue;
		}

		/* get field width */
		if (isdigit(*fmt))
			field_width = atoi(fmt);

		/* get conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z') {
			qualifier = *fmt;
			fmt++;
		}
		base = 10;
		is_sign = 0;

		if (!*fmt || !*str)
			break;

		switch(*fmt++) {
		case 'c':
		{
			char *s = (char *) va_arg(args,char*);
			if (field_width == -1)
				field_width = 1;
			do {
				*s++ = *str++;
			} while(field_width-- > 0 && *str);
			num++;
		}
		continue;
		case 's':
		{
			char *s = (char *) va_arg(args, char *);
			if(field_width == -1)
				field_width = INT_MAX;
			/* first, skip leading white space in buffer */
			while (isspace(*str))
				str++;

			/* now copy until next white space */
			while (*str && ((*str) != ',')) {
				if(isspace(*str)){
					str++;
				}else{
					*s++ = *str++;
				}			
			}
			*s = '\0';
			num++;
		}
		continue;
		case 'n':
			/* return number of characters read so far */
		{
			int *i = (int *)va_arg(args,int*);
			*i = str - buf;
		}
		continue;
		case 'o':
			base = 8;
			break;
		case 'x':
		case 'X':
			base = 16;
			break;
		case 'd':
		case 'i':
			is_sign = 1;
		case 'u':
			break;
		case '%':
			/* looking for '%' in str */
			if (*str++ != '%') 
				return num;
			continue;
		default:
			/* invalid format; stop here */
			return num;
		}

		/* have some sort of integer conversion.
		 * first, skip white space in buffer.
		 */
		while (isspace(*str))
			str++;

		if (!*str || !isdigit(*str))
			break;

		switch(qualifier) {
		case 'h':
			if (is_sign) {
				short *s = (short *) va_arg(args,short *);
				*s = (short) strtol(str,&next,base);
			} else {
				unsigned short *s = (unsigned short *) va_arg(args, unsigned short *);
				*s = (unsigned short) strtoul(str, &next, base);
			}
			break;
		case 'l':
			if (is_sign) {
				long *l = (long *) va_arg(args,long *);
				*l = strtol(str,&next,base);
			} else {
				unsigned long *l = (unsigned long*) va_arg(args,unsigned long*);
				*l = strtoul(str,&next,base);
			}
			break;
		case 'L':
			if (is_sign) {
				long long *l = (long long*) va_arg(args,long long *);
				*l = strtoll(str,&next,base);
			} else {
				unsigned long long *l = (unsigned long long*) va_arg(args,unsigned long long*);
				*l = strtoull(str,&next,base);
			}
			break;
		case 'Z':
		{
			unsigned long *s = (unsigned long*) va_arg(args,unsigned long*);
			*s = (unsigned long) strtoul(str,&next,base);
		}
		break;
		default:
			if (is_sign) {
				int *i = (int *) va_arg(args, int*);
				*i = (int) strtol(str,&next,base);
			} else {
				unsigned int *i = (unsigned int*) va_arg(args, unsigned int*);
				*i = (unsigned int) strtoul(str,&next,base);
			}
			break;
		}
		num++;

		if (!next)
			break;
		str = next;
	}
	return num;
}

char * at_strip(char *str, const char patten )
{
	char *start, *end;
	
	start = str;			
	end = str + strlen(str) -1;	
	
	if(*str == patten)
	{
		start++;
	}

	if(*end == patten)
	{
		*end-- = '\0';	
	}

	return strcpy(str, start);
}



static int at_req_parse_args(const char *req_args, const char *req_expr, ...)
{
    va_list args;
    int req_args_num = 0;

    va_start(args, req_expr);

    //req_args_num = vsscanf(req_args, req_expr, args);	
    req_args_num = at_sscanf(req_args, req_expr, args);
    
    va_end(args);

    return req_args_num;
}


static int parse_args(smanuInfo *pInfo)
{
	const char *req_expr = "=%s,%s,%s";
	int argc;
	int i=0;
	int rc = HAL_OK;

	
    while(('=' != g_EuipRecvData.recv_buf[i++])&&(i < EQUIP_RECIVE_MAX_BUF_LEN));
	
	argc = at_req_parse_args(&(g_EuipRecvData.recv_buf[i-1]), req_expr, pInfo->productId, pInfo->devName, pInfo->devSerc);
	if(3 != argc)
	{
		
		return HAL_ERROR;
	}

	at_strip(pInfo->productId, AT_CMD_DQUOTES_MARK);
	at_strip(pInfo->devName, AT_CMD_DQUOTES_MARK);
	at_strip(pInfo->devSerc, AT_CMD_DQUOTES_MARK);

	if((EXHIBIGTOR_PRODUCT_ID_LEN != strlen(pInfo->productId))
		||(EXHIBIGTOR_DEV_NAME_LEN != strlen(pInfo->devName))
		||(EXHIBIGTOR_DEV_SECRET_LEN != strlen(pInfo->devSerc)))
	{		
		EuipDataSend("+CME ERROR:%d\n", eAUTH_ERR);	
		rc = HAL_ERROR;
	}

	return rc;
}


/*
*@brief 获取固件版本
*/
int FwVersion(void)
{
	EuipDataSend("FwVersion: %s\n", FW_VERSION_STR);
	
	return HAL_OK;
}


int EquipTestDeal(void)
{
	int rc = HAL_ERROR;
	eCmdType eType;
	smanuInfo info;

	eType = ParseCmdType(); 
	switch(eType)
	{
		case eAT_TEST:
				rc = HAL_OK;
				break;
				
		case eSYS_PARA_WRITE:
				memset((uint8_t *)&info, 0, sizeof(smanuInfo));			
				rc = parse_args(&info);
				if(HAL_OK == rc)
				{
					rc = saveManuInfo(&info);
				}				
				break;
				
		case eSYS_PARA_READ:
				rc = loadManuInfo(&info);
				if(HAL_OK == rc)
				{
					if((0xFF == info.productId[0])&&(0xFF == info.devName[0]))
					{
						EuipDataSend("\nNULL\n");
					}
					else
					{
						EuipDataSend("\n%s,%s,%s\n", info.productId, info.devName, info.devSerc);
					}
					
				}
				
				break;
				
		case eFW_CHECK:			
				rc = FwVersion();
				break;
				
		default:
				EuipDataSend("+CME ERROR:%d\n", eCMD_ERR);
				rc = HAL_ERROR;
				break;
	}	
	ResetCmdBuf();

	return rc;
}
