/*bluetooth.c*/
#include "bluetooth.h"
#include "stm32f4_discovery.h"

void USART3_Init (uint16_t baudrate)
{
     
        //-----Declarar ESTRUCTURAS-----     
        GPIO_InitTypeDef  GPIO_InitStruct;
        USART_InitTypeDef USART_InitStruct;
		NVIC_InitTypeDef NVIC_InitStruct;
   
     
     
        //-----DAR RELOJ A LOS PERIFÉRICOS-----     
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD , ENABLE);
 
     
        //-----CONFIGURACION DE LOS PINES DEL CANAL 8 Y 9 PARA LA USART-----     
        GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9; // Pins 8 (TX) and 9 (RX) are used
        GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;            // the pins are configured as alternate function so the USART peripheral has access to them
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;       // this defines the IO speed and has nothing to do with the baudrate!
        GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;          // this defines the output type as push pull mode (as opposed to open drain)
        GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;           // this activates the pullup resistors on the IO pins
     
        GPIO_Init(GPIOD, &GPIO_InitStruct);  
 
 
        //-----CONECTAR PD Y PD9 A LOS PUERTOS USART TX Y RX-----     
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3); //
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);
 
 
        //-----CONFIGURACION DE LA USART-----     
        USART_InitStruct.USART_BaudRate            = baudrate;            // the baudrate is set to the value we passed into this init function
        USART_InitStruct.USART_WordLength          = USART_WordLength_8b; // we want the data frame size to be 8 bits (standard)
        USART_InitStruct.USART_StopBits            = USART_StopBits_1;    // we want 1 stop bit (standard)
        USART_InitStruct.USART_Parity              = USART_Parity_No;     // we don't want a parity bit (standard)
        USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
        USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;      // we want to enable the transmitter and the receiver
     
        USART_Init(USART3, &USART_InitStruct);                            // again all the properties are passed to the USART_Init function which takes care of all the bit setting
				
		//Configurando interrupción para RX
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
		//Configurando interrupción en NVIC
		NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		//Inicializando NVIC
		NVIC_Init(&NVIC_InitStruct);
 
        //-----HABILITAR USART-----    
        USART_Cmd(USART3, ENABLE);
     
}
