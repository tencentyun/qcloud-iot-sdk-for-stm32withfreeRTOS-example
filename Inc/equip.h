/**
  ******************************************************************************
  * @file	equip.h
  * @author	yougaliu	
  * @date	2018-12-06
  ******************************************************************************
  */

#ifndef _EUIP_H_
#define _EUIP_H_
#include "board.h"
#include "stdint.h"



#if (BOARD_TYPE == BOARD_TYPE_CARD)
	#define EQUIP_UART_IRQ                  USART3_IRQn
	#define EQUIP_UART_IRQHandler           USART3_IRQHandler	
#else
	#error unknow board type
#endif

#ifndef __INT_MAX__
#define __INT_MAX__     2147483647
#endif
#define INT_MAX         (__INT_MAX__)

#define EQUIP_RECIVE_MAX_BUF_LEN  		(256)
#define TOTAL_CMD_NUM			  		(4)
#define AT_CMD_CR                      '\r'
#define AT_CMD_LF                      '\n'
#define AT_CMD_DQUOTES_MARK            '"'




typedef enum equip_recv_status
{	
	eRECV_ABLE = 0,
	eREADY_FOR_DEAL,	
	eFULL,
}eEuipUartStatus;


typedef struct equip_recv_buf_struct
{
    uint8_t  	 recv_buf[EQUIP_RECIVE_MAX_BUF_LEN + 1];     
	uint16_t	 pos;
	eEuipUartStatus 	 status;
}sEquipRecvData;

typedef enum
{
	eAT_TEST=0,	
    eSYS_PARA_WRITE,
    eSYS_PARA_READ,
    eFW_CHECK,
    eDefault,
} eCmdType;

typedef struct  _sEuipCmd
{
	uint8_t at_buf[20];
	eCmdType type;
}sEuipCmd;


typedef enum _eCmd_Err_Code_{

	ePARA_ERR = 200,		/*para error*/
	eMEM_ERR = 201,			/*memory error*/
	eNET_ERR = 202,			/*no register network*/
	eGET_IP_ERR = 203,		/*get ip addr error*/
	eAUTH_ERR = 204,		/*auth error*/
	eDEALING_ERR = 205,		/*in progress*/
	eDEAL_ERR = 206,		/*progress error*/
	eOVER_FLOW_ERR = 207,	/*msg packet over size*/
	eCMD_ERR = 208,			/*not support*/
	eUNKNOW_ERR = 209,		/*unknown error*/
	eTIME_OUT_ERR = 210,	/*input timeout*/
}eCmd_Err_Code;


int EquipTestDeal(void);
bool atCmdCome(void);
void EuipDataSend(const char* fmt,...);



#endif // _LOG_H_
