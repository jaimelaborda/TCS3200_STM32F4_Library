# TCS3200_STM32F4_Library
A library for the TCS3200 Color to frequency sensor for the STM32F4 ARM core. Includes functions to read the RGB colors.

h1 __Sensor Color-to-frequency TCS3200/TCS3210__

h1 __Indice__
# [Introducción|#Introducción]\\
# [Características técnicas|#Características técnicas]\\
# [Diagrama de Bloques|#3_Diagrama de bloques]\\
# [PinOut del TCS3200|#4_Pinout del TCS32000]
# [PinOut del modulo integrado|#5_Pinout del modulo integrado]\\
# [Configuración Inicial|#6_Configuracion inicial]\\
# [Configuración de escala y filtro color|#7_Configuracion escala y filtro]\\
# [Lectura de color|#8_Lectura de color]\\
# [Librería TCS3200 para STM32F4|#10_Librería TCS3200 para STM32F4]\\
# [Calibración del sensor|#11_Calibración del sensor]\\
# [Video de ejemplo|#12_Video de Ejemplo]\\
# [Bibliografía|#13_Bibliografia]\\

{anchor:Introducción}
h2 __1. Introducción__
{anchor}

En esta página vamos a hablar de cómo configurar e implementar el uso de un sensor que traduce colores emitidos por luces LED a frecuencia, detectando así de qué color se trata.

El sensor de color con integrado TCS3200 puede filtrar los datos RGB de la fuente de luz y la convierten en una onda cuadrada (50% ciclo de trabajo) con una frecuencia directamente proporcional a la intensidad de la luz de radiación. La frecuencia de salida se puede escalar por uno de los tres valores preestablecidos a través de dos pines de control de entrada SO y S1, con opciones seleccionables del 2%, 20% y 100% de frecuencia; los pines S2 y S3 sirven para controlar el filtro de RGB.

Los pines de entradas y salida permiten ser conectados directamente a un microcontrolador o circuito lógico.

Para facilitar el trabajo con el sensor, disponemos de un módulo integrado que incluye al sensor junto a 4 LED's blancos para iluminar la superficie a escanear y los correspondientes pines de entrada y salida accesibles.

{image:worksite:/CarpetaPública/Sensor de color TCS3200/ModuloIntegrado.PNG|Módulo integrado}

{anchor:Características técnicas}
h2 __2. Características técnicas__
{anchor}

* Escala la frecuencia de salida
* Opera desde una sola fuente de alimentación de 2.7 a 5.5V
* Pin de apagado de funciones
* Error no lineal de 0.2% a 50 kHz
* Alta Resolución de conversión de la intensidad de luz a frecuencia
* Modo de bajo consumo de energía
* LED's incluidos en el PCB para iluminar el objeto a reconocer
* Se comunica directamente con un microcontrolador (PIC, Arduino, etc.)
* Frecuencia de salida a fondo de escala y color programables
* Coeficiente de temperatura estable 200 ppm/°C 
* Para más información, consultar la datasheet en el siguiente enlace: {link:Datasheet TCS3200|worksite:/CarpetaPública/Sensor de color TCS3200/Datasheet TCS3200.pdf}

{anchor: 3_Diagrama de bloques}
h2 __3. Diagrama de bloques__
{anchor}

{image:worksite:/CarpetaPública/Sensor de color TCS3200/Diagrama_bloques_TCS3200.PNG|Diagrama de bloques TCS3200}

{anchor: 4_Pinout del TCS32000}
h2 __4. PinOut del TCS3200__
{anchor}

A continuación mostramos la configuración física del chip:

{image:worksite:/CarpetaPública/Sensor de color TCS3200/Color_Sensor_1.jpg | PinOut TCS3200}

{image:worksite:/CarpetaPública/Sensor de color TCS3200/chip.jpg | Chip TCS3200}

Como se puede observar, en el TCS3200, el conversor de luz a frecuencia lee una matriz de 8x8 fotodiodos. 16 fotodiodos tienen filtros azules, 16 tienen filtros verdes, 16 tienen filtros rojos y 16 son transparentes sin ningún filtro.

