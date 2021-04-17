/**
 ******************************************************************************
 * @file    main.c
 * @author  MCD Application Team
 * @version V2.2.1
 * @date    17-March-2018
 * @brief   USB Host CDC class demo main file
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
#include "usb_bsp.h"
#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_cdc_core.h"
#include "messages.h"
#include "string.h"
#include "stdlib.h"
/** @addtogroup USBH_USER
 * @{
 */
extern hrnp_t zone_ch;
extern hrnp_t status_change_req;
extern hrnp_no_pld_t pwr_up_chk_req;
extern hrnp_no_pld_t ack_msg;
extern hrnp_no_pep_t conn_req;
extern hrnp_no_pep_t close_req;
extern hrnp_t txt_notif_req;
extern hrnp_t txt_msg_open;
extern hrnp_t txt_msg_close;
extern hrnp_t uart_config_req;
extern hrnp_t ptt_press_req;
extern hrnp_t ptt_release_req;
extern hrnp_t call_req;
extern hrnp_t key_notific_req;
extern hrnp_t br_cast_req;
extern hrnp_t dig_audio_tx_req;
extern hrnp_t dig_audio_rx_req;
extern hrnp_t rtp_pld_req;
extern void init_msg(void);
extern volatile uint32_t timer6_exp;
extern void checksum(uint8_t[]);
extern void init_pereph(void);
extern void USART_Configuration(void);
extern void USART_GPIO_Configuration(void);

void read_data(void);
void program(void);
void normal_keys_process(void);
/** @defgroup USBH_USR_MAIN
 * @brief This file is the Host CDC demo main file
 * @{
 */

/** @defgroup USBH_USR_MAIN_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup USBH_USR_MAIN_Private_Defines
 * @{
 */
/**
 * @}
 */
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000
#define MY_FLASH_PAGE_ADDR 0x803FC00
//#define SETTINGS_WORDS 12

uint16_t keys_old_state;
uint16_t keys_debounce;
uint16_t keys_state;
uint32_t channels = 2;
uint32_t call_type[6] = { 'G', 'G', 'G', 'G', 'G', 'G' };
uint32_t call_number[6] = { 10, 20, 104, 20, 30, 40 };
uint32_t chan_number[6] = { 1, 2, 104, 20, 30, 40 };

uint32_t settings[13];
uint32_t set_cnt;
uint32_t rx_flag;
char uart_buff[128];
uint32_t cnt_uart;
uint8_t chan_busy;
uint8_t current_chan;
uint8_t func_key = 1; // выбор режима работы 
uint8_t enable_ptt = 0; // кнопка работает как ртт
uint8_t press_key = 0; // кнопка канала нажата

struct __FILE {
    int handle; /* Add whatever you need here */
};
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
    if (DEMCR & TRCENA) {
        while (ITM_Port32(0) == 0)
            ;
        ITM_Port8(0) = ch;
    }
    return (ch);
}

enum {
    S_CHNG_NOTIFIC = 16, ZONE_CHANNEL
};

/** @defgroup USBH_USR_MAIN_Private_Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup USBH_USR_MAIN_Private_Variables
 * @{
 */
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined ( __ICCARM__ )      /* !< IAR Compiler */
#pragma data_alignment=4
#endif
#endif                          /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_Core __ALIGN_END;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined ( __ICCARM__ )      /* !< IAR Compiler */
#pragma data_alignment=4
#endif
#endif                          /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USBH_HOST USB_Host __ALIGN_END;
/**
 * @}
 */

/** @defgroup USBH_USR_MAIN_Private_FunctionPrototypes
 * @{
 */
/**
 * @}
 */

/** @defgroup USBH_USR_MAIN_Private_Functions
 * @{
 */

/**
 * @brief  Main routine for CDC class application
 * @param  None
 * @retval int
 */

//#define PDL  (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)) 
//#define KEY1 0x20//(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5))
//#define KEY2 0x10//(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4))
//#define KEY3 0x08//(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3))
//#define KEY4 0x04//(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2))
//#define KEY5 0x02//(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1))
//#define KEY6 0x01//(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0))
//#define onLED1 (GPIO_WriteBit(GPIOA, GPIO_Pin_5, 1))
//#define onLED2 (GPIO_WriteBit(GPIOA, GPIO_Pin_4, 1))
//#define onLED3 (GPIO_WriteBit(GPIOA, GPIO_Pin_3, 1))
//#define onLED4 (GPIO_WriteBit(GPIOA, GPIO_Pin_2, 1))
//#define onLED5 0x02//(GPIO_WriteBit(GPIOA, GPIO_Pin_1, 1))
//#define onLED6 0x01//(GPIO_WriteBit(GPIOA, GPIO_Pin_0, 1))
//#define onMIC_LED (GPIO_WriteBit(GPIOC, GPIO_Pin_9, 1))
//#define offLED1 (GPIO_WriteBit(GPIOA, GPIO_Pin_5, 0))
//#define offLED2 (GPIO_WriteBit(GPIOA, GPIO_Pin_4, 0))
//#define offLED3 (GPIO_WriteBit(GPIOA, GPIO_Pin_3, 0))
//#define offLED4 (GPIO_WriteBit(GPIOA, GPIO_Pin_2, 0))
//#define offLED5 (GPIO_WriteBit(GPIOA, GPIO_Pin_1, 0))
//#define offLED6 (GPIO_WriteBit(GPIOA, GPIO_Pin_0, 0))
//#define offMIC_LED (GPIO_WriteBit(GPIOC, GPIO_Pin_9, 0))
volatile uint32_t msg_rx;

