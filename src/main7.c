/**
 ******************************************************************************
 * @file    main.c
 * @author  MCD Application Team
 * @version V2.2.1
 * @date    17-March-2018
 * @brief   USB Host CDC class demo main file
 ******************************************************************************
 
 */

#include "messages.h"
#include "stdlib.h"
#include "stm32f10x_tim.h"
#include "string.h"
#include "usb_bsp.h"
#include "usbh_cdc_core.h"
#include "usbh_core.h"
#include "usbh_usr.h"

#define HSE_8MHZ 1//установить в опциях С/C++ если кварц = 8 MHz
//#define DEBUG
#define UCReq 1
//#define TXT_MSG

// добавил проверку !press_ptt  !press_key при нажатии клавиш и ртт

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
extern hrnp_t short_data_req;
extern void init_msg(void);
extern volatile uint32_t timer6_exp;
extern void checksum(uint8_t[]);
extern void init_pereph(void);
extern void USART_Configuration(void);
extern void USART_GPIO_Configuration(void);
extern void init_portb(void);
extern uint8_t len_rx;
extern void odbProcess(void);
extern void HAL_Delay(uint16_t);
extern char* AT4;
extern uint8_t len_tx;

#define RESET 4
#define ENABLE 5
//void read_data(void);
//void program(void);
//void normal_keys_process(void);
//void extend_key_process(void);
//void prog_mode(void);
//void read_mode(void);

#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000
//#define MY_FLASH_PAGE_ADDR 0x803FC00
//#define MY_FLASH_PAGE_ADDR_MODE 0x803F000
//#define SETTINGS_WORDS 12

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

enum
{
   S_CHNG_NOTIFIC = 16,
   ZONE_CHANNEL
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined(__ICCARM__) /* !< IAR Compiler */
#pragma data_alignment = 4
#endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_Core __ALIGN_END;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined(__ICCARM__) /* !< IAR Compiler */
#pragma data_alignment = 4
#endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USBH_HOST USB_Host __ALIGN_END;

//uint16_t keys_old_state;
//uint16_t keys_debounce;
//uint16_t keys_state;
//uint32_t channels = 2;
//uint32_t call_type[6] = {'G', 'G', 'G', 'G', 'G', 'G'};
//uint32_t call_number[6] = {10, 20, 104, 20, 30, 40};
//uint32_t chan_number[6] = {1, 1, 104, 20, 30, 40};

uint32_t settings[13];
uint32_t set_cnt;
volatile uint8_t rx_flag;
char uart_buff[128];
uint32_t cnt_uart;
//uint8_t chan_busy;
//uint8_t current_chan;
//uint8_t func_key = 0;       // выбор режима работы
//uint8_t enable_ptt = 0;     // кнопка работает как ртт
//uint8_t press_key = 0;      // кнопка канала нажата
//uint8_t busy_channel = 0;   // канал на котором в данный момент идет связь.Нельзя переключить , пока сеанс не закончен(0-свободен,  1..6 занят)
//uint8_t change_channel = 0; // было изменение канала

//uint8_t LEDS_CHANNELS[6] = {0x20}; // 30.07 добавлено
//uint8_t ch = 0x20;

//extern volatile uint8_t tx_enable;
volatile uint32_t msg_rx;
uint32_t ptt_debounce;
uint32_t debounce;
uint32_t prg;
uint32_t channel_rx;
uint32_t channel_tx;
//uint16_t LEDmic = 0x200;
//uint16_t PDLkey = 0x100;
//uint16_t LEDs[] = {0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
//uint16_t KEYs[] = {0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
uint8_t start = 0;
uint32_t state;
uint8_t tx[128];
uint8_t rx[64];
uint16_t packet_num = 0;
uint32_t press_ptt = 0;
uint32_t rx_id;
uint32_t temp;
uint32_t err = 0;
volatile uint8_t cnt_tx;
//uint8_t sensor_counter;
//uint16_t sensors_mem = 0x0733;
//uint16_t sensors_now;
//uint16_t sensors_dbn;
//uint8_t disable_sensors_tx;
uint32_t req_id; // для текстовых сообщений увеличивается на 1.
uint8_t sensor_flag;
//uint16_t mask = 0x07F3;
uint8_t is_link;
///char *p;
//char delim[] = " ";
//char str_ok[] = " OK!\n";
//char str_err[] = " ERROR!\n";
//char str_epm[] = " ENTER PROGRAMMING MODE\n";
//void send_string(char s[]);
GPIO_InitTypeDef GPIO_InitStructure;
int main(void)
{

   state = CONN;
   //init_msg();
   init_pereph();

   //memcpy(tx, (char *)&conn_req, conn_req.length[1]); // see timer irq every 300ms
   //checksum(tx);
   SysTick_Config(72000000UL/1000);
   USBH_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USB_Host, &CDC_cb, &USR_Callbacks);
   //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, DISABLE);
   SET_BIT(USART1->CR1, USART_CR1_RXNEIE);	 
   //GPIOC->ODR |= 0x01; // usb 5V enable
   //TIM7->PSC = 36000 - 1; // 500 us
   //TIM7->ARR = 10000; // 5sec
   //TIM7->CNT = 1;
 	 GPIOB->ODR |= 0x03;
	 HAL_Delay(2000);
	 GPIOB->ODR &= ~0x01;
	 HAL_Delay(1000);
	 GPIOB->ODR |= 0x01;
 	  HAL_Delay(500);
	 
