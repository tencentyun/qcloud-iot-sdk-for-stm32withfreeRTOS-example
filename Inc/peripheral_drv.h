/*********************************************************************************
 *
 * File Name:    peripheral_drv.h
 *
 * Description:    peripheral device drivers for stm32F103 dev kit board
 *
 * History:      <author>          <time>        <version>
 *               yougaliu          2018-12-05        1.0
 * Desc:           ORG.
 ********************************************************************************/

#ifndef __PERIPHERAL_H_
#define __PERIPHERAL_H_
#include "stm32f1xx_hal.h"
#include "qcloud_iot_export.h"

#define ONE_LINE_CHAR_NUM			(240/16)
#define NAME_POS_X					(75)
#define NAME_POS_Y					(130)
#define DELTA_X						(240)
#define DELTA_Y						(80)
#define FLASH_BASE_ADDR				(0x08000000)
#define FLASH_END_ADDR				(0x0803FFFF)	
//#define FLASH_PAGE_SIZE    			((uint16_t)0x800) 				//页大小2k
#define NAME_FLASH_ADDR   			((uint32_t)(0x0803F000))		//第126页
#define MANU_INFO_ADDR				((uint32_t)(0x0803F800))		//制造信息
#define MAX_SIZE_OF_DEVICE_SERC  	 24
#define MAX_NICKNAME_LEN			 32

#define EXHIBIGTOR_PRODUCT_ID_LEN	(10)
#define EXHIBIGTOR_DEV_NAME_LEN		(12)
#define EXHIBIGTOR_DEV_SECRET_LEN	(24)


typedef struct _manuInfo_{
	char productId[MAX_SIZE_OF_PRODUCT_ID + 4];
	char devName[MAX_SIZE_OF_DEVICE_NAME + 4];
	char devSerc[MAX_SIZE_OF_DEVICE_SERC + 4];
}smanuInfo;


void showName(char *pName);
int saveManuInfo(smanuInfo *info);
int loadManuInfo(smanuInfo *info);


#endif

