/**
   @file main.c

   @brief Proyecto Sensor de Color TCS3200 con Bluetooth HC-05

   @author Jaime Laborda, Andrés Reverón, Rafa Oriol
   @date 2017/01/13
*/

// main.c 

#include "stm32f4_discovery.h"
#include "delays_library.h"
#include "tcs3200.h"
#include "bluetooth.h"
#include <stdio.h>
#include <stdbool.h>

#define VIRTUALIZAR_SENSOR 0

//FUNCIONES


int colorRed, colorGreen, colorBlue;

int main(void)
{

	Captura_TCS3200_Init();
	Delay_Init();
	TCS3200_Config();
	
	USART3_Init (9600);
	
	Set_Filter(Clear);
	Set_Scaling(Scl100);
	
	colorRed = 0;
	colorGreen = 0;
	colorBlue = 0;
	
	while(1){
		colorRed = GetColor(Red);
		colorGreen = GetColor(Green);
		colorBlue = GetColor(Blue);
		
		if(VIRTUALIZAR_SENSOR){
			colorRed = colorRed + 10;
			colorGreen = colorGreen + 5;
			colorBlue = colorBlue + 15;
			
			if(colorRed >= 255) colorRed = 0;
			if(colorGreen >= 255) colorGreen = 0;
			if(colorBlue >= 255) colorBlue = 0;
		}
		
		USART_SendData(USART3, colorRed);
		DelayMillis(10);
		USART_SendData(USART3, colorGreen);
		DelayMillis(10);
		USART_SendData(USART3, colorBlue);
		DelayMillis(10);
		
		printf("Color: %d", colorRed);
		
		
		DelayMillis(500);
	}
}
