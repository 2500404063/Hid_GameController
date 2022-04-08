#ifndef __TIMERS_H__
#define __TIMERS_H__

#include "CH579SFR.h"

#define TIMER0_Clock_us() ((double)R32_TMR0_COUNT / 32.0)
#define TIMER0_Clock_ms() ((double)R32_TMR0_COUNT / 32000.0)
#define TIMER0_Clock_s() ((double)R32_TMR0_COUNT / 32000000.0)
#define TIMERX_Duration_ms(x) 32 * 1000 * x

void TIMER0_Init(UINT32 t);
void TIMER0_Disabe();

#endif