Del mismo modo, existe una versión más reducida del chip que es la que utilizaremos nosotros, el TCS3210, cuya matriz de fotodiodos es de 4x6, teniendo 6 fotodiodos de cada tipo, en lugar de los 16 anteriores.

Todos los fotodiodos del mismo tipo de filtro están conectados en paralelo.

Configurando los pines S2 y S3 configuraremos que fotodiodos están activos en cada momento.

{anchor: 5_Pinout del modulo integrado}
h2 __5. PinOut del módulo integrado__
{anchor}

{image:worksite:/CarpetaPública/Sensor de color TCS3200/TCS3200 module diagram.png| PinOut Módulo}


{anchor: 6_Configuracion inicial}
h2 __6. Configuración inicial__
{anchor}

Para empezar a trabajar con el módulo integrado necesitaremos configurar varios pines GPIO para las entradas y salidas, un timer en modo captura y una interrupción para capturar y medir el ancho del pulso, que posteriormente se traduce a frecuencia y, finalmente, a color.

En primer lugar, creamos una función para configurar los pines de salida que utilizaremos para configurar el sensor (S0, S1, S2, S3). Estos pines se configurarán como salidas e irán conectados al puerto B como indica la tabla siguiente:

{image:worksite:/CarpetaPública/Sensor de color TCS3200/tabla_pines_stm.PNG| Table pines STM32F4}  

La función es la siguiente:

{code:}
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
{code}

Adicionalmente, se crea una función que configure el Timer 3, este es el [Timer|Timers] que utilizaremos, dado que es un Timer de propósito general de 16bits:

{code}
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
	
	TIM_TimeBaseInitStructure.TIM_Prescaler = 84; //Resolución de 1us
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
	TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE);
	
	TIM_Cmd(TIM3, ENABLE);	
	
	//Configurar interrupción del Channel 4 (BAJADA) del TIM3 -> NVIC
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	
	NVIC_Init(&NVIC_InitStructure);
}
{code}

En él, configuramos un canal del Timer 3 (Channel 3) y lo conectamos a la ~~Alternative Function~~ del GPIO PB0.
Además, se configura una [interrupción|Interrupciones] del periférico Timer 3 habilitando los registros pertinentes en el [NVIC|NVIC].

En cuanto a la base de tiempos escogida, hemos elegido cuentas de 1us dado que nos da una resolución adecuada para la tarea que vamos a desarrollar(1MHz).

//maximo?

{anchor: 7_Configuracion escala y filtro}
h2 __7. Configuración de escala y filtro color__
{anchor}

Antes de leer la frecuencia de salida del sensor debemos de configurarlo mediante los pines S0, S1, S2 y S3. Los primeros (S0 y S1) configuran el factor de escala por el que multiplica la frecuencia de salida, mientras que el S2 y S3 configuran el filtro de color que se aplica a la matriz de fototransistores, y con ello, el color que se va a leer.

Para obtener un mayor nivel de abstracción a la hora de usar el sensor, se han creado dos funciones que escriben en los pines del sensor y lo configuran de acuerdo a la tabla adjunta.

{image:worksite:/CarpetaPública/Sensor de color TCS3200/modes_and_filter.PNG| PinOut Módulo}

La función ~~void Set_Scaling (uint8_t mode)~~ configura el factor de escala escribiendo en los pines S0 y S1 como se observa en la implementación de la función:


{code}
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
{code}

Se ha declarado un tipo ~~enum~~ que contiene la definición de los diferentes modos que le podemos pasar a la función:
{code}
enum Scaling{Scl0, Scl2, Scl20, Scl100};
{code}

Para la función de configuración del filtro se ha realizado una función similar:

{code}
enum Filter{Red, Blue, Clear, Green};

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
{code}

{anchor: 8_Lectura de color}
h2 __8. Lectura de Color__
{anchor}

Para facilitar la lectura de color mediante interrupción, hemos preparado una función de lectura para cada color. Dicha función configura el filtro necesario, habilita la interrupción del timer, espera a que la frecuencia sea leída y por último, hace la conversión de frecuencia a código RGB.

{code}
int GetColor(int set_color) //Funcion que Devulve RGB de color Rojo (0-255)
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
	
	return Output_Color	; //retorno valor
}
{code}

El valor de frecuencia es leído mediante el manejador de interrupción del TIM3 con la siguiente función.

