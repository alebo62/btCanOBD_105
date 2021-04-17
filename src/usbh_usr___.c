/**
 ******************************************************************************
 * @file    usbh_usr.c
 * @author  MCD Application Team
 * @version V2.2.1
 * @date    17-March-2018
 * @brief   This file includes the usb host library user callbacks
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                      <http://www.st.com/SLA0044>
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------ */
#include "usbh_usr.h"
#include "usbh_fs.h"
#include "usbh_data.h"
#include "usbh_cdc_core.h"
//#include "stm32f4xx_tim.h"
#include "stm32f10x_tim.h"
#include "messages.h"

extern USB_OTG_CORE_HANDLE USB_OTG_Core;
extern USBH_HOST USB_Host;
extern CDC_LineCodingTypeDef CDC_GetLineCode;
extern CDC_LineCodingTypeDef CDC_SetLineCode;
extern CDC_Usercb_TypeDef UserCb;
extern uint8_t tx[];
extern volatile uint8_t rx[];
extern uint8_t buff[];
extern uint8_t state;
extern uint16_t packet_num;
volatile uint32_t timer6_exp;
extern uint32_t press_ptt;
extern char uart_buff[];
extern uint32_t cnt_uart;
volatile extern uint32_t rx_flag;
extern uint32_t prg;
extern uint16_t LEDs[];
extern uint32_t channel_rx;

uint8_t prev_select = 0;
uint8_t enable_display_received_data = 0;
CDC_DEMO_StateMachine cdc_demo;
CDC_DEMO_SETTING_StateMachine cdc_settings_state;
CDC_DEMO_SelectMode cdc_select_mode;

static uint32_t i = 0;
#if defined(USE_STM324x9I_EVAL)
static uint16_t refcolumn = 479;
#else
static uint16_t refcolumn = 320;
#endif

static uint8_t line = 48;

static uint8_t Prev_BaudRateIdx = 0;
static uint8_t Prev_DataBitsIdx = 0;
static uint8_t Prev_ParityIdx = 0;
static uint8_t Prev_StopBitsIdx = 0;
static uint8_t Prev_Select = 0;

const uint32_t BaudRateValue[NB_BAUD_RATES] = { 2400, 4800, 9600, 19200, 38400,
		57600, 115200, 230400, 460800, 921600 };

const uint8_t DataBitsValue[4] = { 5, 6, 7, 8 };
const uint8_t ParityArray[3][5] = { "NONE", "EVEN", "ODD" };
const uint8_t StopBitsArray[2][2] = { "1", "2" };

/* Points to the DEVICE_PROP structure of current device */
/* The purpose of this register is to speed up the execution */

USBH_Usr_cb_TypeDef USR_Callbacks = { USBH_USR_Init, USBH_USR_DeInit,
		USBH_USR_DeviceAttached, USBH_USR_ResetDevice,
		USBH_USR_DeviceDisconnected, USBH_USR_OverCurrentDetected,
		USBH_USR_DeviceSpeedDetected, USBH_USR_Device_DescAvailable,
		USBH_USR_DeviceAddressAssigned, USBH_USR_Configuration_DescAvailable,
		USBH_USR_Manufacturer_String, USBH_USR_Product_String,
		USBH_USR_SerialNum_String, USBH_USR_EnumerationDone, USBH_USR_UserInput,
		USBH_USR_Application, USBH_USR_DeviceNotSupported,
		USBH_USR_UnrecoveredError };

static void CDC_SelectItem(uint8_t ** menu, uint8_t item);
static void CDC_Handle_Menu(void);
static void CDC_Handle_Send_Menu(void);
static void CDC_Handle_Receive_Menu(void);
static void CDC_Handle_Configuration_Menu(void);
static void CDC_ChangeSelectMode(CDC_DEMO_SelectMode select_mode);
static void CDC_AdjustSettingMenu(
		CDC_DEMO_SETTING_StateMachine * settings_state);
static void CDC_SelectSettingsItem(uint8_t item);
static void CDC_OutputData(uint8_t * ptr);
static void CDC_SetInitialValue(void);

void init_pereph(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE); //  uart1(program)  +  flash led_rx_chan
	TIM7->CNT = 1;
	TIM7->PSC = 72-1; // 1 us
	TIM7->ARR = 2000;  // 20 ms
	TIM7->DIER |= TIM_DIER_UIE;
	TIM7->EGR |= TIM_EGR_UG;
	TIM7->SR &= ~(TIM_SR_UIF);
	NVIC_EnableIRQ(TIM7_IRQn);
	
	GPIO_InitTypeDef GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