	 len_tx = 4;
	 memcpy(tx, AT4, len_tx);
	 TIM3->CNT = 1;
	 TIM3->ARR = 4000; //  1sec
	 cnt_tx = 0;
	 state = INIT_STATE;
	 
	 while(1){
		if((state == INIT_STATE) && !cnt_tx )
			{
				printf("AT->");
				GPIOB->ODR &= ~0x01;
				HAL_Delay(100);
				GPIOB->ODR |= 0x01;
				TIM3->CR1 |= TIM_CR1_CEN;
				cnt_tx = 1;
			} 
		if (rx_flag && (len_rx >= 2))
    {
			rx_flag = 0;
   		 if((state == INIT_STATE) && (rx[0] != 'O') && !cnt_tx)
				 {
						printf("AT->");
						TIM3->CR1 |= TIM_CR1_CEN;
						cnt_tx = 1;
				 }
				 else 
				   odbProcess();
			}
   }
   while (1)
   {
      USBH_Process(&USB_OTG_Core, &USB_Host);

      if (!start && (0 != USB_Host.device_prop.address))
      {
         start = 1;
         TIM3->CR1 |= TIM_CR1_CEN; // 3sec delay
         GPIOA->ODR |= 0x3F;
      }

      if (rx[0])
      {
         rx[0] = 0;

#ifdef DEBUG
         printf("%x%x-%d\r\n", rx[13], rx[14], state);
#endif

         switch (state)
         {
         case CONN:
            if (rx[3] == 0xFD)
            {
               packet_num++;
               pwr_up_chk_req.packet_num[0] = packet_num >> 8;
               pwr_up_chk_req.packet_num[1] = packet_num & 0xFF;
               memcpy(tx, (char *)&pwr_up_chk_req,
                   pwr_up_chk_req.length[1]);
               checksum(tx);
               USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
               state = PWR_UP;
            }
            else
            {
#ifdef DEBUG
               printf("connect fail!\r\n");
#endif
            }
            break;

         case PWR_UP:
            if ((rx[13] == 0xC6) && (rx[14] == 0x80))
            {
               packet_num++;
               txt_notif_req.packet_num[0] = packet_num >> 8;
               txt_notif_req.packet_num[1] = packet_num & 0xFF;
               memcpy(tx, (char *)&txt_notif_req, 17);
               memcpy(tx + 17, (char *)txt_notif_req.pep.pld,
                   txt_notif_req.pep.num_of_bytes[0]);
               memcpy(tx + 17 + txt_notif_req.pep.num_of_bytes[0],
                   (char *)(&txt_notif_req) + 21, 2);
               checksum(tx);
               USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7); //
               state = KEY_NOTIFIC;
            }
            else
            {
#ifdef DEBUG
               printf("pwr up chk fail!\r\n");
#endif
            }
            break;

         case KEY_NOTIFIC: //3
            if ((rx[13] == 0xD0) && (rx[14] == 0x80))
            {
               is_link = 1;
               state = TXT_NOTIFIC;
               GPIOC->ODR |= 0x02;
               TIM7->CR1 |= TIM_CR1_CEN;
//               RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//               GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
//               GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//               GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//               GPIO_Init(GPIOB, &GPIO_InitStructure);
//               RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); // тк РВ3 не работает то подключили ножку рядом с ним PD2
//               GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
//               GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//               GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//               GPIO_Init(GPIOD, &GPIO_InitStructure);
            }
            else
            {
#ifdef DEBUG
               printf("txt notif fail!\r\n");
#endif
            }
            break;
         case TXT_NOTIFIC:
         {
         }
         }
         USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
      }
      if (rx_flag && (len_rx >= 2))
      {
         msg_rx = 0;
				 odbProcess();
//         sensor_flag = 0;
//         sensor_counter++;
//         sensors_now = (GPIOD->IDR & 0x04) | ((GPIOB->IDR & mask) & 0x03) + (((GPIOB->IDR & mask) >> 1) & 0x18) + (((GPIOB->IDR & mask) >> 3) & 0xE0) + (((GPIOB->IDR & mask) << 2) & 0x300);

//         if (is_link && (sensors_now != sensors_mem)) // сравним с тем , что было раньше
//         {
//            GPIOC->ODR |= 0x04;
//            sensor_counter = 0;
//            for (int i = 0; i < 100000; i++)
//               ;
//            sensors_dbn = (GPIOD->IDR & 0x04) | ((GPIOB->IDR & mask) & 0x03) + (((GPIOB->IDR & mask) >> 1) & 0x18) + (((GPIOB->IDR & mask) >> 3) & 0xE0);
//            if (sensors_now == sensors_dbn)
//            {
//               sensors_mem = sensors_now;
//#ifdef TXT_MSG
//               txt_msg_open.pep.pld[20] = 0x30 + (sensors_now & 0x01);
//               txt_msg_open.pep.pld[24] = 0x30 + ((sensors_now >> 1) & 0x01);
//               txt_msg_open.pep.pld[28] = 0x30 + ((sensors_now >> 3) & 0x01);
//               txt_msg_open.pep.pld[32] = 0x30 + ((sensors_now >> 4) & 0x01);
//               txt_msg_open.pep.pld[36] = 0x30 + ((sensors_now >> 5) & 0x01);
//               txt_msg_open.pep.pld[40] = 0x30 + ((sensors_now >> 6) & 0x01);
//               txt_msg_open.pep.pld[44] = 0x30 + ((sensors_now >> 7) & 0x01);
//               txt_msg_open.pep.pld[48] = 0x30 + ((sensors_now >> 8) & 0x01);
//               txt_msg_open.pep.pld[52] = 0x30 + ((sensors_now >> 9) & 0x01);
//               txt_msg_open.pep.pld[56] = 0x30 + ((sensors_now >> 10) & 0x01);
//#else
//               short_data_req.pep.pld[12] = sensors_now & 0xFF;
//               ;
//#endif
//               packet_num++;

//#ifdef TXT_MSG
//               txt_msg_open.packet_num[0] = packet_num >> 8;
//               txt_msg_open.packet_num[1] = packet_num & 0xFF;
//               txt_msg_open.pep.pld[0] = req_id >> 24; // req id
//               txt_msg_open.pep.pld[1] = req_id >> 16;
//               txt_msg_open.pep.pld[2] = req_id >> 8;
//               txt_msg_open.pep.pld[3] = req_id & 0xFF;
//               while (cnt_tx != 0xFF)
//                  ;
//               memcpy(tx, (char *)&txt_msg_open, 17);
//               memcpy(tx + 17, (char *)txt_msg_open.pep.pld, txt_msg_open.pep.num_of_bytes[1]);
//               memcpy(tx + 17 + txt_msg_open.pep.num_of_bytes[1], (char *)&txt_msg_open + 21, 2);
//               checksum(tx);
//#else
//               short_data_req.packet_num[0] = packet_num >> 8;
//               short_data_req.packet_num[1] = packet_num & 0xFF;
//               short_data_req.pep.pld[0] = req_id >> 24; // req id
//               short_data_req.pep.pld[1] = req_id >> 16;
//               short_data_req.pep.pld[2] = req_id >> 8;
//               short_data_req.pep.pld[3] = req_id & 0xFF;
//               req_id++;
//               memcpy(tx, (char *)(&short_data_req), 17);
//               memcpy(tx + 17, (char *)short_data_req.pep.pld, 12 + 1);
//               memcpy(tx + 17 + 12 + 1, (char *)&short_data_req + 21, 2);
//               checksum(tx);
//#endif
//               USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
//            }
//         }

//         if (sensor_counter == 6) //
//         {
//            GPIOC->ODR |= 0x04;
//            for (int i = 0; i < 100000; i++)
//               ;
//            sensor_counter = 0;
//            {
//#ifdef TXT_MSG
//               txt_msg_open.pep.pld[20] = 0x30 + (sensors_now & 0x01);
//               txt_msg_open.pep.pld[24] = 0x30 + ((sensors_now >> 1) & 0x01);
//               txt_msg_open.pep.pld[28] = 0x30 + ((sensors_now >> 3) & 0x01);
//               txt_msg_open.pep.pld[32] = 0x30 + ((sensors_now >> 4) & 0x01);
//               txt_msg_open.pep.pld[36] = 0x30 + ((sensors_now >> 5) & 0x01);
//               txt_msg_open.pep.pld[40] = 0x30 + ((sensors_now >> 6) & 0x01);
//               txt_msg_open.pep.pld[44] = 0x30 + ((sensors_now >> 7) & 0x01);
//               txt_msg_open.pep.pld[48] = 0x30 + ((sensors_now >> 8) & 0x01);
//               txt_msg_open.pep.pld[52] = 0x30 + ((sensors_now >> 9) & 0x01);
//               txt_msg_open.pep.pld[56] = 0x30 + ((sensors_now >> 10) & 0x01);
//#else
//               short_data_req.pep.pld[12] = sensors_now & 0xFF;
//#endif

//               packet_num++;
//#ifdef TXT_MSG
//               txt_msg_open.packet_num[0] = packet_num >> 8;
//               txt_msg_open.packet_num[1] = packet_num & 0xFF;
//               txt_msg_open.pep.pld[0] = req_id >> 24; // req id
//               txt_msg_open.pep.pld[1] = req_id >> 16;
//               txt_msg_open.pep.pld[2] = req_id >> 8;
//               txt_msg_open.pep.pld[3] = req_id & 0xFF;
//               req_id++;
//               //while(cnt_tx != 0xFF);
//               memcpy(tx, (char *)&txt_msg_open, 17);
//               memcpy(tx + 17, (char *)txt_msg_open.pep.pld, txt_msg_open.pep.num_of_bytes[1]);
//               memcpy(tx + 17 + txt_msg_open.pep.num_of_bytes[1], (char *)&txt_msg_open + 21, 2);
//               checksum(tx);
//#else
//               short_data_req.packet_num[0] = packet_num >> 8;
//               short_data_req.packet_num[1] = packet_num & 0xFF;

//               short_data_req.pep.pld[0] = req_id >> 24; // req id
//               short_data_req.pep.pld[1] = req_id >> 16;
//               short_data_req.pep.pld[2] = req_id >> 8;
//               short_data_req.pep.pld[3] = req_id & 0xFF;
//               req_id++;

//               memcpy(tx, (char *)(&short_data_req), 17);
//               memcpy(tx + 17, (char *)short_data_req.pep.pld, 12 + 1);
//               memcpy(tx + 17 + 12 + 1, (char *)&short_data_req + 21, 2);
//               checksum(tx);
//#endif
               USBH_BulkSendData(&USB_OTG_Core, tx, tx[9], 7);
//            }
//         }
         USBH_BulkReceiveData(&USB_OTG_Core, rx, 64, 5);
         GPIOC->ODR &= ~0x04;
      }
			else if(msg_rx && (len_rx < 2))
					msg_rx = 0;
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
void assert_failed(uint8_t *file, uint32_t line)
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
