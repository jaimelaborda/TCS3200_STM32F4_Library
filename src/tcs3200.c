/*-------------------------------------------------------------------------------*/
/*----------------------LIBRERIA SENSOR COLOR TCS3200----------------------------*/
/*------------------------Proyecto Sistemas Embebidos----------------------------*/
/*-----------------Diseñada y programada por Jaime Laborda-----------------------*/
/*------------------------------Diciembre de 2016--------------------------------*/
/*----------------------------------tcs3200.c------------------------------------*/
/*-------------------------------------------------------------------------------*/

#include <stdbool.h>
#include "stm32f4_discovery.h"
#include "delays_library.h"
#include "tcs3200.h"
#include "bluetooth.h"

 _Bool IC_ColorMode=false;
uint8_t calibrate_number;

uint16_t TimeColor_H=0, TimeColor_L=0;
uint16_t TimeColor;
int FreqColor;

void Captura_TCS3200_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* GPIOB clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	/*-------------------------- GPIO Configuration ----------------------------*/
	/* GPIOB Configuration: PB0 como entrada para captura */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/* Connect TIM4 pins to AF2 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_TIM3); //TIM3 CC3 -> PB0

	
	/*Activo Clock para el periferico del timer*/
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	/*Configuro la base de tiempos del timer*/
	
	TIM_TimeBaseInitStructure.TIM_Prescaler = 84; //Resolución de 0.001ms = 1us
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 19999;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = 0;
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	
	
	//CHANNEL 3 -> SUBIDA
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 5;
	
	TIM_ICInit(TIM3, &TIM_ICInitStructure);
	
	
	//Configuro interrupcion en el TIM3 CC3
	//TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE);
	
	TIM_Cmd(TIM3, ENABLE);	
	
	//Configurar interrupción del Channel 4 (BAJADA) del TIM3 -> NVIC
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	
	NVIC_Init(&NVIC_InitStructure);
}

void TCS3200_Config(void)
{
	//Configuración de los pines de entrada y salida para configurar el filtro y el preescaler
	//GPIOB
	//S0 -> GPIO_Pin_1
	//S1 -> GPIO_Pin_2
	//S2 -> GPIO_Pin_3
	//S3 -> GPIO_Pin_4
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* GPIOB clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	/*-------------------------- GPIO Configuration ----------------------------*/
	/* GPIOB Configuration: PB0 como entrada para captura */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Set_Filter (uint8_t mode) //Mode es de tipo enum Filtro
{
	switch (mode){
		case(Red):
			GPIO_ResetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_4);
			break;
		case(Blue):
			GPIO_ResetBits(GPIOB, GPIO_Pin_3);
			GPIO_SetBits(GPIOB, GPIO_Pin_4);
			break;
		case(Clear):
			GPIO_ResetBits(GPIOB, GPIO_Pin_4);
			GPIO_SetBits(GPIOB, GPIO_Pin_3);
			break;
		case(Green):
			GPIO_SetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_4);
			break;
	}
}

void Set_Scaling (uint8_t mode) //Mode es de tipo enum Filtro
{
	switch (mode){
		case(Scl0):
			GPIO_ResetBits(GPIOB, GPIO_Pin_1 | GPIO_Pin_2);
			break;
		case(Scl2):
			GPIO_ResetBits(GPIOB, GPIO_Pin_1);
			GPIO_SetBits(GPIOB, GPIO_Pin_2);
			break;
		case(Scl20):
			GPIO_ResetBits(GPIOB, GPIO_Pin_2);
			GPIO_SetBits(GPIOB, GPIO_Pin_1);
			break;
		case(Scl100):
			GPIO_SetBits(GPIOB, GPIO_Pin_1 | GPIO_Pin_2);
			break;
	}
}
	int Output_Color;
int GetColor(int set_color) //Funcion que Devuelve RGB de color Rojo (0-255)
{
	//char Output_Color;
	
	calibrate_number=0;

	TimeColor_H=0;
	TimeColor_L=0;
	TimeColor=0;
	
	
	TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE); //Enable interruption
	
	Set_Filter(set_color); //Set filter to Color
	DelayMillis(100);
	IC_ColorMode = true;
	
	while(IC_ColorMode == true); //Wait until value is get on the interrupt routine
	
	Set_Filter(Clear); //Set filter to default
	
	TIM_ITConfig(TIM3, TIM_IT_CC3, DISABLE); //Disable interruption
	
	/*Timing calculation -> Get Color Value*/
	if(TimeColor_H > TimeColor_L) //Avoid overflow
		TimeColor = TimeColor_H - TimeColor_L;
	else
		TimeColor = 0xFFFF - TimeColor_L + TimeColor_H;
	
	
	//FreqColor = SystemCoreClock/(TimeColor*84); //Frequency conversion by means of SystemCoreClock 
	FreqColor= SystemCoreClock/(TimeColor*168);
	
	//Freq to Color -> Depending of the filter
	switch (set_color){
		case Red:
			Output_Color = (255.0/(MAX_RED-MIN_RED))*(FreqColor-MIN_RED); //MAPEAR FUNCIÓN
			break;
		
		case Green:
			Output_Color = (255.0/(MAX_GREEN-MIN_GREEN))*(FreqColor-MIN_GREEN);  //MAPEAR FUNCIÓN
			break;
		
		case Blue: 
			Output_Color = (255.0/(MAX_RED-MIN_BLUE))*(FreqColor-MIN_BLUE);  //MAPEAR FUNCIÓN
			break;
	}
	
	//Constrain Value to MaxRange
	if (Output_Color > 255) Output_Color = 255;
	if (Output_Color < 0) Output_Color = 0;
	
	return Output_Color	; //Mapeo y retorno valor
}

void TIM3_IRQHandler(void)
{
	//Manejador de interrupción del TIM3 
	
	if (TIM_GetITStatus(TIM3,TIM_IT_CC3)!= RESET){
		//Clear flag is the first statement
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
		
		if((IC_ColorMode == true) && (calibrate_number == 0))
		{
			TimeColor_L = TIM_GetCapture3(TIM3);
			calibrate_number = 1;
		}
		else if((IC_ColorMode == true) && (calibrate_number == 1))
		{
			TimeColor_H = TIM_GetCapture3(TIM3);
			IC_ColorMode = false;
			calibrate_number = 0;
		}
	}
	
}