uint32_t ptt_debounce;
uint32_t debounce;
uint32_t prg;
uint32_t channel_rx;
uint32_t channel_tx;
uint16_t LEDmic = 0x200;
uint16_t PDLkey = 0x100;
uint16_t LEDs[] = { 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
uint16_t KEYs[] = { 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
uint8_t start = 0;
uint32_t state;
uint8_t tx[64];
uint8_t rx[64];
uint16_t packet_num = 0;
uint32_t press_ptt = 0;
uint32_t rx_id;

int main(void) {

    state = CONN;
    init_msg();
    init_pereph();
    //if(GPIO_ReadInputDataBit(GPIOB, GPIO_PinSource9))
    {
        //prg = 1;
        //program();

    }
		
	

    memcpy(tx, (char*) &conn_req, conn_req.length[1]);// see timer irq every 300ms
    checksum(tx);
    USBH_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USB_Host, &CDC_cb,
              &USR_Callbacks);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, DISABLE);

    //read_data();
    channel_tx = 0;
		
		if(GPIOB->IDR & 0x200)
			func_key = 1;
		
    TIM7->PSC = 3600 - 1; // 50 us
    TIM7->ARR = 4000; // 200ms
    TIM7->CNT = 1;
    while (1) {
        USBH_Process(&USB_OTG_Core, &USB_Host);

        //if (!start && (HOST_CTRL_XFER == USB_Host.gState)) {
        if (!start && (0 != USB_Host.device_prop.address)) {
            start = 1;
            TIM3->CR1 |= TIM_CR1_CEN; // 3sec delay
            GPIOA->ODR |= 0x3F;
        }

        if (rx[0]) {
            rx[0] = 0;
            //            for(int i = 0; i < rx[9]; i++)
            //            {
            //                printf("%x,", rx[i]);
            //            }
            printf("%x%x\r\n", rx[13], rx[14]);
            //printf("->%d\r\n", state);
            switch (state) {
            case CONN:
                if (rx[3] == 0xFD) {
                    packet_num++;
                    pwr_up_chk_req.packet_num[0] = packet_num >> 8;
                    pwr_up_chk_req.packet_num[1] = packet_num & 0xFF;
                    memcpy(tx, (char*) &pwr_up_chk_req,
                           pwr_up_chk_req.length[1]);
                    checksum(tx);
                    USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                    state = PWR_UP;
                } else {
                    printf("connect fail!\r\n");
                }
                break;

            case PWR_UP:
                if ((rx[13] == 0xC6) && (rx[14] == 0x80)) {
                    packet_num++;
                    txt_notif_req.packet_num[0] = packet_num >> 8;
                    txt_notif_req.packet_num[1] = packet_num & 0xFF;
                    memcpy(tx, (char*) &txt_notif_req, 17);
                    memcpy(tx + 17, (char*) txt_notif_req.pep.pld,
                           txt_notif_req.pep.num_of_bytes[0]);
                    memcpy(tx + 17 + txt_notif_req.pep.num_of_bytes[0],
                            (char*) (&txt_notif_req) + 21, 2);
                    checksum(tx);
                    USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                    state = KEY_NOTIFIC;
                } else {
                    printf("pwr up chk fail!\r\n");
                }
                break;

            case KEY_NOTIFIC: //3
                if ((rx[13] == 0xD0) && (rx[14] == 0x80)) {
                    packet_num++;
                    key_notific_req.packet_num[0] = packet_num >> 8; // audio route rx tx  page232
                    key_notific_req.packet_num[1] = packet_num & 0xFF;
                    memcpy(tx, (char*) (&key_notific_req), 17);
                    memcpy(tx + 17, (char*) (key_notific_req.pep.pld),
                           key_notific_req.pep.num_of_bytes[0]);
                    memcpy(tx + 17 + key_notific_req.pep.num_of_bytes[0],
                            (char*) (&key_notific_req) + 21, 2);
                    checksum(tx);
                    USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                    state = BRDCST_NOTIFIC;
                } else {
                    printf("txt notif fail!\r\n");
                }
                break;
            case BRDCST_NOTIFIC:
                if ((rx[13] == 0xE4) && (rx[14] == 0x80)) {
                    if (rx[17] == 0) {
                        packet_num++;
                        br_cast_req.packet_num[0] = packet_num >> 8; // broadcast rx and tx  page80
                        br_cast_req.packet_num[1] = packet_num & 0xFF;
                        memcpy(tx, (char*) (&br_cast_req), 17);
                        memcpy(tx + 17, (char*) (br_cast_req.pep.pld),
                               br_cast_req.pep.num_of_bytes[0]);
                        memcpy(tx + 17 + br_cast_req.pep.num_of_bytes[0],
                                (char*) (&br_cast_req) + 21, 2);
                        checksum(tx);
                        USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        //state = ROUTE_RX;
                        state = RTP_PAYLOAD;
                    } else {
                        printf("key notif fail!\r\n");
                    }
                }
                break;

            case RTP_PAYLOAD:
                if ((rx[13] == 0xC9) && (rx[14] == 0x80)) {
                    if (rx[17] == 0x00) {
                        packet_num++;
                        rtp_pld_req.packet_num[0] = packet_num >> 8; // page 230  pcm pmu  pma  0 8 0x78
                        rtp_pld_req.packet_num[1] = packet_num & 0xFF;
                        memcpy(tx, (char*) (&rtp_pld_req), 17);
                        memcpy(tx + 17, (char*) (rtp_pld_req.pep.pld),
                               rtp_pld_req.pep.num_of_bytes[0]);
                        memcpy(tx + 17 + rtp_pld_req.pep.num_of_bytes[0],
                                (char*) (&rtp_pld_req + 21), 2);
                        checksum(tx);
                        USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        state = ROUTE_RX;
                    }
                }

                break;
            case ROUTE_RX:
                //				if((rx[13] == 0xC9)&& (rx[14] == 0x80))
                //					{
                //						if(rx[17] == 0x00)
                //						{
                if ((rx[13] == 0x19) && (rx[14] == 0x84)) {
                    if (rx[17] == 0x00) {
                        packet_num++;
                        dig_audio_rx_req.packet_num[0] = packet_num >> 8; // audio route rx tx  page232
                        dig_audio_rx_req.packet_num[1] = packet_num & 0xFF;
                        memcpy(tx, (char*) (&dig_audio_rx_req), 17);
                        memcpy(tx + 17, (char*) (dig_audio_rx_req.pep.pld),
                               dig_audio_rx_req.pep.num_of_bytes[0]);
                        memcpy(tx + 17 + dig_audio_rx_req.pep.num_of_bytes[0],
                                (char*) (&dig_audio_rx_req) + 21, 2);
                        checksum(tx);
                        USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        state = ROUTE_TX;
                    } else {
                        printf("br cast fail!\r\n");
                    }
                }
                break;
            case ROUTE_TX:
                if ((rx[13] == 0xDF) && (rx[14] == 0x80)) {
                    if (rx[17] == 0x00) {
                        packet_num++;
                        dig_audio_tx_req.packet_num[0] = packet_num >> 8; // audio route rx tx  page232
                        dig_audio_tx_req.packet_num[1] = packet_num & 0xFF;
                        memcpy(tx, (char*) (&dig_audio_tx_req), 17);
                        memcpy(tx + 17, (char*) (dig_audio_tx_req.pep.pld),
                               dig_audio_tx_req.pep.num_of_bytes[0]);
                        memcpy(tx + 17 + dig_audio_tx_req.pep.num_of_bytes[0],
                                (char*) (&dig_audio_tx_req) + 21, 2);
                        checksum(tx);
                        USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        state = ZONE_CHANNEL;
                    } else {
                        printf("route rx fail!\r\n");
                    }
                }
                break;
            case ZONE_CHANNEL:
                if ((rx[13] == 0xDF) && (rx[14] == 0x80)) {
                    packet_num++;
                    zone_ch.packet_num[0] = packet_num >> 8; // audio route rx tx  page232
                    zone_ch.packet_num[1] = packet_num & 0xFF;
                    zone_ch.pep.pld[0] = 1; // read chan number
                    memcpy(tx, (char*) (&zone_ch), 17);
                    memcpy(tx + 17, (char*) (zone_ch.pep.pld),
                           zone_ch.pep.num_of_bytes[0]);
                    memcpy(tx + 17 + zone_ch.pep.num_of_bytes[0],
                            (char*) (&zone_ch) + 21, 2);
                    checksum(tx);
                    USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                    state = S_CHNG_NOTIFIC;
                } else {
                    printf("route tx fail!\r\n");
                }
                break;
            case S_CHNG_NOTIFIC:
                if ((rx[13] == 0xC4) && (rx[14] == 0x80)) {
                    current_chan = rx[27];
                    channel_tx = current_chan - 1;
									  call_req.pep.pld[0] =
                            (call_type[channel_tx] == 0x67) ? 1 : 0; //  set channel for ptt
                    memcpy((char*) call_req.pep.pld + 1,
                           (char*) (call_number + channel_tx), 4);
                    packet_num++;
                    status_change_req.packet_num[0] = packet_num >> 8; // audio route rx tx  page232
                    status_change_req.packet_num[1] = packet_num & 0xFF;
                    memcpy(tx, (char*) (&status_change_req), 17);
                    memcpy(tx + 17, (char*) (status_change_req.pep.pld),
                           status_change_req.pep.num_of_bytes[0]);
                    memcpy(tx + 17 + status_change_req.pep.num_of_bytes[0],
                            (char*) (&status_change_req) + 21, 2);
                    checksum(tx);
                    USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                    state = TXT_NOTIFIC;
                }
                break;
            case TXT_NOTIFIC:
                if ((rx[13] == 0xC7) && (rx[14] == 0x80)) {
                    if (rx[17] == 0x00) {
                        if (!func_key)
                            GPIOA->ODR |= LEDs[channel_tx];
                        else
                            GPIOA->ODR &= ~0x3F;
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        //TIM4->CR1 |= TIM_CR1_CEN;
                    } else
                        printf("route tx fail\r\n");
                }
                // ******************** RX ***********************************
                else if ((rx[13] == 0xC8) && (rx[14] == 0xB0)) {
                    chan_busy = rx[18]; // 1- detect carrier
                    if (func_key) {
                        if (chan_busy)
                            GPIOA->ODR |= LEDs[channel_tx];
                        else
                            GPIOA->ODR &= ~0x3F;
                    }

                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                } else if ((rx[13] == 0x44) && (rx[14] == 0xB8)) {
                    if (rx[17] == 1) { // rx[19] = 1(gr) =0(ind)  21-23 id
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        if (!rx[19])
                            rx_id = (rx[27] << 16) + (rx[26] << 8) + rx[25];
                        else
                            rx_id = (rx[23] << 16) + (rx[22] << 8) + rx[21];
                        //printf("%d,", rx_id);
                        for (int i = 0; i < channels; i++) {
                            if (rx_id == call_number[i]) {
                                channel_rx = i;
                                GPIOA->ODR &= ~0x3F;
                                TIM7->CR1 |= TIM_CR1_CEN;
                            }
                        }
                        state = CALL_RX_HANG;
                    }
                } else if ((rx[13] == 0xC4) && (rx[14] == 0x80)) {
                    current_chan = rx[27];
                    enable_ptt = 1;
									  debounce = 0; 
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                }
                break;
            case CALL_RX_HANG:
                if ((rx[13] == 0x44) && (rx[14] == 0xB8)) {
                    if (rx[17] == 2) {
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        state = CALL_RX_END;
                    }
                } else if ((rx[13] == 0xC8) && (rx[14] == 0xB0)) {
                    chan_busy = rx[18]; // 1- detect carrier
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                }
                break;
            case CALL_RX_END:
                if ((rx[13] == 0x44) && (rx[14] == 0xB8)) 
									{
                    if (rx[17] == 3) { // call ended
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        state = TXT_NOTIFIC;
                        TIM7->CR1 &= ~TIM_CR1_CEN;
                        GPIOA->ODR &= ~0x3F;
                        if (!func_key)
                            GPIOA->ODR |= LEDs[channel_tx];
                        else
                            GPIOA->ODR &= ~0x3F;
                  } 
									else if (rx[17] == 1) 
										{ // call continue
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        if (!rx[19])
                            rx_id = (rx[27] << 16) + (rx[26] << 8) + rx[25];
                        else
                            rx_id = (rx[23] << 16) + (rx[22] << 8) + rx[21];
                        printf("%d,", rx_id);
                        for (int i = 0; i < channels; i++) {
                            if (rx_id == call_number[i]) {
                                channel_rx = i;
                                GPIOA->ODR &= ~0x3F;
                                TIM7->CR1 |= TIM_CR1_CEN;
                            }
                        }
                        state = CALL_RX_HANG;
                    }
                } else if ((rx[13] == 0xC8) && (rx[14] == 0xB0)) {
                    chan_busy = rx[18]; // 1- detect carrier
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                }
                break;
                // ************************** Continue Calling procedure *******************
            case WAIT_CALL_REPLY: //
                if ((rx[13] == 0x41) && (rx[14] == 0x88)) {
                    packet_num++;
                    ptt_press_req.packet_num[0] = packet_num >> 8; // page 159
                    ptt_press_req.packet_num[1] = packet_num & 0xFF;
                    memcpy(tx, (char*) (&ptt_press_req), 17);
                    memcpy(tx + 17, (char*) (ptt_press_req.pep.pld),
                           ptt_press_req.pep.num_of_bytes[0]);
                    memcpy(tx + 17 + ptt_press_req.pep.num_of_bytes[0],
                            (char*) (&ptt_press_req) + 21, 2);
                    checksum(tx);
                    USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                    state = WAIT_CALL_REPORT;
                    TIM4->CNT = 3700; // reset timer
                } else {
                    state = TXT_NOTIFIC;
                }
                break;
            case WAIT_CALL_REPORT: //
                if ((rx[13] == 0x41) && (rx[14] == 0x80)) { // page 94
                    if (rx[17] == 0) {
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        TIM4->CNT = 3700; // reset timer

                    } else {
											 printf("%d,", rx[17]);
                        //Radio_Reg_State = READY;
                    }
                } else if ((rx[13] == 0x43) && (rx[14] == 0xB8)) {
                    if (rx[17] == 1) { // voice call
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        state = WAIT_STOP_CALL_REQ;
                        TIM4->CR1 &= ~TIM_CR1_CEN; // stop timer  start call
                    } else {

                    }
                }
                break;
                // ********************** TX *******************************
            case WAIT_STOP_CALL_REQ:
                if ((rx[13] == 0x41) && (rx[14] == 0x80)) { // page 94
                    if (rx[17] == 0) {
                        state = WAIT_STOP_CALL_HANG;
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        TIM4->CNT = 1; // 300 ms

                        //TIM4->CR1 |= TIM_CR1_CEN;
                    } else {
                        //Radio_Reg_State = READY;
                    }
                }
                break;
            case WAIT_STOP_CALL_HANG:
                if ((rx[13] == 0x43) && (rx[14] == 0xB8)) { // page 82
                    if (rx[17] == 2) {
                        state = WAIT_STOP_CALL_END;
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        TIM4->CNT = 1; // 4sec hang
                    }
                } else { // rx?

                }
								
                break;
            case WAIT_STOP_CALL_END:
                if ((rx[13] == 0x43) && (rx[14] == 0xB8)) { // page 82
                    if (rx[17] == 3) {
                        state = TXT_NOTIFIC;
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        TIM4->CR1 &= ~TIM_CR1_CEN;
                        TIM7->CR1 &= ~TIM_CR1_CEN;
                        if (!func_key) {
                            GPIOA->ODR &= ~0x3F;
                            GPIOA->ODR |= LEDs[channel_tx];
                        } else
                            GPIOA->ODR &= ~0x3F;
                    }
                } else if ((rx[13] == 0x44) && (rx[14] == 0xB8)) {
                    if (rx[17] == 1) { // rx[19] = 1(gr) =0(ind)  21-23 id
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        if (!rx[19])
                            rx_id = (rx[27] << 16) + (rx[26] << 8) + rx[25];
                        else
                            rx_id = (rx[23] << 16) + (rx[22] << 8) + rx[21];
                        //printf("%d,", rx_id);
                        for (int i = 0; i < channels; i++) {
                            if (rx_id == call_number[i]) {
                                channel_rx = i;
                                TIM7->CR1 |= TIM_CR1_CEN;
                            }
                        }
                        state = CALL_RX_HANG;
												TIM4->CR1 &= ~TIM_CR1_CEN;
                    }
                }
								else if ((rx[13] == 0xC8) && (rx[14] == 0xB0)) {
                    chan_busy = rx[18]; // 1- detect carrier
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                }
                break;
            }

        }
				
				
        // read keys channels
        keys_state = (GPIOC->IDR) & 0x3F;

        if (!func_key) {
            normal_keys_process();
        }
        else // func key = 1
        {
            if ((keys_state != 0x3F) && !debounce && !enable_ptt)
            {
                //if (keys_state != keys_old_state)
                {
                    keys_debounce = keys_state;
                    TIM6->CNT = 1;
                    TIM6->CR1 |= TIM_CR1_CEN;
                    debounce = 1;
                }
            }
            // after delay
            if (debounce == 2) {
                if (keys_state == keys_debounce) {
                    //keys_old_state = keys_state;
                    switch (keys_state) {
                    case 0x3E:
                        channel_tx = 5;
                        break;
                    case 0x3D:
                        channel_tx = 4;
                        break;
                    case 0x3B:
                        channel_tx = 3;
                        break;
                    case 0x37:
                        channel_tx = 2;
                        break;
                    case 0x2F:
                        channel_tx = 1;
                        break;
                    case 0x1F:
                        channel_tx = 0;
                        break;
                    default:
                        break;
                    }
                    debounce = 0;
                    GPIOA->ODR &= ~0x3F;
                    //GPIOA->ODR |= LEDs[channel_tx];
                    call_req.pep.pld[0] =
                            (call_type[channel_tx] == 0x67) ? 1 : 0;
                    memcpy((char*) call_req.pep.pld + 1,
                           (char*) (call_number + channel_tx), 4); // check!!!!!!!!!!!!!!!!

                    if (current_chan != chan_number[channel_tx] // если надо переключить канал
                            && (state == TXT_NOTIFIC))
                    {
											  debounce = 3; // see answer ch_zone
                        packet_num++;
                        zone_ch.packet_num[0] = packet_num >> 8; // audio route rx tx  page232
                        zone_ch.packet_num[1] = packet_num & 0xFF;
                        zone_ch.pep.pld[0] = 0; // set chan number
                        zone_ch.pep.pld[3] = chan_number[channel_tx];
                        zone_ch.pep.pld[4] = 0;
                        memcpy(tx, (char*) (&zone_ch), 17);
                        memcpy(tx + 17, (char*) (zone_ch.pep.pld),
                               zone_ch.pep.num_of_bytes[0]);
                        memcpy(tx + 17 + zone_ch.pep.num_of_bytes[0],
                                (char*) (&zone_ch) + 21, 2);
                        checksum(tx);
                        USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
                        USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                        enable_ptt = 0; // ждем когда перекл.ючится канал
                    }
                    else
                    {
                        enable_ptt = 1;
                    }
                }
            }

            if(enable_ptt && !press_key)
            {
                switch (state)
                {
                case CALL_RX_END: // quick answer
                    //GPIOC->ODR |= LEDmic;
                    packet_num++;
                    printf("+pkptt\r\n"); //, packet_num);
                    ptt_press_req.packet_num[0] = packet_num >> 8; // page 159
                    ptt_press_req.packet_num[1] = packet_num & 0xFF;
                    memcpy(tx, (char*) (&ptt_press_req), 17);
                    memcpy(tx + 17, (char*) (ptt_press_req.pep.pld),
                           ptt_press_req.pep.num_of_bytes[0]);
                    memcpy(tx + 17 + ptt_press_req.pep.num_of_bytes[0],
                            (char*) (&ptt_press_req) + 21, 2);
                    checksum(tx);
                    USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                    state = WAIT_CALL_REPORT;
                    TIM4->CNT = 4000 - 300; // 300 ms
                    TIM4->CR1 |= TIM_CR1_CEN;
                    press_key = 1;
										break;
                case TXT_NOTIFIC: // new call
                    //GPIOC->ODR |= LEDmic;
								    for(int i = 0; i < 100000; i++);
                    packet_num++;
                    printf("pkptt\r\n"); //printf("pptt->%d\r\n", packet_num);
                    call_req.packet_num[0] = packet_num >> 8; // page 230  pcm pmu  pma  0 8 0x78
                    call_req.packet_num[1] = packet_num & 0xFF;
                    memcpy(tx, (char*) (&call_req), 17);
                    memcpy(tx + 17, (char*) (call_req.pep.pld),
                           call_req.pep.num_of_bytes[0]);
                    memcpy(tx + 17 + call_req.pep.num_of_bytes[0],
                            (char*) (&call_req) + 21, 2);
                    checksum(tx);
                    USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
                    state = WAIT_CALL_REPLY;
                    TIM4->CNT = 4000 - 300; // 500ms
                    TIM4->CR1 |= TIM_CR1_CEN;
                    press_key = 1;
										break;
                default:
                    break;
                }
            }
            if ((state == WAIT_STOP_CALL_REQ) && ((GPIOC->IDR & 0x3F) == 0x3F) && press_key)
            {
                //GPIOC->ODR &= ~LEDmic;
                press_key = 0;
                packet_num++;
                printf("rkptt\r\n");        //printf("rptt->%d\r\n", packet_num);
                ptt_release_req.packet_num[0] = packet_num >> 8;
                ptt_release_req.packet_num[1] = packet_num & 0xFF;
                memcpy(tx, (char*) (&ptt_release_req), 17);
                memcpy(tx + 17, (char*) (ptt_release_req.pep.pld),
                       ptt_release_req.pep.num_of_bytes[0]);
                memcpy(tx + 17 + ptt_release_req.pep.num_of_bytes[0],
                        (char*) (&ptt_release_req) + 21, 2);
                checksum(tx);
                USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
                state = WAIT_STOP_CALL_REQ;
                TIM4->CNT = 1;
                TIM4->CR1 |= TIM_CR1_CEN;
								enable_ptt = 0;

            }

            

            // read ptt
            if ((((GPIOC->IDR) & PDLkey) == 0) && !press_ptt && !ptt_debounce) {
                TIM5->CNT = 1;
                TIM5->CR1 |= TIM_CR1_CEN;
                ptt_debounce = 1;
            }
            // after delay
            if ((ptt_debounce == 2) && (((GPIOC->IDR) & PDLkey) == 0)
                    && !press_ptt) {
                switch (state) {
                case CALL_RX_END: // quick answer
                    GPIOC->ODR |= LEDmic;
                    packet_num++;
                    printf("+pptt\r\n"); //, packet_num);
                    ptt_press_req.packet_num[0] = packet_num >> 8; // page 159
                    ptt_press_req.packet_num[1] = packet_num & 0xFF;
                    memcpy(tx, (char*) (&ptt_press_req), 17);
                    memcpy(tx + 17, (char*) (ptt_press_req.pep.pld),
                           ptt_press_req.pep.num_of_bytes[0]);
                    memcpy(tx + 17 + ptt_press_req.pep.num_of_bytes[0],
                            (char*) (&ptt_press_req) + 21, 2);
                    checksum(tx);
                    USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
                    USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                    state = WAIT_CALL_REPORT;
                    TIM4->CNT = 4000 - 300; // 300 ms
                    TIM4->CR1 |= TIM_CR1_CEN;
                    ptt_debounce = 0;
                    press_ptt = 1;
                    break;
                case TXT_NOTIFIC: // new call
                    GPIOC->ODR |= LEDmic;
                    packet_num++;
                    printf("pptt\r\n"); //printf("pptt->%d\r\n", packet_num);
                    call_req.packet_num[0] = packet_num >> 8; // page 230  pcm pmu  pma  0 8 0x78
                    call_req.packet_num[1] = packet_num & 0xFF;
                    memcpy(tx, (char*) (&call_req), 17);
                    memcpy(tx + 17, (char*) (call_req.pep.pld),
                           call_req.pep.num_of_bytes[0]);
                    memcpy(tx + 17 + call_req.pep.num_of_bytes[0],
                            (char*) (&call_req) + 21, 2);
                    checksum(tx);
                    USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
                    state = WAIT_CALL_REPLY;
                    TIM4->CNT = 4000 - 300; // 500ms
                    TIM4->CR1 |= TIM_CR1_CEN;
                    ptt_debounce = 0;
                    press_ptt = 1;
                    break;
                default:
                    break;
                }

                //USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
                //USBH_Process(&USB_OTG_Core, &USB_Host);

            }
            //}
            if ((state == WAIT_STOP_CALL_REQ) && ((GPIOC->IDR) & PDLkey)
                    && press_ptt) {
                GPIOC->ODR &= ~LEDmic;
                press_ptt = 0;
                packet_num++;
                printf("rptt\r\n");        //printf("rptt->%d\r\n", packet_num);
                ptt_release_req.packet_num[0] = packet_num >> 8;
                ptt_release_req.packet_num[1] = packet_num & 0xFF;
                memcpy(tx, (char*) (&ptt_release_req), 17);
                memcpy(tx + 17, (char*) (ptt_release_req.pep.pld),
                       ptt_release_req.pep.num_of_bytes[0]);
                memcpy(tx + 17 + ptt_release_req.pep.num_of_bytes[0],
                        (char*) (&ptt_release_req) + 21, 2);
                checksum(tx);
                USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
                state = WAIT_STOP_CALL_REQ;
                TIM4->CNT = 1;
                TIM4->CR1 |= TIM_CR1_CEN;

            }

        }
    }
}

