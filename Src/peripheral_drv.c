/*********************************************************************************
 *
 * File Name:    peripheral_drv.c
 *
 * Description:    peripheral device drivers for stm32F103 dev kit board
 *
 * History:      <author>          <time>        <version>
 *               yougaliu          2018-12-05        1.0
 * Desc:           
 ********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "board.h"
#include "peripheral_drv.h"
#include "lcd.h"
#include "stm32f10x_flash.h"



/* Private variables ---------------------------------------------------------*/
static void NameClear(void)
{
	uint32_t index = 0;      
	uint32_t totalpoint = 32*240;

	LCD_SetCursor(0x00,NAME_POS_Y);	//设置光标位置 
	LCD_WriteRAM_Prepare();     //开始写入GRAM	 	
	
	for(index=0; index < totalpoint; index++)
	{
		LCD_WR_DATA(BK);
	}

}  

void showName(char *pName)
{
	uint8_t namelen;
	uint16_t xpos;

	/*cal start xpos*/
	namelen = strlen(pName);
	if(namelen < ONE_LINE_CHAR_NUM)
	{
		xpos = (ONE_LINE_CHAR_NUM - namelen)*8;
	}
	else
	{
		xpos = 0;
	}
	
    //clear name area
	NameClear();

	//show name	
	LCD_ShowString(xpos, NAME_POS_Y, 240, 32, 32, pName);
	

}


/*Use CubMX  Flash HAL driver data coulde not be written, so use a f103 flash driver actually, It's not a nice way.
*  if you find the reseason that api from hal_flash.c and hal_flash_ex.c  work not correctly, please tell me. just now I have no time to deal this problem
* 
*/
int  FLASH_WriteMultiByte(uint32_t startAddress,uint16_t *writeData,uint16_t countToWrite)
{
	int rc;				  
	uint16_t dataIndex;

	if((startAddress < FLASH_BASE_ADDR)||((startAddress + countToWrite*2) > FLASH_END_ADDR))
	{
		return HAL_ERROR;//非法地址
	}
	
	FLASH_UnlockBank1();         //解锁写保护

	 __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);
	
	rc = FLASH_ErasePage(startAddress);//擦除这个扇区	
	
	for(dataIndex = 0; dataIndex < countToWrite; dataIndex++)
	{
		rc |= FLASH_ProgramHalfWord(startAddress + dataIndex*2, writeData[dataIndex]);//写入数据
		
	}

	FLASH_LockBank1();//上锁写保护

	return rc;
}

int Flash_ReadMultiBtye(uint32_t ReadAddress, uint8_t *ReadBuf, int32_t ReadNum)
{   
    int DataNum = 0;


    while(DataNum < ReadNum)   
    {        
        ReadBuf[DataNum] = *(__IO uint8_t*) ReadAddress++;  
        DataNum++;     
    }

    return DataNum;    
}


int saveNameToFlash(uint8_t *pName)
{
	uint16_t nameBuff[MAX_NICKNAME_LEN];
	int rc;

	memset((uint8_t *)nameBuff, 0xff, MAX_NICKNAME_LEN);	
	memcpy((uint8_t *)nameBuff, pName, strlen(pName));	/*halfword字节对齐*/
	*((uint8_t *)((uint8_t *)nameBuff+strlen(pName))) = '\0';

	rc = FLASH_WriteMultiByte(NAME_FLASH_ADDR, nameBuff, MAX_NICKNAME_LEN/2);
	
	return (rc == FLASH_COMPLETE)? HAL_OK : HAL_ERROR;
	
}

int loadNameFromFlash(uint8_t *pName)
{
	return Flash_ReadMultiBtye(NAME_FLASH_ADDR, pName, MAX_NICKNAME_LEN);
}


int saveManuInfo(smanuInfo *info)
{
	int rc;
	
	rc = FLASH_WriteMultiByte(MANU_INFO_ADDR, (uint16_t *)info, sizeof(smanuInfo)/2);
	
	return HAL_OK;
}


/*Data would be blank if without factory write*/
int loadManuInfo(smanuInfo * pInfo)
{
	int rc;
	
	rc = Flash_ReadMultiBtye(MANU_INFO_ADDR, (uint8_t *)pInfo, sizeof(smanuInfo));
	if(sizeof(smanuInfo) == rc)
	{
		/*EXHIBIGTOR board name and secret len are fix, so ruel check len*/

		if(((0xFF == pInfo->devName[0])&&(0xFF == pInfo->devName[1]))
			||((0xFF == pInfo->devSerc[0])&&(0xFF == pInfo->devSerc[1]))
			||(EXHIBIGTOR_PRODUCT_ID_LEN != strlen(pInfo->productId))
			||(EXHIBIGTOR_DEV_NAME_LEN != strlen(pInfo->devName))
			||(EXHIBIGTOR_DEV_SECRET_LEN != strlen(pInfo->devSerc)))
		{
			rc = HAL_ERROR;
		}	
		else
		{
			rc = HAL_OK;
		}
	}

	return rc;
}

/***********************END OF FILE**************************/
