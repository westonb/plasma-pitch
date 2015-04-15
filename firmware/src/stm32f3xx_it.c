/**
  ******************************************************************************
  * @file    stm32f3xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-June-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_it.h"
#include "main.h"
#define audio_filter_k_p 200
#define mod_filter_k_p 2
#define mod_filter_k_i 20
#define integral_max 200000
#define integral_min -200000
#define volume 10
#define dc_offset 200
#define ADC_SAMPLE_TIME 100

//#include “core_cm3.h” //needed for data types


#include "stm32f30x.h" //prehaps this does something? 

int32_t audio_filtered = 0; //signed 
    
/** @addtogroup HRTIM_DualBuck
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                          */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	static uint32_t counter;
	counter++;
 	TimingDelay_Decrement();
	
}

/******************************************************************************/
/*                 STM32F3xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f334x8.s).                                             */
/******************************************************************************/
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/**
  * @brief  This function handles HRTIM1_TIMD_IRQHandler interrupt request.
  * The duty cycle on TD1 and TD2 are modified in real time increased
  * @param  None
  * @retval None
  */
void HRTIM1_TIMA_IRQHandler(void)
{
	int32_t duty_cycle;
	int32_t error_i_next;
	static uint32_t counter;
	static int32_t error;
	static int32_t error_last;
	static int32_t control;
	static int32_t control_last;
	static int32_t error_i;
	
	counter++;
	HRTIM_ClearITPendingBit(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_TIM_FLAG_REP);
 	
	//12 bit ADC readings 
	//output_current = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1); 
	output_voltage = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_2); 
	//supply_voltage = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_3); 
	audio_voltage = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_4); 

	
	//IIR lowpass filter for audio input 
	//audio filtered is scaled by 2**8
	//audio filtered value is signed
	//audio filtered ranges between 522240 and -522240 (20 bit bit signed number)
	//audo voltage is 3.3/2^20 volts
	audio_filtered = ((audio_filter_k_p*audio_filtered)>>8)+(256-audio_filter_k_p)*((int32_t)audio_voltage-(1<<11));


	//target output voltage is volume * 3.3/2^20
	//output voltage is output_voltage * 59.3/2^12
	//error is 1/(2^24) volts
	error = (((audio_filtered>>8)*74 - (int32_t)output_voltage*236)); 
	
	error_i_next = error+ error_i;
	if((error_i_next < integral_max) && (error_i_next>integral_min)){
	error_i = error_i_next;
	}
	

	duty_cycle = ((mod_filter_k_p*error)>>8) + ((mod_filter_k_i*error_i)>>8);

	if(duty_cycle>=BUCK_PWM_MAX){
		HRTIM_SlaveSetCompare(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1,  (uint32_t)BUCK_PWM_MAX); /* Duty cycle update */
    		HRTIM_SlaveSetCompare(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2,  (uint32_t)ADC_SAMPLE_TIME); /* ADC trigger update */
	}
	else if(duty_cycle<=BUCK_PWM_MIN){
		HRTIM_SlaveSetCompare(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1,  (uint32_t)BUCK_PWM_MIN); /* Duty cycle update */
    		HRTIM_SlaveSetCompare(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2,  (uint32_t)ADC_SAMPLE_TIME); /* ADC trigger update */
	}
	else {
		HRTIM_SlaveSetCompare(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1,  (uint32_t)duty_cycle); /* Duty cycle update */
	    	HRTIM_SlaveSetCompare(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2,  (uint32_t)ADC_SAMPLE_TIME); /* ADC trigger update */	
	}
	
	
}


/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
