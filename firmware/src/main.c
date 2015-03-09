
#include "stm32f30x.h"



/*
system setup:
HSE = 13.56MHz
SYSCLK = 67.8MHz
APB2 = 67.8MHz
APB1 = 33.9MHz

Pins:
LCD_D/C: PB0
Button_B: PB3
Button_A: PB4
LED_1: PB6
LED_2: PB7

CURRENT_ADC: PA0
OUTPUT_ADC: PA1
SUPPLY_ADC: PA2
AUDIO_ADC: PA3

LCD_CS!: PA4
SPI_SCK: PA5
SPI_MOSI: PA7
MOD_A: PA8  HRTIM1_CHA1
MOD_B: PA9  HRTIM1_CHA2
RF_A: PA10  HRTIM1_CHB1
RF_B: PA11  HRTIM1_CHB2
*/

#define PERIOD_6_78MHZ 320
#define BUCK_PWM_PERIOD 10000
#define DT_FALLING 30 //*250ps
#define DT_RISING 30

//function prototypes: 
void led_on();
void led_off();
void GPIO_config();

void RF_config();
void RF_enable();
void RF_disable(); 
void SMPS_config();

int main(){
	GPIO_config();
	led_on();
	RF_config();
	
	SMPS_config();
	RF_enable();
	
	int i = 0;
	while(1){
		//code goes here
		i++;
		//led_on();
		//led_off();

	}


	return 0;
}

