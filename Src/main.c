/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "stm32f1xx_hal.h"
#include "board.h"
#include "at_driver.h"
#include "peripheral_drv.h"
#include "stm32f10x_flash.h"
#include "version.h"
#include "equip.h"


#define  LOGO_POSX		(60)
#define  LOGO_POSY		(285)


/* extern function ---------------------------------------------------------*/
extern void exhibitor_shadow_task(void);
extern uint8_t sg_display_localname[];
extern smanuInfo g_DevInfo;


/* Private variables ---------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId mqttTaskHandle;
osThreadId equipTaskHandle;
osThreadId shadowTaskHandle;



/* Private function prototypes -----------------------------------------------*/
void StartDefaultTask(void const * argument);
void equipATtask(void const * argument);

#define  HAL_TCP_API_DEBUG_EABLE    (0)

#if HAL_TCP_API_DEBUG_EABLE
/*You can debug your hal api follow this example*/
void hal_tcp_api_test(void)
{
	int fd;
	char sendbuff[256] = "hello_world";
	char readbuff[256];
	
	printf("\n\rRegister network....");
	if(HAL_OK != NetInit())
	{
		printf("\n\rNet init fail");
		goto end;
	}
	printf("\n\rNet init ok");

	if(HAL_OK == net_connect_by_at("123.207.117.108",  2000, &fd,  eTCP, NULL))
	{
		printf("\n\rNet init ok, fd:%d",fd);		
	}
	else
	{
		printf("\n\rNet init err");
		goto end;
	}
	
	while(1)
	{
		if(HAL_ERROR == net_write_by_at(fd, sendbuff, strlen(sendbuff), 0xffff))
		{
			printf("\n\rnet_write_by_at err");
		}
		else
		{
			printf("\n\rnet_write_by_at ok");
		}

		HAL_Delay(5000);
		memset(readbuff, 0, 256);
		if(HAL_ERROR == net_read_by_at(fd, readbuff, 10, 0xffff))
		{
			printf("\n\rnet_read_by_at err");
		}
		else
		{
			readbuff[10]='\0';
			printf("\n\rnet_read_by_at [%s]",readbuff);
		}
		HAL_Delay(3000);
			
	}

end:

	printf("\n\rnet err,system reset");
	HAL_NVIC_SystemReset();
}
#endif


void display(void)//ÏÔÊ¾ÐÅÏ¢
{    	

	LCD_ShowString(0,10,240,24,24,"Tencent Cloud Summit");	
	memset(sg_display_localname, 0, MAX_NICKNAME_LEN);
	loadNameFromFlash(sg_display_localname);
	if(((0xff == sg_display_localname[0])&&(0xff == sg_display_localname[1]))
		||(0 == sg_display_localname[0]))
	{
		showName("Your name");	
	}
	else
	{
		showName(sg_display_localname);	
	}
	showimage(LOGO_POSX,LOGO_POSY);
}

void mem_info(void)
{
	printf("\n\rTotal_mem:%d freeMem:%d", configTOTAL_HEAP_SIZE, xPortGetFreeHeapSize());
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	BoardInit();
	printf("\n\r==================================================================================================");
	printf("\n\r======================Tencent IotSDK transplant example based on STM32F103+FreeRTOS===============");
	printf("\n\r======================FW Version:%s                       ================================", FW_VERSION_STR);
	printf("\n\r======================Build Time:%s %s            =================================",BUILDING_DATE, BUILDING_TIME);
	printf("\n\r==================================================================================================");
	printf("\n\rBoard init over");
	printf("\n\rSysclk[%d]", HAL_RCC_GetHCLKFreq());
	HwInitBc26();


	/*Init Lcd*/	
	LCD_Init();	
	display();
	printf("\n\rLcd init over");

	#if HAL_TCP_API_DEBUG_EABLE
	hal_tcp_api_test();
	#endif
	
	memset((uint8_t *)&g_DevInfo, 0, sizeof(smanuInfo));
	
	if(HAL_OK != loadManuInfo(&g_DevInfo))
	{
		showName("NO_SECRET");
		osThreadDef(equipTask, equipATtask, osPriorityNormal, 0, 512);
		equipTaskHandle = osThreadCreate(osThread(equipTask), NULL);
		printf("\n\rFactory mode");
	}
	else
	{
		printf("\n\rDevinfo product_id:%s,devName:%s,devSerc:%s",g_DevInfo.productId, g_DevInfo.devName, g_DevInfo.devSerc);
		osThreadDef(shadowTask, exhibitor_shadow_task, osPriorityAboveNormal, 1, 4096);
		shadowTaskHandle = osThreadCreate(osThread(shadowTask), NULL);
		printf("\n\rUser mode");
	}


	mem_info();
	
	
	/* Create the thread(s) */
	/* definition and creation of defaultTask */
//	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
//	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);	

	printf("\n\rstart os task...");
	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	while (1)
	{
		printf("\n\rSomething goes wrong!!!!");
		HAL_NVIC_SystemReset();
	}
  /* USER CODE END 3 */
}


/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  while (1)
  {
	  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_4);
	  osDelay(500);
  } 
}

void equipATtask(void const * argument)
{
	int rc;
	int sleepTime = 100;

	equip_menu_show();
	while(1)
	{
		if(true == atCmdCome())
		{	
			rc = EquipTestDeal();
			if(HAL_OK == rc)
			{
				EuipDataSend("OK\n\r");
			}
			else
			{
				EuipDataSend("ERROR\n\r");
			}	
		}

		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_4);
		osDelay(sleepTime);
	}

	
}


/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