void USBH_USR_Init(void) {
	static uint8_t startup = 0;

	if (startup == 0) {
		startup = 1;

#ifndef STM32F10X_CL
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
		TIM6->CNT = 1;
		TIM6->PSC = 8400 - 1;
		TIM6->ARR = 10000;
		TIM6->DIER |= TIM_DIER_UIE;
		TIM6->EGR |= TIM_EGR_UG;
		TIM6->SR &= ~(TIM_SR_UIF);
		NVIC_EnableIRQ(TIM6_DAC_IRQn);

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
		TIM8->CNT = 1;
		TIM8->PSC = 16800 - 1;
		TIM8->ARR = 10000;
		TIM8->DIER |= TIM_DIER_UIE;
		//TIM8->EGR |= TIM_EGR_UG;
		//TIM8->SR &= ~(TIM_SR_UIF);
		NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);

		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14
		| GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_Init(GPIOD, &GPIO_InitStructure);

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
#else

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE); //  uart1(program)  +  flash led_rx_chan
	TIM7->CNT = 1;
	TIM7->PSC = 72-1; // 1 us
	TIM7->ARR = 2000;  // 20 ms
	TIM7->DIER |= TIM_DIER_UIE;
	TIM7->EGR |= TIM_EGR_UG;
	TIM7->SR &= ~(TIM_SR_UIF);
	NVIC_EnableIRQ(TIM7_IRQn);
	
	GPIO_InitTypeDef GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		TIM3->CNT = 1;
		TIM3->PSC = 36000 - 1;        // 500 us
		TIM3->ARR = 10000;  // 5 sec
		TIM3->DIER |= TIM_DIER_UIE;
		TIM3->EGR |= TIM_EGR_UG;
		TIM3->SR &= ~(TIM_SR_UIF);
		NVIC_EnableIRQ(TIM3_IRQn);

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
		TIM4->CNT = 1;
		TIM4->PSC = 36000 - 1;  // 500 us
		TIM4->ARR = 10000; // 5sec
		TIM4->DIER |= TIM_DIER_UIE;
		TIM4->EGR |= TIM_EGR_UG;
		TIM4->SR &= ~(TIM_SR_UIF);
		NVIC_EnableIRQ(TIM4_IRQn);

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
		TIM5->CNT = 1;
		TIM5->PSC = 360 - 1;  //  5us
		TIM5->ARR = 100 * 100; // 50ms
		TIM5->DIER |= TIM_DIER_UIE;
		TIM5->EGR |= TIM_EGR_UG;
		TIM5->SR &= ~(TIM_SR_UIF);
		NVIC_EnableIRQ(TIM5_IRQn);

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); // for debounce
		TIM6->CNT = 1; //
		TIM6->PSC = 360 - 1;
		TIM6->ARR = 100 * 100; // 50ms
		TIM6->DIER |= TIM_DIER_UIE;
		TIM6->EGR |= TIM_EGR_UG;
		TIM6->SR &= ~(TIM_SR_UIF);
		NVIC_EnableIRQ(TIM6_IRQn);

		

		//GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_0
				| GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

//        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
//        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//        GPIO_Init(GPIOA, &GPIO_InitStructure);

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2
				| GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

//        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
//        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
//        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//        GPIO_Init(GPIOC, &GPIO_InitStructure);

//        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;// | GPIO_Pin_2 | GPIO_Pin_3 |GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8;
//        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//        GPIO_Init(GPIOB, &GPIO_InitStructure);

#endif

#if defined (USE_STM322xG_EVAL)

#elif defined(USE_STM324xG_EVAL)

#elif defined (USE_STM3210C_EVAL)

#elif defined(USE_STM324x9I_EVAL)

#else
#error "Missing define: Evaluation board (ie. USE_STM322xG_EVAL)"
#endif

#ifdef USE_USB_OTG_HS
#ifdef USE_EMBEDDED_PHY

#else

#endif
#else

#endif

	}
}

#ifndef STM32F10X_CL  // if 407vg
//void TIM6_DAC_IRQHandler(void) {
//    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
//        TIM6->SR &= ~TIM_SR_UIF;
//        //tx[7] = packet_num;
//        USBH_BulkSendData(&USB_OTG_Core,    tx, tx[9], 7); //
//        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64   , 5);
//        TIM6->CR1 &= ~TIM_CR1_CEN;// 3sec delay
//        GPIO_WriteBit(GPIOD, GPIO_Pin_12, BitReset);
//    }
//}

//void TIM8_UP_TIM13_IRQHandler(void) {
//    if (TIM_GetITStatus(TIM8, TIM_IT_Update) != RESET) {
//        TIM8->SR &= ~TIM_SR_UIF;
//        //GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
//        TIM8->CR1 &= ~TIM_CR1_CEN;
//        state = 3;//TXT_NOTIFIC;
//    }
//}
#else 
void TIM3_IRQHandler(void) // start delay timer
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
		TIM3->SR &= ~TIM_SR_UIF;
		USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
		USBH_BulkReceiveData(&USB_OTG_Core, (uint8_t*) rx, 64, 5);
		TIM3->CR1 &= ~TIM_CR1_CEN;
		GPIOA->ODR &= ~0x3F;
	}
}