//
//MOD_A: PA8  HRTIM1_CHA1
//MOD_B: PA9  HRTIM1_CHA2
void SMPS_config(){

	HRTIM_OutputCfgTypeDef HRTIM_TIM_OutputStructure;
	HRTIM_BaseInitTypeDef HRTIM_BaseInitStructure;
	HRTIM_TimerInitTypeDef HRTIM_TimerInitStructure;  
	HRTIM_TimerCfgTypeDef HRTIM_TimerWaveStructure;
	HRTIM_CompareCfgTypeDef HRTIM_CompareStructure;
	HRTIM_DeadTimeCfgTypeDef HRTIM_TIM_DeadTimeStructure;
	HRTIM_ADCTriggerCfgTypeDef HRTIM_ADCTrigStructure;   
	NVIC_InitTypeDef    NVIC_InitStructure;


	/* --------------------------------------------------- */
	/* TIMERA initialization: timer mode and PWM frequency */
	/* --------------------------------------------------- */
	//intilize timer
	HRTIM_TimerInitStructure.HalfModeEnable = HRTIM_HALFMODE_DISABLED;
	HRTIM_TimerInitStructure.StartOnSync = HRTIM_SYNCSTART_DISABLED;
	HRTIM_TimerInitStructure.ResetOnSync = HRTIM_SYNCRESET_DISABLED;
	HRTIM_TimerInitStructure.DACSynchro = HRTIM_DACSYNC_NONE;
	HRTIM_TimerInitStructure.PreloadEnable = HRTIM_PRELOAD_ENABLED;
	HRTIM_TimerInitStructure.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;
	HRTIM_TimerInitStructure.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
	HRTIM_TimerInitStructure.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;

	//sets counter for timer
	HRTIM_BaseInitStructure.Period = BUCK_PWM_PERIOD; /* 400kHz switching frequency */
	HRTIM_BaseInitStructure.RepetitionCounter = 127;   /* 1 ISR every 128 PWM periods */
	HRTIM_BaseInitStructure.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL32;
	HRTIM_BaseInitStructure.Mode = HRTIM_MODE_CONTINOUS;          

	HRTIM_Waveform_Init(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, &HRTIM_BaseInitStructure, &HRTIM_TimerInitStructure);


	/* ------------------------------------------------ */
	/* TIMERA output and registers update configuration */
	/* ------------------------------------------------ */
	//sets more timer things
	HRTIM_TimerWaveStructure.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_ENABLED;
	HRTIM_TimerWaveStructure.DelayedProtectionMode = HRTIM_TIMDELAYEDPROTECTION_DISABLED;
	HRTIM_TimerWaveStructure.FaultEnable = HRTIM_TIMFAULTENABLE_NONE;//no stopping this
	HRTIM_TimerWaveStructure.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;
	HRTIM_TimerWaveStructure.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;
	HRTIM_TimerWaveStructure.ResetTrigger = HRTIM_TIMRESETTRIGGER_NONE;
	HRTIM_TimerWaveStructure.ResetUpdate = HRTIM_TIMUPDATEONRESET_DISABLED;
	HRTIM_TimerWaveStructure.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_NONE;
	HRTIM_WaveformTimerConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, &HRTIM_TimerWaveStructure);


	/* -------------------------------- */
	/* TA1 and TA2 waveform description */
	/* -------------------------------- */
	/* PWM on TA1 */
	//set up outputs TA1 and TA2
	
	
	HRTIM_TIM_OutputStructure.Polarity = HRTIM_OUTPUTPOLARITY_HIGH; 
	HRTIM_TIM_OutputStructure.SetSource = HRTIM_OUTPUTSET_TIMPER;  
	HRTIM_TIM_OutputStructure.ResetSource = HRTIM_OUTPUTRESET_TIMCMP1; 
	HRTIM_TIM_OutputStructure.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;  
	HRTIM_TIM_OutputStructure.IdleState = HRTIM_OUTPUTIDLESTATE_INACTIVE;          
	HRTIM_TIM_OutputStructure.FaultState = HRTIM_OUTPUTFAULTSTATE_INACTIVE;          
	HRTIM_TIM_OutputStructure.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;        
	HRTIM_TIM_OutputStructure.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;
	HRTIM_WaveformOutputConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, &HRTIM_TIM_OutputStructure);
	HRTIM_WaveformOutputConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA2, &HRTIM_TIM_OutputStructure);
	

	/* Set compare registers for duty cycle on TA1 */
	//this sets the compare for the switching of TA1/TA2
	HRTIM_CompareStructure.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;
	HRTIM_CompareStructure.AutoDelayedTimeout = 0;
	HRTIM_CompareStructure.CompareValue = BUCK_PWM_PERIOD/5;     //starting value
	HRTIM_WaveformCompareConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, &HRTIM_CompareStructure);

	//this sets up the deadtime value 
	HRTIM_TIM_DeadTimeStructure.FallingLock = HRTIM_TIMDEADTIME_FALLINGLOCK_WRITE;
	HRTIM_TIM_DeadTimeStructure.FallingSign = HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE;
	HRTIM_TIM_DeadTimeStructure.FallingSignLock = HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_WRITE;
	HRTIM_TIM_DeadTimeStructure.FallingValue = DT_FALLING;
	HRTIM_TIM_DeadTimeStructure.Prescaler = 0x0;
	HRTIM_TIM_DeadTimeStructure.RisingLock = HRTIM_TIMDEADTIME_RISINGLOCK_WRITE;
	HRTIM_TIM_DeadTimeStructure.RisingSign = HRTIM_TIMDEADTIME_RISINGSIGN_POSITIVE;
	HRTIM_TIM_DeadTimeStructure.RisingSignLock = HRTIM_TIMDEADTIME_RISINGSIGNLOCK_WRITE;
	HRTIM_TIM_DeadTimeStructure.RisingValue = DT_RISING;
	HRTIM_DeadTimeConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, &HRTIM_TIM_DeadTimeStructure);

	
	/* --------------------------*/
	/* ADC trigger initialization */
	/* --------------------------*/
	/* Set compare 3 registers for ADC trigger */
	
	HRTIM_CompareStructure.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;
	HRTIM_CompareStructure.AutoDelayedTimeout = 0;
	HRTIM_CompareStructure.CompareValue = BUCK_PWM_PERIOD/10;  //samples in middle of duty cycle 
	HRTIM_WaveformCompareConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2, &HRTIM_CompareStructure);

	HRTIM_ADCTrigStructure.Trigger = HRTIM_ADCTRIGGEREVENT24_TIMERA_CMP2;
	HRTIM_ADCTrigStructure.UpdateSource = HRTIM_ADCTRIGGERUPDATE_TIMER_D;
	HRTIM_ADCTriggerConfig(HRTIM1, HRTIM_ADCTRIGGER_2, &HRTIM_ADCTrigStructure);
	

	
	/*   */
	/* -------------------------*/
	/* Interrupt initialization */
	/* -------------------------*/
	/* Configure and enable HRTIM TIMERA interrupt channel in NVIC */
	NVIC_InitStructure.NVIC_IRQChannel = HRTIM1_TIMA_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIMER A issues an interrupt on each repetition event */
	 HRTIM_ITConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_TIM_IT_REP, ENABLE);
	

	/* ---------------*/
	/* HRTIM start-up */
	/* ---------------*/
	/* Enable HRTIM's outputs TA1 and TA2 */
	/* Note: it is necessary to enable also GPIOs to have outputs functional */
	HRTIM_WaveformOutputStart(HRTIM1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2); 

	/* Start HRTIM's TIMER A */
	HRTIM_WaveformCounterStart(HRTIM1, HRTIM_TIMERID_TIMER_A); 
	HRTIM_SlaveSetCompare(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1,  BUCK_PWM_PERIOD/3);


}

