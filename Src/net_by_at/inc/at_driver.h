#ifndef __GSM_DRV_H_
#define __GSM_DRV_H_

#include "board.h"


#define AT_MAX_LEN					(128)
#define ACK_MAX_LEN					(128)
#define AT_RECIVE_MAX_BUF_LEN		(1500)

#define TIMOUT_1S					(1000)
#define TIMOUT_2S					(2000)
#define TIMOUT_3S					(3000)
#define TIMOUT_4S					(4000)
#define TIMOUT_5S					(5000)

#if (BOARD_TYPE == BOARD_TYPE_CARD)
	#define AT_UART_IRQ                  USART2_IRQn
	#define AT_UART_IRQHandler           USART2_IRQHandler	
#else
	#error unknow board type
#endif



//#define AT_INFO_DEBUG_ENABLE

#ifdef AT_INFO_DEBUG_ENABLE
#define AT_INFO_DEBUG(args...)\
	do {\
	   printf(##args); \
   }while(0)

#else
#define AT_INFO_DEBUG(fmt, args...) HAL_Delay(20)
//#define AT_INFO_DEBUG(fmt, args...)
#endif

typedef enum at_recv_status
{	
	eBUFF_RECV_ABLE = 0,
	eBUFF_READY_FOR_DEAL,	
	eBUFF_FULL,
}eBufStatus;

typedef struct at_cmd_struct
{
    uint8_t  at_buf[AT_MAX_LEN];   
    uint8_t  ack_patten[ACK_MAX_LEN];
	uint16_t timeOut;
	void *	 pAckBuf;
}sAtCmd;


typedef struct at_recv_buf_struct
{
    uint8_t  	 recv_buf[AT_RECIVE_MAX_BUF_LEN + 1];     
	uint16_t	 pos;
	eBufStatus 	 status;
}sAtRecvData;

typedef enum
{
	eTCP = 6,
	eUDP = 17,	
}eNetProto;


void init_send_cmd(sAtCmd *pCmd,  const char *atStr, const char * ackPatten, uint16_t timeOut);
HAL_StatusTypeDef at_send_data(uint8_t *pdata, uint16_t len);
HAL_StatusTypeDef exec_gsm_at(sAtCmd *pCmd);
HAL_StatusTypeDef WaitForAck(uint8_t * pAckpatten,  uint16_t timeOut);
void FlushDataBuff(void);
uint8_t *GetDataBuff(void);



#endif

