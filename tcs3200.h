/**
   @file tcs3200.h

   @brief Librería Sensor de color TCS3200 

   @author Jaime Laborda, Andrés Reverón, Rafa Oriol
   @date 2017/01/13
*/
/*-------------------------------------------------------------------------------*/
/*----------------------LIBRERIA SENSOR COLOR TCS3200----------------------------*/
/*------------------------Proyecto Sistemas Embebidos----------------------------*/
/*-----------------Diseñada y programada por Jaime Laborda-----------------------*/
/*------------------------------Diciembre de 2016--------------------------------*/
/*----------------------------------tcs3200.h------------------------------------*/
/*-------------------------------------------------------------------------------*/

#include <stdbool.h>

/*#define MIN_RED 6000.0
#define MAX_RED 35000.0
#define MIN_GREEN 10000.0
#define MAX_GREEN 56000.0
#define MIN_BLUE 10000.0
#define MAX_BLUE 35000.0*/

#define MIN_RED 5000.0
#define MAX_RED 16400.0
#define MIN_GREEN 7000.0
#define MAX_GREEN 11000.0
#define MIN_BLUE 6000.0
#define MAX_BLUE 10000.0

enum Colors{Red, Blue, Clear, Green};
enum Scaling{Scl0, Scl2, Scl20, Scl100};

//PUBLIC FUNCTIONS
void Captura_TCS3200_Init(void);
void TCS3200_Config(void);
void Set_Filter (uint8_t mode); //Mode es de tipo enum Color
void Set_Scaling (uint8_t mode); //Mode es de tipo enum Scaling
int GetColor(int set_color); //Funcion que Devulve RGB de color Rojo (0-255)

void TIM3_IRQHandler(void); //Manejador de interrupción