void GPIO_config(){
	

	//enable clock to pin ports
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB |
        RCC_AHBPeriph_GPIOF | RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_CRC, ENABLE);
	//init GPIO pin 
	GPIO_InitTypeDef GPIO_InitStruct;

	//LED_1: PB6
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStruct);

	//LED_2: PB7
	GPIO_ResetBits(GPIOB,GPIO_Pin_7);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	//MOD_A: PA8, uses HRTIMER
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//MOD_A alternative function setup
	// HRTIMER is on AF_13
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_13);

	//MOD_B: PA9, uses HRTIMER
	GPIO_ResetBits(GPIOA,GPIO_Pin_9);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA,&GPIO_InitStruct);

	//MOD_B alternative function setup
	// HRTIMER is on AF_13
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_13);
	
	//RF_A: PA10, uses HRTIMER
	GPIO_ResetBits(GPIOA,GPIO_Pin_10);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA,&GPIO_InitStruct);

	//RF_A alternative function setup
	// HRTIMER is on AF_13
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_13);

	//RF_B: PA11, uses HRTIMER
	GPIO_ResetBits(GPIOA,GPIO_Pin_11);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA,&GPIO_InitStruct);

	//RF_B alternative function setup
	// HRTIMER is on AF_13
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_13);
	

}

void led_on(){

	GPIO_SetBits(GPIOB,GPIO_Pin_6);
}

void led_off(){
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);
}