{code}
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

{code}
Dicha función es ejecutada cada vez que se lee un flanco de subida en el pin PB0. La función se encarga de registrar el instante de tiempo en el que se produce cada uno de los dos flancos y los almacena en las variables TimeColor_L y TimeColor_H, para luego calcular la diferencia y con ello el periodo, para después ser traducido en la función GetColor a frecuencia y posteriormente a código RGB.

{anchor: 10_Librería TCS3200 para STM32F4}
h2 __10. Librería TCS3200 para STM32F4__
{anchor}
Para facilitar la lectura del sensor TCS3200 se ha creado una librería que encierra todas las funciones descritas a continuación, dicha librería aporta un mayor nivel de abstracción a la hora de realizar cualquiera diseño de sistema embebido donde se requiera el uso de dicho sensor y permite al desarollador centrarse en el problema principal a resolver.

La librería puede ser descargada desde el siguiente enlace: {link: Librería TCS3200 para STM32F4| https://poliformat.upv.es/access/content/group/GRA_12165_2016/CarpetaP%C3%BAblica/Sensor%20de%20color%20TCS3200/tcs3200_library.zip}

Estás son las funciones de las que dispone la librería:
{code}
//PUBLIC FUNCTIONS
void Captura_TCS3200_Init(void); //Configura el TIM3 (PB0) como modo captura para que salte la interrupción
void TCS3200_Config(void); //Configura los puertos GPIO para configurar los parametros del sensor (Pines predefinidos en tcs3200.h)
void Set_Filter (uint8_t mode); //Set color filter. Mode es de tipo enum Color: Red, Green, Blue, Clear
void Set_Scaling (uint8_t mode); //Mode es de tipo enum Scaling: Scl10, Scl2, Scl20, Scl100
int GetColor(int set_color); //Funcion que Devuelve RGB del color que le pasemos - set_color debe ser Red, Green o Blue

void TIM3_IRQHandler(void); //Manejador de interrupción del TIM
{code}

{anchor: 11_Calibración del sensor}
h2 __11. Calibración del sensor__
{anchor}

El sensor es difícil de calibrar y está muy influenciado por la luz ambiente y otros parámetros difíciles de controlar,
por lo que a veces las lecturas que realiza son erróneas.

La calibración del sensor se realiza cambiando las frecuencias máximas y mínimas para cada filtro de color en los #defines de la cabecera de la librería. 
{code}
#define MIN_RED 5000.0
#define MAX_RED 16400.0
#define MIN_GREEN 7000.0
#define MAX_GREEN 11000.0
#define MIN_BLUE 6000.0
#define MAX_BLUE 10000.0
{code}

El proceso de calibración se realiza siguiendo los siguientes pasos para cada color:

# Leer el valor de un solo color mediante la función "GetColor(color)".
# Posicionar el sensor en una superficie negra y leer el valor de frecuencia de salida.
# Coger este valor como MIN para el valor de color requerido.
# Repetir el proceso en una superficie blanca y homogénea cogiendo el valor como MAX.

Los valores claros corresponden a frecuencias mayores que los valores oscuros. Cogiendo los dos extremos de color conseguimos, teóricamente, calibrar correctamente el color.

Para facilitar la calibración se puede imprimir (en color) la siguiente hoja de calibración: \\
{link: Hoja de calibración 1|https://poliformat.upv.es/access/content/group/GRA_12165_2016/CarpetaP%C3%BAblica/Sensor%20de%20color%20TCS3200/plantilla%20calibracion%20color_0.pdf}\\
{link: Hola de calibración 2|https://poliformat.upv.es/access/content/group/GRA_12165_2016/CarpetaP%C3%BAblica/Sensor%20de%20color%20TCS3200/plantilla%20calibracion%20color_1.pdf}

{anchor: 12_Video de Ejemplo}
h2 __12. Video de Ejemplo__
{anchor}

Un video de una aplicación a modo de ejemplo de funcionamiento puede ser consultado: {link: Sensor TCS3200 con STM32F4 y Bluetooth HC-05| https://www.youtube.com/watch?v=qfZFWDSn4_A}



{anchor: 13_Bibliografia}
h2 __13. Bibliografía__
{anchor}

