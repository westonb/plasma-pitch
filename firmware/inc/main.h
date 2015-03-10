// Define to prevent recursive inclusion -------------------------------------
#ifndef __MAIN_H
#define __MAIN_H

//exported functions
#include "stm32f30x.h"

void TimingDelay_Decrement(void);
void Delay(volatile uint32_t nTime);


#endif /* __MAIN_H */

