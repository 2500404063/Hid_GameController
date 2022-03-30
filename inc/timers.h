#ifndef __TIMERS_H__
#define __TIMERS_H__

#include "CH579SFR.h"

#define TIMER0_Clock_us() (R32_TMR0_COUNT / 32.0f)
#define TIMER0_Clock_ms() (R32_TMR0_COUNT / 32000.0f)
#define TIMER0_Clock_s() (R32_TMR0_COUNT / 32000000.0f)
#define TIMERX_Duration_ms(x) 32 * 1000 * x

void TIMER0_Init(UINT32 t);
void TIMER0_Disabe();

#endif