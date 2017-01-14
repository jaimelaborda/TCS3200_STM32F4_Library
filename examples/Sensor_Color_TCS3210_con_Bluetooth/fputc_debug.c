/**
   @file fputc_debug.c
   @brief Trying to redirect printf() to debug port
   @date 2012/06/25

*/

#include <stdio.h>
#include <stm32f4xx.h>

int fputc(int c, FILE *stream)
{
return (int)ITM_SendChar((uint32_t)c);
}


#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

struct __FILE { int handle; /* Add whatever needed */ };
FILE __stdout;
FILE __stdin;

int xfputc(int ch, FILE *f) {
  if (DEMCR & TRCENA) {
    while (ITM_Port32(0) == 0);
    ITM_Port8(0) = ch;
  }
  return(ch);
}


