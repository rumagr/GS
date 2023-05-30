/**
 ******************************************************************************
 * @file    main.c
 * @author  TI Team
 *          HAW-Hamburg
 *          Labor für technische Informatik
 *          Berliner Tor  7
 *          D-20099 Hamburg
 * @version 1.1
 *
 * @date    17. Juli 2020
 * @brief   Rahmen fuer C Programme, die auf dem ITSboard ausgefuehrt werden.
 *
 *					25.04.2022 Tobias De Gasperis: printf hinzugefuegt
 ******************************************************************************
 */

#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "init.h"
#include "delay.h"
#include "LCD_GUI.h"
#include "LCD_Demos.h"
#include "lcd.h"
#include "fontsFLASH.h"
#include "LCD_Touch.h"
#include "error.h"
#include "one_wire.h"
#include "time.h"
#include "timer.h"
#include "gpio_basefunct.h"

#define ROM_0 0x1700000d8989a128
#define ROM_1 0x7300000d895c3228
#define ROM_2 0xf900000d87336828
#define ROM_3 0x800000d872ca428

#define TRUE 0x1
#define FALSE 0x0


void demoRom(void);
void demoTemp(void);
void getTemp(uint64_t rom, int16_t* vk, uint16_t* nk);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void){
	initITSboard();                 // Initialisierung des ITS Boards
	GUI_init(DEFAULT_BRIGHTNESS);   // Initialisierung des LCD Boards mit Touch
	TP_Init(false);                 // Initialisierung des LCD Boards mit Touch
	if (!checkVersionFlashFonts()) {
	    // Ueberpruefe Version der Fonts im Flash passt nicht zur Software Version
		Error_Handler();
	}
	
	//demoRom();
	while(true)
	{
		demoTemp();
		wait(500000);
	}
}


void demoRom(void)
{
	initTimer(); 
	init_GPIO(); 
	
	uint64_t rom = 0; 
	readRom(&rom);

	if(!checkCRC(rom,7))
	{
		Error_Handler(); 
	}
	
	for(uint32_t i = 0; i < 64; ++i)
	{
		uint64_t shift = (((uint64_t) 1) << i);
		uint64_t var = rom & shift; 
		
		if(var)
		{
			lcdPrintC('1');
		}
		else 
		{
			lcdPrintC('0'); 
		}
	}	
}
	

void demoTemp(void)
{
	initTimer(); 
	init_GPIO();
	
	int16_t vk0 = 0; 
	int16_t vk1 = 0; 
	int16_t vk2 = 0; 
	int16_t vk3 = 0;

	uint16_t nk0 = 0;
	uint16_t nk1 = 0; 
	uint16_t nk2 = 0; 
	uint16_t nk3 = 0; 	
	
	getTemp(ROM_0,&vk0, &nk0);
	getTemp(ROM_1,&vk1, &nk1);
	getTemp(ROM_2,&vk2, &nk2);
	getTemp(ROM_3,&vk3, &nk3);
	
	lcdGotoXY(0,1); 
	lcdPrintS("Temp0: "); 
	lcdPrintInt(vk0);
	lcdPrintC('.'); 
	if(nk0 < 1000)
	{
		lcdPrintC('0');
	}
	lcdPrintInt(nk0);
	
	lcdGotoXY(0,2); 
	lcdPrintS("Temp1: "); 
	lcdPrintInt(vk1);
	lcdPrintC('.'); 
	if(nk1 < 1000)
	{
		lcdPrintC('0');
	}
	lcdPrintInt(nk1);
	
	
	lcdGotoXY(0,3); 
	lcdPrintS("Temp2: "); 
	lcdPrintInt(vk2);
	lcdPrintC('.'); 
	if(nk2 < 1000)
	{
		lcdPrintC('0');
	}
	lcdPrintInt(nk2);
	
	lcdGotoXY(0,4); 
	lcdPrintS("Temp3: "); 
	lcdPrintInt(vk3);
	lcdPrintC('.'); 
	if(nk3 < 1000)
	{
		lcdPrintC('0');
	}
	lcdPrintInt(nk3);
	
}

void getTemp(uint64_t rom, int16_t* vk, uint16_t* nk)
{
	uint8_t temp[9] = {0};
	uint8_t negative = FALSE; 
	
	uint16_t res = 0;  
	
	readTemp(rom,temp);
	
	for(uint32_t i = 0; i < 2; ++i)
	{
		res |= ((int16_t) temp[i]) << (i*8); 
	}
	
	if(res & (0x1 << 15))
	{
		res = ~res; 
		res += 0x1;
			
		negative = TRUE; 
	}
	
	*vk = res >> 4;
	
	if(negative)
	{
		*vk = -*vk; 
	}
		
	if(res & (0x1 << 3))
	{
		*nk += 5000; 
	}
	if(res & (0x1 << 2))
	{
		*nk += 2500; 
	}
	if(res & (0x1 << 1))
	{
		*nk += 1250; 
	}
	if(res & (0x1 << 0))
	{
		*nk += 625; 
	}
	
	
}
	
// EOF
