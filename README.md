#LIBRERIA SENSOR COLOR TCS3200
###Proyecto Sistemas Embebidos


Esta librería pretende facilitar y aportar un mayor nivel de abstracción en la configuración y proceso de lectura
con el sensor TCS3200 para la familia de procesadores STM32F4 y más concretamente para la placa de desarrollo
Discovery F4. 

En la wiki se encuentra la documentación completa necesaria para usarla.
https://github.com/jaimelaborda/TCS3200_STM32F4_Library/wiki

Instrucciones: 

	1. Añadir los archivos .c y .h a la raiz del proyecto para STM32F4.
	2. Añadir los archivos en el proyecto mediante en el compilador.

Funciones de la librería:
	void Captura_TCS3200_Init(void); //Inicializa el TIM3 en modo captura -> LLamarla siempre en el main
	void TCS3200_Config(void); //Configura los pines de comunicación del sensor -> LLamarla siempre en el main
	void Set_Filter (uint8_t mode); //Mode es de tipo enum Color
	void Set_Scaling (uint8_t mode); //Mode es de tipo enum Scaling
	int GetColor(int set_color); //Funcion que Devulve RGB de color Rojo (0-255)
