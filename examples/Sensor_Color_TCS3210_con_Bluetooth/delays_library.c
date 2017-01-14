#include "delays_library.h"

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

/*System clock  definition*/
#define SystemFreq 180000000 //168MHz

uint32_t multiplier;

void Delay_Init(void) {
    
    /* While loop takes 4 cycles */
    /* For 1 us delay, we need to divide with 4M */
    multiplier = SystemFreq / 4000000;
}

void DelayMicros(uint32_t micros) {
    /* Multiply micros with multipler */
    /* Substract 10 */
    micros = micros * multiplier - 10;
    /* 4 cycles for one loop */
    while (micros--);
}

void DelayMillis(uint32_t millis) {
    /* Multiply millis with multipler */
    /* Substract 10 */
    millis = 1000 * millis * multiplier - 10;
    /* 4 cycles for one loop */
    while (millis--);
}
