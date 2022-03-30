#include "timers.h"

void TIMER0_Init(UINT32 t)
{
    // R8_TMR0_INTER_EN = RB_TMR_IE_CYC_END; // Interrupt Cycle End
    R8_TMR0_CTRL_MOD = RB_TMR_ALL_CLEAR;
    R8_TMR0_CTRL_MOD = RB_TMR_COUNT_EN;
    R32_TMR0_CNT_END = t; // Duration (SysTick)
}

void TIMER0_Disabe()
{
    R8_TMR0_CTRL_MOD = 0;
}