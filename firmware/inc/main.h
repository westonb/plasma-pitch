// Define to prevent recursive inclusion -------------------------------------
#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f30x.h"

#define PERIOD_6_78MHZ 320
#define BUCK_PWM_PERIOD 10000
#define DT_FALLING 30 //*250ps
#define DT_RISING 30
#define MOD_DC_OFFSET 1500
#define MOD_UPDATE_RATE 1

//exported functions
void TimingDelay_Decrement(void);
void Delay(volatile uint32_t nTime);
extern uint32_t output_current, output_voltage, supply_voltage, audio_voltage;

#endif // __MAIN_H 

