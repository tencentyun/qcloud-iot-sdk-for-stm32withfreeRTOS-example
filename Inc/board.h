/*********************************************************************************
 *
 * File Name:    board.h
 *
 * Description:    board drivers for stm32F103 dev kit board
 *
 * History:      <author>          <time>        <version>
 *              yougaliu          2018-7-19        1.0
 * Desc:          
 ********************************************************************************/

#ifndef __BOARD_H_
#define __BOARD_H_

 
#define AT_UART_BAUDRATE			(115200)
#define DEBUG_UART_BAUDRATE			(115200)
#define BOARD_TYPE_CARD				(1)
#define BOARD_TYPE					(BOARD_TYPE_CARD)
#define RTOS_USED					(1)
#define EQUIP_UART_ENBALE			(1)

//typedef unsigned char  uint8_t;
//typedef unsigned short uint16_t;
//typedef unsigned short uint32_t;







void SystemClock_Config(void);
void BoardInit(void);



#endif

