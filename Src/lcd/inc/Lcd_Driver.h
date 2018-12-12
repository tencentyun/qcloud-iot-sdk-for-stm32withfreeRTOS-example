
#ifndef _LCD_DRIVER_H_
#define _LCD_DRIVER_H_

#include "main.h"
#include "stm32f1xx_hal.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#define  u8   			uint8_t
#define  u16				uint16_t
#define  delay_ms   HAL_Delay

/////////////////////////////////////用户配置区///////////////////////////////////	 
//支持横竖屏快速定义切换
#define USE_HORIZONTAL  		0	//定义是否使用横屏 		0,不使用.1,使用.

//-----------------------------SPI 总线配置--------------------------------------//
#define USE_HARDWARE_SPI     0  //1:Enable Hardware SPI;0:USE Soft SPI

//-------------------------屏幕物理像素设置--------------------------------------//
#define LCD_X_SIZE	        240
#define LCD_Y_SIZE	        320

#if USE_HORIZONTAL//如果定义了横屏 
#define X_MAX_PIXEL	        LCD_Y_SIZE
#define Y_MAX_PIXEL	        LCD_X_SIZE
#else
#define X_MAX_PIXEL	        LCD_X_SIZE
#define Y_MAX_PIXEL	        LCD_Y_SIZE
#endif
//////////////////////////////////////////////////////////////////////////////////
	 






#define RED  	  0xf800
#define GREEN	  0x07e0
#define BLUE 	  0x001f
#define WHITE	  0xffff
#define BLACK	  0x0000
#define YELLOW  0xFFE0
#define GRAY0   0xEF7D   	//灰色0 3165 00110 001011 00101
#define GRAY1   0x8410      	//灰色1      00000 000000 00000
#define GRAY2   0x4208      	//灰色2  1111111111011111



//本测试程序使用的是模拟SPI接口驱动
//可自由更改接口IO配置，使用任意最少4 IO即可完成本款液晶驱动显示
/******************************************************************************
接口定义在Lcd_Driver.h内定义，请根据接线修改并修改相应IO初始化LCD_GPIO_Init()
#define LCD_CTRL   	  	GPIOB		//定义TFT数据端口
#define LCD_LED        	GPIO_Pin_9  //PB9 连接至TFT -LED
#define LCD_RS         	GPIO_Pin_10	//PB10连接至TFT --RS
#define LCD_CS        	GPIO_Pin_11 //PB11 连接至TFT --CS
#define LCD_RST     	  GPIO_Pin_12	//PB12连接至TFT --RST
#define LCD_SCL        	GPIO_Pin_13	//PB13连接至TFT -- CLK
#define LCD_SDA        	GPIO_Pin_15	//PB15连接至TFT - SDI
*******************************************************************************/
#define  LCD_CS_CLR    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define  LCD_CS_SET		 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

#define  LCD_RS_CLR    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET)
#define  LCD_RS_SET		 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET)

#define	 LCD_RST_CLR   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET)
#define	 LCD_RST_SET   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET)



//#define LCD_CS_SET(x) LCD_CTRL->ODR=(LCD_CTRL->ODR&~LCD_CS)|(x ? LCD_CS:0)




//#define LCD_DATAOUT(x) LCD_DATA->ODR=x; //数据输出
//#define LCD_DATAIN     LCD_DATA->IDR;   //数据输入

//#define LCD_WR_DATA(data){\
//LCD_RS_SET;\
//LCD_CS_CLR;\
//LCD_DATAOUT(data);\
//LCD_WR_CLR;\
//LCD_WR_SET;\
//LCD_CS_SET;\
//} 



void LCD_GPIO_Init(void);
void Lcd_WriteIndex(u8 Index);
void Lcd_WriteData(u8 Data);
void Lcd_WriteReg(u8 Index,u8 Data);
u16 Lcd_ReadReg(u8 LCD_Reg);
void Lcd_Reset(void);
void Lcd_Init(void);
void Lcd_Clear(u16 Color);
void Lcd_SetXY(u16 x,u16 y);
void Gui_DrawPoint(u16 x,u16 y,u16 Data);
unsigned int Lcd_ReadPoint(u16 x,u16 y);
void Lcd_SetRegion(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);
void Lcd_WriteData_16Bit(u16 Data);

#endif