void TIM4_IRQHandler(void) //  reset timer
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
		TIM4->SR &= ~TIM_SR_UIF;
		state = TXT_NOTIFIC;
		printf("timer4 it\n");
		TIM4->CR1 &= ~TIM_CR1_CEN; // 3sec delay
	}
}

extern uint32_t ptt_debounce;
void TIM5_IRQHandler(void) // debounce timer
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
		TIM5->SR &= ~TIM_SR_UIF;
		ptt_debounce = 2;
		TIM5->CR1 &= ~TIM_CR1_CEN;
	}
}

extern uint32_t debounce;

void TIM6_IRQHandler(void) // debounce timer
{
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
		TIM6->SR &= ~TIM_SR_UIF;
		debounce = 2;
		TIM6->CR1 &= ~TIM_CR1_CEN;
	}
}

void TIM7_IRQHandler(void)  // program timer or flash led
{
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
	{
		TIM7->SR &= ~TIM_SR_UIF;
		if (!prg)
		{
			GPIOA->ODR ^= LEDs[channel_rx];
		}
		else
		{ // all message receive
			TIM7->CR1 &= ~TIM_CR1_CEN;
			rx_flag = 1;
			cnt_uart = 0;
		}
	}
}
#endif

void USART_Configuration(void) {
	USART_InitTypeDef USART_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	/* Enable UART clock */

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/* USART configuration */
	USART_Init(USART1, &USART_InitStructure);
	/* Enable the USART1 */
	USART_Cmd(USART1, ENABLE);
}

void USART_GPIO_Configuration(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	NVIC_EnableIRQ(USART1_IRQn);
	USART_Cmd(USART1, ENABLE);
}

void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) { /* read interrupt                   */
		//USART1->SR &= ~USART_SR_RXNE;	      /* clear interrupt                  */
		if (!cnt_uart)
		{
			TIM7->CR1 |= TIM_CR1_CEN; //
		}
		else
		{
			TIM7->CNT = 1; // reset timer7
		}
		uart_buff[cnt_uart++] = USART1->DR;
		cnt_uart &= 0x7F;
	}

	//    if (sr & USART_SR_TXE) {
	//      USART1->SR &= ~USART_SR_TXE;	      /* clear interrupt                    */

	//
	//    }
}

void USBH_USR_DeviceAttached(void) {

}

void USBH_USR_UnrecoveredError(void) {

}

void USBH_USR_DeviceDisconnected(void) {

}

void USBH_USR_ResetDevice(void) {
	/* Users can do their application actions here for the USB-Reset */
}

void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed) {
	if (DeviceSpeed == HPRT0_PRTSPD_HIGH_SPEED) {

	} else if (DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED) {

	} else if (DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED) {

	} else {

	}
}

void USBH_USR_Device_DescAvailable(void *DeviceDesc) {
	USBH_DevDesc_TypeDef *hs;
	hs = DeviceDesc;

}

void USBH_USR_DeviceAddressAssigned(void) {

}

void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
		USBH_InterfaceDesc_TypeDef * itfDesc, USBH_EpDesc_TypeDef * epDesc) {
	USBH_InterfaceDesc_TypeDef *id;

	id = itfDesc;

	if ((*id).bInterfaceClass == COMMUNICATION_DEVICE_CLASS_CODE) {
	} else {

	}
}

void USBH_USR_Manufacturer_String(void *ManufacturerString) {
}

void USBH_USR_Product_String(void *ProductString) {
}

void USBH_USR_SerialNum_String(void *SerialNumString) {
}

void USBH_USR_EnumerationDone(void) {
	/* Enumeration complete */
	UserCb.Receive = CDC_OutputData;
}

void USBH_USR_DeviceNotSupported(void) {

}

USBH_USR_Status USBH_USR_UserInput(void) {

	USBH_USR_Status usbh_usr_status;

	usbh_usr_status = USBH_USR_NO_RESP;

	if (STM_EVAL_PBGetState(Button_KEY) == RESET) {
		usbh_usr_status = USBH_USR_RESP_OK;
	}

	return usbh_usr_status;

}

void USBH_USR_OverCurrentDetected(void) {
}

void USBH_USR_DeInit(void) {
}

int USBH_USR_Application(void) {
	CDC_Handle_Menu();
	return 0;
}

void CDC_DEMO_ProbeKey(JOYState_TypeDef state) {

}

static void CDC_SelectItem(uint8_t ** menu, uint8_t item) {

}

static void CDC_Handle_Menu(void) {

}

static void CDC_Handle_Send_Menu(void) {

}

static void CDC_Handle_Receive_Menu(void) {

}

static void CDC_Handle_Configuration_Menu(void) {

}

static void CDC_ChangeSelectMode(CDC_DEMO_SelectMode select_mode) {

}

static void CDC_AdjustSettingMenu(
		CDC_DEMO_SETTING_StateMachine * settings_state) {

}

static void CDC_SelectSettingsItem(uint8_t item) {

}

static void CDC_OutputData(uint8_t * ptr) {

}

static void CDC_SetInitialValue(void) {

}