void read_data(void) {
    FLASH_Unlock();
    uint32_t p_flash = MY_FLASH_PAGE_ADDR;
    channels = (*(__IO uint32_t*)p_flash);
    p_flash += 4;
    for (int i = 0; i < channels; i++) {
			  chan_number[i] = (*(__IO uint32_t*)p_flash);
			  p_flash += 4;
        call_type[i] = (*(__IO uint32_t*)p_flash);
        p_flash += 4;
        call_number[i] = (*(__IO uint32_t*)p_flash);
        p_flash += 4;
    }
    FLASH_Lock();
}

void program(void) {

    USART_Configuration();
    USART_GPIO_Configuration();
    uint32_t temp;
    uint32_t err = 0;
    while (1) {
        if (rx_flag) {
            rx_flag = 0;
            set_cnt = 0;
            char delim[] = " ";
            char *p = strtok(uart_buff, delim);
            while (p != NULL) {
                if (set_cnt == 0) {
                    temp = atoi(p);
                    if (temp > 0 && temp < 7)
                        settings[set_cnt++] = temp;
                    else {
                        err = 1;
                        break;
                    }
                } else {
									  settings[set_cnt++] = atoi(p); 
                    settings[set_cnt++] = (int) *(p+1) ;
                    settings[set_cnt++] = atoi(p + 2);
                }
                p = strtok(NULL, delim);
                if (set_cnt == 19)
                    break;
            }
            if (err == 0) 
						{
                FLASH_Unlock();
                FLASH_ErasePage(MY_FLASH_PAGE_ADDR);
                uint32_t *source_addr = (void *) &settings;
                uint32_t *dest_addr = (uint32_t *) MY_FLASH_PAGE_ADDR;
                for (uint32_t i = 0; i < settings[0] * 3 + 1; i++) 
							  {
                    FLASH_ProgramWord((uint32_t) dest_addr, *source_addr);
                    source_addr++;
                    dest_addr++;
                }
                FLASH_Lock();
            } else {
                // error
            }

        }
    }
}