//configure high resolution timer for buck converter and RF out
//push pull mode for HRTIM1_B
void RF_config(){

	HRTIM_OutputCfgTypeDef HRTIM_TIM_OutputStructure;
	HRTIM_BaseInitTypeDef HRTIM_BaseInitStruct;
	HRTIM_TimerInitTypeDef HRTIM_TimerInitStructure;  
	HRTIM_TimerCfgTypeDef HRTIM_TimerWaveStructure;

	/* ----------------------------*/
	/* HRTIM Global initialization */
	/* ----------------------------*/
	/* Use the PLLx2 clock for HRTIM */
	RCC_HRTIM1CLKConfig(RCC_HRTIM1CLK_PLLCLK);
	/* Enable HRTIM clock*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_HRTIM1, ENABLE);

	/* HRTIM DLL calibration: periodic calibration, set period to 14µs */
	HRTIM_DLLCalibrationStart(HRTIM1, HRTIM_CALIBRATIONRATE_14);
	HRTIM1_COMMON->DLLCR |= HRTIM_DLLCR_CALEN; 

	/* Wait calibration completion*/
	while(HRTIM_GetCommonFlagStatus(HRTIM1, HRTIM_ISR_DLLRDY) == RESET);


	/* --------------------------------------------------- */
	/* TIMERB initialization: timer mode and PWM frequency */
	/* --------------------------------------------------- */
	


	HRTIM_TimerInitStructure.HalfModeEnable = HRTIM_HALFMODE_DISABLED;
	HRTIM_TimerInitStructure.StartOnSync = HRTIM_SYNCSTART_DISABLED;
	HRTIM_TimerInitStructure.ResetOnSync = HRTIM_SYNCRESET_DISABLED;
	HRTIM_TimerInitStructure.DACSynchro = HRTIM_DACSYNC_NONE;
	HRTIM_TimerInitStructure.PreloadEnable = HRTIM_PRELOAD_ENABLED;
	HRTIM_TimerInitStructure.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;
	HRTIM_TimerInitStructure.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
	HRTIM_TimerInitStructure.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;


	//setup hrtimer for max clock speed in continous mode.

	HRTIM_BaseInitStruct.Period = PERIOD_6_78MHZ;
	HRTIM_BaseInitStruct.RepetitionCounter = 255; //this is for interupts?
	HRTIM_BaseInitStruct.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL32; 
	HRTIM_BaseInitStruct.Mode= HRTIM_MODE_CONTINOUS; 

	
	//setup timer B
	HRTIM_Waveform_Init(HRTIM1, HRTIM_TIMERINDEX_TIMER_B, &HRTIM_BaseInitStruct, &HRTIM_TimerInitStructure);
	
	HRTIM_TimerWaveStructure.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_DISABLED;
	HRTIM_TimerWaveStructure.DelayedProtectionMode = HRTIM_TIMDELAYEDPROTECTION_DISABLED;
	HRTIM_TimerWaveStructure.FaultEnable = HRTIM_TIMFAULTENABLE_NONE;
	HRTIM_TimerWaveStructure.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;
	HRTIM_TimerWaveStructure.PushPull = HRTIM_TIMPUSHPULLMODE_ENABLED;
	HRTIM_TimerWaveStructure.ResetTrigger = HRTIM_TIMRESETTRIGGER_NONE;
	HRTIM_TimerWaveStructure.ResetUpdate = HRTIM_TIMUPDATEONRESET_DISABLED;
	HRTIM_TimerWaveStructure.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_NONE;

	HRTIM_WaveformTimerConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_B, &HRTIM_TimerWaveStructure);

	//more configure things
	HRTIM_TIM_OutputStructure.Polarity = HRTIM_OUTPUTPOLARITY_HIGH; 
	HRTIM_TIM_OutputStructure.SetSource = HRTIM_OUTPUTSET_TIMPER;  
	HRTIM_TIM_OutputStructure.ResetSource = HRTIM_OUTPUTRESET_TIMCMP1; 
	HRTIM_TIM_OutputStructure.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;  
	HRTIM_TIM_OutputStructure.IdleState = HRTIM_OUTPUTIDLESTATE_INACTIVE;          
	HRTIM_TIM_OutputStructure.FaultState = HRTIM_OUTPUTFAULTSTATE_INACTIVE;          
	HRTIM_TIM_OutputStructure.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;        
	HRTIM_TIM_OutputStructure.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;
	HRTIM_WaveformOutputConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_OUTPUT_TB1, &HRTIM_TIM_OutputStructure);
	HRTIM_WaveformOutputConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_OUTPUT_TB2, &HRTIM_TIM_OutputStructure);

	//enable timer
	HRTIM_WaveformCounterStart(HRTIM1, HRTIM_TIMERID_TIMER_B); 
}

void RF_enable(){
	//enable outputs for class E drive stage
	HRTIM_WaveformOutputStart(HRTIM1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
}

void RF_disable(){
	HRTIM_WaveformOutputStop(HRTIM1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);

}