void FLASH_Init(void) {
    /* Next commands may be used in SysClock initialization function
         In this case using of FLASH_Init is not obligatorily */
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
}

void normal_keys_process(void)
{
    if ((keys_state != 0x3F) && !debounce) {
        if (keys_state != keys_old_state) {
            keys_debounce = keys_state;
            TIM6->CNT = 1;
            TIM6->CR1 |= TIM_CR1_CEN;
            debounce = 1;
        }
    }
    // after delay
    if (debounce == 2) {
        if (keys_state == keys_debounce) {
            keys_old_state = keys_state;
            switch (keys_state) {
            case 0x3E:
                channel_tx = 5;
                break;
            case 0x3D:
                channel_tx = 4;
                break;
            case 0x3B:
                channel_tx = 3;
                break;
            case 0x37:
                channel_tx = 2;
                break;
            case 0x2F:
                channel_tx = 1;
                break;
            case 0x1F:
                channel_tx = 0;
                break;
            default:
                break;
            }
            debounce = 0;
            GPIOA->ODR &= ~0x3F;
            GPIOA->ODR |= LEDs[channel_tx];
            call_req.pep.pld[0] =
                    (call_type[channel_tx] == 0x67) ? 1 : 0;
            memcpy((char*) call_req.pep.pld + 1,
                   (char*) (call_number + channel_tx), 4); // check!!!!!!!!!!!!!!!!
            if (current_chan != chan_number[channel_tx]) {
                packet_num++;
                zone_ch.packet_num[0] = packet_num >> 8; // audio route rx tx  page232
                zone_ch.packet_num[1] = packet_num & 0xFF;
                zone_ch.pep.pld[0] = 0; // set chan number
                zone_ch.pep.pld[3] = chan_number[channel_tx];
                zone_ch.pep.pld[4] = 0;
                memcpy(tx, (char*) (&zone_ch), 17);
                memcpy(tx + 17, (char*) (zone_ch.pep.pld),
                       zone_ch.pep.num_of_bytes[0]);
                memcpy(tx + 17 + zone_ch.pep.num_of_bytes[0],
                        (char*) (&zone_ch) + 21, 2);
                checksum(tx);
                USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
                USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
            }
        }
    }

    // read ptt
    if ((((GPIOC->IDR) & PDLkey) == 0) && !press_ptt && !ptt_debounce) {
        TIM5->CNT = 1;
        TIM5->CR1 |= TIM_CR1_CEN;
        ptt_debounce = 1;
    }
    // after delay
    if ((ptt_debounce == 2) && (((GPIOC->IDR) & PDLkey) == 0)
            && !press_ptt) {
        switch (state) {
        case CALL_RX_END: // quick answer
            GPIOC->ODR |= LEDmic;
            packet_num++;
            printf("+pptt\r\n"); //, packet_num);
            ptt_press_req.packet_num[0] = packet_num >> 8; // page 159
            ptt_press_req.packet_num[1] = packet_num & 0xFF;
            memcpy(tx, (char*) (&ptt_press_req), 17);
            memcpy(tx + 17, (char*) (ptt_press_req.pep.pld),
                   ptt_press_req.pep.num_of_bytes[0]);
            memcpy(tx + 17 + ptt_press_req.pep.num_of_bytes[0],
                    (char*) (&ptt_press_req) + 21, 2);
            checksum(tx);
            USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
            USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
            state = WAIT_CALL_REPORT;
            TIM4->CNT = 4000 - 300; // 300 ms
            TIM4->CR1 |= TIM_CR1_CEN;
            ptt_debounce = 0;
            press_ptt = 1;
            break;
        case TXT_NOTIFIC: // new call
            GPIOC->ODR |= LEDmic;
            packet_num++;
            printf("pptt\r\n"); //printf("pptt->%d\r\n", packet_num);
            call_req.packet_num[0] = packet_num >> 8; // page 230  pcm pmu  pma  0 8 0x78
            call_req.packet_num[1] = packet_num & 0xFF;
            memcpy(tx, (char*) (&call_req), 17);
            memcpy(tx + 17, (char*) (call_req.pep.pld),
                   call_req.pep.num_of_bytes[0]);
            memcpy(tx + 17 + call_req.pep.num_of_bytes[0],
                    (char*) (&call_req) + 21, 2);
            checksum(tx);
            USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
            state = WAIT_CALL_REPLY;
            TIM4->CNT = 4000 - 300; // 500ms
            TIM4->CR1 |= TIM_CR1_CEN;
            ptt_debounce = 0;
            press_ptt = 1;
            break;
        default:
            break;
        }

        //USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
        //USBH_Process(&USB_OTG_Core, &USB_Host);

    }
    //}
    if ((state == WAIT_STOP_CALL_REQ) && ((GPIOC->IDR) & PDLkey)
            && press_ptt) {
        GPIOC->ODR &= ~LEDmic;
        press_ptt = 0;
        packet_num++;
        printf("rptt\r\n");        //printf("rptt->%d\r\n", packet_num);
        ptt_release_req.packet_num[0] = packet_num >> 8;
        ptt_release_req.packet_num[1] = packet_num & 0xFF;
        memcpy(tx, (char*) (&ptt_release_req), 17);
        memcpy(tx + 17, (char*) (ptt_release_req.pep.pld),
               ptt_release_req.pep.num_of_bytes[0]);
        memcpy(tx + 17 + ptt_release_req.pep.num_of_bytes[0],
                (char*) (&ptt_release_req) + 21, 2);
        checksum(tx);
        USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
        state = WAIT_STOP_CALL_REQ;
        TIM4->CNT = 1;
        TIM4->CR1 |= TIM_CR1_CEN;

    }
}	

#ifdef USE_FULL_ASSERT
/**
 * @brief  assert_failed
 *         Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  File: pointer to the source file name
 * @param  Line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t * file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line
         * number,ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
         * line) */

    /* Infinite loop */
    while (1)
    {
    }
}

#endif

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
