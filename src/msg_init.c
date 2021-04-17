#include "messages.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"

hdpep_t pep;
hdpep_no_pld_t pep_npld;

hrnp_no_pld_t ack_msg;
hrnp_no_pep_t conn_req;
hrnp_no_pep_t close_req;
hrnp_no_pld_t pwr_up_chk_req;
hrnp_t status_change_req;
hrnp_t uart_config_req;
hrnp_t txt_notif_req;
hrnp_t txt_msg_open;
hrnp_t zone_ch;
hrnp_t serialNum_req;
hrnp_t chan_status;
hrnp_t radio_id;
hrnp_t call_req;
hrnp_t ptt_press_req;
hrnp_t ptt_release_req;
hrnp_t br_cast_req;
hrnp_t rtp_pld_req;
hrnp_t dig_audio_rx_req;
hrnp_t dig_audio_tx_req;
hrnp_t key_notific_req;
hrnp_t short_data_req;
extern unsigned short checksum(unsigned char *ip);
extern void checksum_pep_no_pld(hrnp_no_pld_t *ip);
extern void checksum_pep(hrnp_t *ip);
extern uint8_t tx[];
//uint8_t p[1] = { 0 };
uint8_t ptt_press[21];
uint8_t ptt_release[21];

//static int cs;

void init_msg(void)
{
	short_data_req.header = 0x7E;
				short_data_req.version = 0x04;
				short_data_req.block = 0;
				short_data_req.opcode = DATA;
				short_data_req.source = 0x20;
				short_data_req.dest = 0x10;
				short_data_req.packet_num[0] = 0;
				short_data_req.packet_num[1] = 3;
				short_data_req.length[0] = 0;
				short_data_req.length[1] = 19 + 12 + 1; // + pCount

				short_data_req.pep.MsgHdr = 0x09;
				short_data_req.pep.opcode[0] = 0x80;
				short_data_req.pep.opcode[1] = 0xAE; // short private ddata message (0xAF --> ack)
				short_data_req.pep.num_of_bytes[0] = 0;
				short_data_req.pep.num_of_bytes[1] = 12 + 1; // + 2 sensors

				short_data_req.pep.pld = (uint8_t*) malloc(short_data_req.pep.num_of_bytes[1]);

				short_data_req.pep.pld[0] = 0;  // req id
				short_data_req.pep.pld[1] = 0;
				short_data_req.pep.pld[2] = 0;
				short_data_req.pep.pld[3] = 0;

				short_data_req.pep.pld[4] = 10; // dest
				short_data_req.pep.pld[5] = 0;
				short_data_req.pep.pld[6] = 3;
				short_data_req.pep.pld[7] = 0xED; //  935 kss
				//short_data_req.pep.dest_ip[3] = 0xED; //  1005 kss

				short_data_req.pep.pld[8] =  10; // source
				short_data_req.pep.pld[9] =  0;
				short_data_req.pep.pld[10] = 3;
				short_data_req.pep.pld[11] = 0x85;// 901 

				//short_data_req.pep.pld[12]; // sensors
				//short_data_req.pep.pld[13]; // sensors
				
				short_data_req.pep.Checksum = 0x00;
				short_data_req.pep.MsgEnd = 0x03;
	
	key_notific_req.header = 0x7E;
	key_notific_req.version = 0x04;
	key_notific_req.block = 0;
	key_notific_req.opcode = DATA;
	key_notific_req.source = 0x20;
	key_notific_req.dest = 0x10;
	key_notific_req.packet_num[0] = 0;
	key_notific_req.packet_num[1] = 3;
	key_notific_req.length[0] = 0;
	key_notific_req.length[1] = (12 + 5) + 3 + 2;
	;
	key_notific_req.pep.MsgHdr = 0x02;
	key_notific_req.pep.opcode[0] = 0xE4;
	key_notific_req.pep.opcode[1] = 0x10;
	key_notific_req.pep.num_of_bytes[0] = 3;
	key_notific_req.pep.num_of_bytes[1] = 0;
	key_notific_req.pep.pld = (uint8_t*) malloc(
			key_notific_req.pep.num_of_bytes[0]);
	key_notific_req.pep.pld[0] = 1;  // req id
	key_notific_req.pep.pld[1] = 3;
	key_notific_req.pep.pld[2] = 2;
	key_notific_req.pep.Checksum = 0x00;
	key_notific_req.pep.MsgEnd = 0x03;

	ack_msg.header = 0x7E;
	ack_msg.version = 0x04;
	ack_msg.block = 0;
	ack_msg.opcode = DATA_ACK;
	ack_msg.source = 0x20;
	ack_msg.dest = 0x10;
	ack_msg.packet_num[0] = 0;
	ack_msg.packet_num[1] = 0;
	ack_msg.length[0] = 0;
	ack_msg.length[1] = 12;
	//checksum((char*)&ack_msg);

	close_req.header = 0x7E;
	close_req.version = 0x04;
	close_req.block = 0;
	close_req.opcode = CLOSE;
	close_req.source = 0x20;
	close_req.dest = 0x10;
	close_req.packet_num[0] = 0;
	close_req.packet_num[1] = 0;
	close_req.length[0] = 0;
	close_req.length[1] = 12;

	conn_req.header = 0x7E;
	conn_req.version = 0x04;
	conn_req.block = 0;
	conn_req.opcode = CONNECT;
	conn_req.source = 0x20;
	conn_req.dest = 0x10;
	conn_req.packet_num[0] = 0;
	conn_req.packet_num[1] = 0;
	conn_req.length[0] = 0;
	conn_req.length[1] = 12;
	//checksum((char*)&conn_req);

	pwr_up_chk_req.header = 0x7E;
	pwr_up_chk_req.version = 0x04;
	pwr_up_chk_req.block = 0;
	pwr_up_chk_req.opcode = DATA;
	pwr_up_chk_req.source = 0x20;
	pwr_up_chk_req.dest = 0x10;
	pwr_up_chk_req.packet_num[0] = 0;
	pwr_up_chk_req.packet_num[1] = 1;
	pwr_up_chk_req.length[0] = 0;
	pwr_up_chk_req.length[1] = 19;

	pwr_up_chk_req.pep_npld.MsgHdr = 0x02;
	pwr_up_chk_req.pep_npld.opcode[0] = 0xC6;
	pwr_up_chk_req.pep_npld.opcode[1] = 0x00;
	pwr_up_chk_req.pep_npld.num_of_bytes[0] = 0;
	pwr_up_chk_req.pep_npld.num_of_bytes[1] = 0;
	pwr_up_chk_req.pep_npld.Checksum = 0x00;
	pwr_up_chk_req.pep_npld.MsgEnd = 0x03;
	//checksum_pep_no_pld(&pwr_up_chk_req);
	//checksum((char*)&pwr_up_chk_req);

		status_change_req.header = 0x7E;
		status_change_req.version = 0x04;
		status_change_req.block = 0;
		status_change_req.opcode = DATA;
		status_change_req.source = 0x20;
		status_change_req.dest = 0x10;
		status_change_req.packet_num[0] = 0;
		status_change_req.packet_num[1] = 2;
		status_change_req.length[0] = 0;
		status_change_req.length[1] = 22;

		status_change_req.pep.MsgHdr = 0x02;
		status_change_req.pep.opcode[0] = 0xC7;
		status_change_req.pep.opcode[1] = 0x10;
		status_change_req.pep.num_of_bytes[0] = 3;
		status_change_req.pep.num_of_bytes[1] = 0;

		status_change_req.pep.pld = (uint8_t*) malloc(
				status_change_req.pep.num_of_bytes[0]);
		status_change_req.pep.pld[0] = 1;
		status_change_req.pep.pld[1] = 0x0C;
		status_change_req.pep.pld[2] = 1;

		status_change_req.pep.Checksum = 0x00;
		status_change_req.pep.MsgEnd = 0x03;

	txt_notif_req.header = 0x7E;
	txt_notif_req.version = 0x04;
	txt_notif_req.block = 0;
	txt_notif_req.opcode = DATA;
	txt_notif_req.source = 0x20;
	txt_notif_req.dest = 0x10;
	txt_notif_req.packet_num[0] = 0;
	txt_notif_req.packet_num[1] = 2;
	txt_notif_req.length[0] = 0;
	txt_notif_req.length[1] = 20;

	txt_notif_req.pep.MsgHdr = 0x02;
	txt_notif_req.pep.opcode[0] = 0xD0;
	txt_notif_req.pep.opcode[1] = 0x10;
	txt_notif_req.pep.num_of_bytes[0] = 1;
	txt_notif_req.pep.num_of_bytes[1] = 0;

	txt_notif_req.pep.pld = (uint8_t*) malloc(
			txt_notif_req.pep.num_of_bytes[0]);
	txt_notif_req.pep.pld[0] = 0;// no inform pereph

	txt_notif_req.pep.Checksum = 0x00;
	txt_notif_req.pep.MsgEnd = 0x03;

	//checksum_pep(&txt_notif_req);
	//checksum((unsigned char*)&txt_notif_req);

        txt_msg_open.header = 0x7E;
        txt_msg_open.version = 0x04;
        txt_msg_open.block = 0;
        txt_msg_open.opcode = DATA;
        txt_msg_open.source = 0x20;
        txt_msg_open.dest = 0x10;
        txt_msg_open.packet_num[0] = 0;
        txt_msg_open.packet_num[1] = 3;
        txt_msg_open.length[0] = 0;
        txt_msg_open.length[1] = 19 + 12 + 8;// + 38;

        txt_msg_open.pep.MsgHdr = 0x09;
        txt_msg_open.pep.opcode[0] = 0x80;
        txt_msg_open.pep.opcode[1] = 0xA1;
        txt_msg_open.pep.num_of_bytes[0] = 0;
        txt_msg_open.pep.num_of_bytes[1] = 12 + 8 ;//+ 38;
				
				txt_msg_open.pep.pld = (uint8_t*) malloc(txt_msg_open.pep.num_of_bytes[1]);

        txt_msg_open.pep.pld[0] = 0;  // req id
        txt_msg_open.pep.pld[1] = 0;
        txt_msg_open.pep.pld[2] = 0;
        txt_msg_open.pep.pld[3] = 0;

        txt_msg_open.pep.pld[4] = 10; // dest
        txt_msg_open.pep.pld[5] = 0;
				txt_msg_open.pep.pld[6] = 3;
				//txt_msg_open.pep.pld[7] = 0xA8; //  936 kss
        //txt_msg_open.pep.pld[7] = 0xA7; //  935 kss
				//txt_msg_open.pep.pld[7] = 0x93; // 915 raduga
				txt_msg_open.pep.pld[7] = 0xED; //  1005 xpt

        txt_msg_open.pep.pld[8] = 10;  // source
        txt_msg_open.pep.pld[9] = 0;
        txt_msg_open.pep.pld[10] = 3;
        txt_msg_open.pep.pld[11] = 0x85; // 901

        txt_msg_open.pep.pld[12] = 0x54; // message
        txt_msg_open.pep.pld[13] = 0x00;
				txt_msg_open.pep.pld[14] = 0x4C;
        txt_msg_open.pep.pld[15] = 0x00;
        txt_msg_open.pep.pld[16] = 0x4D;
				txt_msg_open.pep.pld[17] = 0x00;
				txt_msg_open.pep.pld[18] = 0x20;
				txt_msg_open.pep.pld[19] = 0x00;
        
//				txt_msg_open.pep.pld[20] = 0x30;
//				txt_msg_open.pep.pld[21] = 0x00;
//				txt_msg_open.pep.pld[22] = 0x20;
//				txt_msg_open.pep.pld[23] = 0x00;
//        
//				txt_msg_open.pep.pld[24] = 0x30;
//				txt_msg_open.pep.pld[25] = 0x00;
//				txt_msg_open.pep.pld[26] = 0x20;
//				txt_msg_open.pep.pld[27] = 0x00;
//        
//				txt_msg_open.pep.pld[28] = 0x30;
//				txt_msg_open.pep.pld[29] = 0x00;
//				txt_msg_open.pep.pld[30] = 0x20;
//				txt_msg_open.pep.pld[31] = 0x00;
//        
//				txt_msg_open.pep.pld[32] = 0x30;
//				txt_msg_open.pep.pld[33] = 0x00;
//				txt_msg_open.pep.pld[34] = 0x20;
//				txt_msg_open.pep.pld[35] = 0x00;
//        
//				txt_msg_open.pep.pld[36] = 0x30;
//				txt_msg_open.pep.pld[37] = 0x00;
//				txt_msg_open.pep.pld[38] = 0x20;
//				txt_msg_open.pep.pld[39] = 0x00;
//        
//				txt_msg_open.pep.pld[40] = 0x30;
//				txt_msg_open.pep.pld[41] = 0x00;
//				txt_msg_open.pep.pld[42] = 0x20;
//				txt_msg_open.pep.pld[43] = 0x00;
//        
//				txt_msg_open.pep.pld[44] = 0x30;
//				txt_msg_open.pep.pld[45] = 0x00;
//				txt_msg_open.pep.pld[46] = 0x20;
//				txt_msg_open.pep.pld[47] = 0x00;
//        
//				txt_msg_open.pep.pld[48] = 0x30;
//				txt_msg_open.pep.pld[49] = 0x00;
//				txt_msg_open.pep.pld[50] = 0x20;
//				txt_msg_open.pep.pld[51] = 0x00;
//        
//				txt_msg_open.pep.pld[52] = 0x30;
//				txt_msg_open.pep.pld[53] = 0x00;
//				txt_msg_open.pep.pld[54] = 0x20;
//				txt_msg_open.pep.pld[55] = 0x00;
//        
//				txt_msg_open.pep.pld[56] = 0x30;
//				txt_msg_open.pep.pld[57] = 0x00;
				
        txt_msg_open.pep.Checksum = 0x00;
        txt_msg_open.pep.MsgEnd = 0x03;

	serialNum_req.header = 0x7E;
	serialNum_req.version = 0x04;
	serialNum_req.block = 0;
	serialNum_req.opcode = DATA;
	serialNum_req.source = 0x20;
	serialNum_req.dest = 0x10;
	serialNum_req.packet_num[0] = 0;
	serialNum_req.packet_num[1] = 3;
	serialNum_req.length[0] = 0;
	serialNum_req.length[1] = (12 + 5) + 1 + 2;

	serialNum_req.pep.MsgHdr = 0x02;
	serialNum_req.pep.opcode[0] = 0x01;
	serialNum_req.pep.opcode[1] = 0x02;
	serialNum_req.pep.num_of_bytes[0] = 1;
	serialNum_req.pep.num_of_bytes[1] = 0;

	serialNum_req.pep.pld = (uint8_t*) malloc(
			serialNum_req.pep.num_of_bytes[0]);

	serialNum_req.pep.pld[0] = 0x02; // s/n

	serialNum_req.pep.Checksum = 0x00;
	serialNum_req.pep.MsgEnd = 0x03;

	zone_ch.header = 0x7E;
	zone_ch.version = 0x04;
	zone_ch.block = 0;
	zone_ch.opcode = DATA;
	zone_ch.source = 0x20;
	zone_ch.dest = 0x10;
	zone_ch.packet_num[0] = 0;
	zone_ch.packet_num[1] = 3;
	zone_ch.length[0] = 0;
	zone_ch.length[1] = 12 + 12;

	zone_ch.pep.MsgHdr = 0x02;
	zone_ch.pep.opcode[0] = 0xC4;
	zone_ch.pep.opcode[1] = 0x00;
	zone_ch.pep.num_of_bytes[0] = 5;
	zone_ch.pep.num_of_bytes[1] = 0;

	zone_ch.pep.pld = (uint8_t*) malloc(zone_ch.pep.num_of_bytes[0]);

	zone_ch.pep.pld[0] = 0x00; // message  0-set 1-read
	zone_ch.pep.pld[1] = 0x01;
	zone_ch.pep.pld[2] = 0x00;
	zone_ch.pep.pld[3] = 0x01;
	zone_ch.pep.pld[4] = 0x00;

	zone_ch.pep.Checksum = 0x00;
	zone_ch.pep.MsgEnd = 0x03;

	chan_status.header = 0x7E;
	chan_status.version = 0x04;
	chan_status.block = 0;
	chan_status.opcode = DATA;
	chan_status.source = 0x20;
	chan_status.dest = 0x10;
	chan_status.packet_num[0] = 0;
	chan_status.packet_num[1] = 3;
	chan_status.length[0] = 0;
	chan_status.length[1] = (12 + 5) + 2 + 2;

	chan_status.pep.MsgHdr = 0x02;
	chan_status.pep.opcode[0] = 0xE7;
	chan_status.pep.opcode[1] = 0x00;
	chan_status.pep.num_of_bytes[0] = 2;
	chan_status.pep.num_of_bytes[1] = 0;

	chan_status.pep.pld = (uint8_t*) malloc(chan_status.pep.num_of_bytes[0]);

	chan_status.pep.pld[0] = 0x07; // channel status
	chan_status.pep.pld[1] = 0x00;

	chan_status.pep.Checksum = 0x00;
	chan_status.pep.MsgEnd = 0x03;

	radio_id.header = 0x7E;
	radio_id.version = 0x04;
	radio_id.block = 0;
	radio_id.opcode = DATA;
	radio_id.source = 0x20;
	radio_id.dest = 0x10;
	radio_id.packet_num[0] = 0;
	radio_id.packet_num[1] = 3;
	radio_id.length[0] = 0;
	radio_id.length[1] = (12 + 5) + 1 + 2;

	radio_id.pep.MsgHdr = 0x02;
	radio_id.pep.opcode[0] = 0x52;
	radio_id.pep.opcode[1] = 0x04;
	radio_id.pep.num_of_bytes[0] = 1;
	radio_id.pep.num_of_bytes[1] = 0;

	radio_id.pep.pld = (uint8_t*) malloc(radio_id.pep.num_of_bytes[0]);

	radio_id.pep.pld[0] = 0x00; // radio id

	radio_id.pep.Checksum = 0x00;
	radio_id.pep.MsgEnd = 0x03;

	call_req.header = 0x7E;
	call_req.version = 0x04;
	call_req.block = 0;
	call_req.opcode = DATA;
	call_req.source = 0x20;
	call_req.dest = 0x10;
	call_req.packet_num[0] = 0;
	call_req.packet_num[1] = 3;
	call_req.length[0] = 0;
	call_req.length[1] = (12 + 5) + 5 + 2;

	call_req.pep.MsgHdr = 0x02;
	call_req.pep.opcode[0] = 0x41;
	call_req.pep.opcode[1] = 0x08;
	call_req.pep.num_of_bytes[0] = 5;
	call_req.pep.num_of_bytes[1] = 0;

	call_req.pep.pld = (uint8_t*) malloc(call_req.pep.num_of_bytes[0]);

	call_req.pep.pld[0] = 0x00; // call type  0-priv 1-gr  2-all
	call_req.pep.pld[1] = 0x01; // 1-0xffffffff radio id
	call_req.pep.pld[2] = 0x00;
	call_req.pep.pld[3] = 0x00;
	call_req.pep.pld[4] = 0x00;

	call_req.pep.Checksum = 0x00;
	call_req.pep.MsgEnd = 0x03;

	ptt_press[0] = 0x7E;
	ptt_press[1] = 4;
	ptt_press[2] = 0;
	ptt_press[3] = DATA;
	ptt_press[4] = 0x20;
	ptt_press[5] = 0x10;
	ptt_press[6] = 0;
	ptt_press[7] = 0;
	ptt_press[8] = 0;
	ptt_press[9] = 21;
	ptt_press[10] = 0;
	ptt_press[11] = 0;
	ptt_press[12] = 2;
	ptt_press[13] = 0x41;
	ptt_press[14] = 0;
	ptt_press[15] = 2;
	ptt_press[16] = 0;
	ptt_press[17] = 3;
	ptt_press[18] = 1;
	ptt_press[19] = 0;
	ptt_press[20] = 3;

	ptt_press_req.header = 0x7E;
	ptt_press_req.version = 0x04;
	ptt_press_req.block = 0;
	ptt_press_req.opcode = DATA;
	ptt_press_req.source = 0x20;
	ptt_press_req.dest = 0x10;
	ptt_press_req.packet_num[0] = 0;
	ptt_press_req.packet_num[1] = 0;
	ptt_press_req.length[0] = 0;
	ptt_press_req.length[1] = (12 + 5) + 2 + 2;

	ptt_press_req.pep.MsgHdr = 0x02;
	ptt_press_req.pep.opcode[0] = 0x41;
	ptt_press_req.pep.opcode[1] = 0x00;
	ptt_press_req.pep.num_of_bytes[0] = 2;
	ptt_press_req.pep.num_of_bytes[1] = 0;

	ptt_press_req.pep.pld = (uint8_t*) malloc(
			ptt_press_req.pep.num_of_bytes[0]);

	ptt_press_req.pep.pld[0] = 0x03; // ptt key
	ptt_press_req.pep.pld[1] = 0x01; // press

	ptt_press_req.pep.Checksum = 0x00;
	ptt_press_req.pep.MsgEnd = 0x03;
	ptt_release[0] = 0x7E;
	ptt_release[1] = 4;
	ptt_release[2] = 0;
	ptt_release[3] = DATA;
	ptt_release[4] = 0x20;
	ptt_release[5] = 0x10;
	ptt_release[6] = 0;
	ptt_release[7] = 0;
	ptt_release[8] = 0;
	ptt_release[9] = 21;
	ptt_release[10] = 0;
	ptt_release[11] = 0;
	ptt_release[12] = 2;
	ptt_release[13] = 0x41;
	ptt_release[14] = 0;
	ptt_release[15] = 2;
	ptt_release[16] = 0;
	ptt_release[17] = 3;
	ptt_release[18] = 0;
	ptt_release[19] = 0;
	ptt_release[20] = 3;

	ptt_release_req.header = 0x7E;
	ptt_release_req.version = 0x04;
	ptt_release_req.block = 0;
	ptt_release_req.opcode = DATA;
	ptt_release_req.source = 0x20;
	ptt_release_req.dest = 0x10;
	ptt_release_req.packet_num[0] = 0;
	ptt_release_req.packet_num[1] = 3;
	ptt_release_req.length[0] = 0;
	ptt_release_req.length[1] = (12 + 5) + 2 + 2;

	ptt_release_req.pep.MsgHdr = 0x02;
	ptt_release_req.pep.opcode[0] = 0x41;
	ptt_release_req.pep.opcode[1] = 0x00;
	ptt_release_req.pep.num_of_bytes[0] = 2;
	ptt_release_req.pep.num_of_bytes[1] = 0;

	ptt_release_req.pep.pld = (uint8_t*) malloc(
			ptt_release_req.pep.num_of_bytes[0]);

	ptt_release_req.pep.pld[0] = 0x03; // ptt key
	ptt_release_req.pep.pld[1] = 0x00; // release

	ptt_release_req.pep.Checksum = 0x00;
	ptt_release_req.pep.MsgEnd = 0x03;

	br_cast_req.header = 0x7E;
	br_cast_req.version = 0x04;
	br_cast_req.block = 0;
	br_cast_req.opcode = DATA;
	br_cast_req.source = 0x20;
	br_cast_req.dest = 0x10;
	br_cast_req.packet_num[0] = 0;
	br_cast_req.packet_num[1] = 3;
	br_cast_req.length[0] = 0;
	br_cast_req.length[1] = (12 + 5) + 5 + 2;

	br_cast_req.pep.MsgHdr = 0x02;
	br_cast_req.pep.opcode[0] = 0xC9;
	br_cast_req.pep.opcode[1] = 0x10;
	br_cast_req.pep.num_of_bytes[0] = 5;
	br_cast_req.pep.num_of_bytes[1] = 0;

	br_cast_req.pep.pld = (uint8_t*) malloc(br_cast_req.pep.num_of_bytes[0]);

	br_cast_req.pep.pld[0] = 0x02; // size
	br_cast_req.pep.pld[1] = 0x00; // release
	br_cast_req.pep.pld[2] = 0x01; // ptt key
	br_cast_req.pep.pld[3] = 0x01; // release
	br_cast_req.pep.pld[4] = 0x01; // ptt key

	br_cast_req.pep.Checksum = 0x00;
	br_cast_req.pep.MsgEnd = 0x03;

	rtp_pld_req.header = 0x7E;
	rtp_pld_req.version = 0x04;
	rtp_pld_req.block = 0;
	rtp_pld_req.opcode = DATA;
	rtp_pld_req.source = 0x20;
	rtp_pld_req.dest = 0x10;
	rtp_pld_req.packet_num[0] = 0;
	rtp_pld_req.packet_num[1] = 3;
	rtp_pld_req.length[0] = 0;
	rtp_pld_req.length[1] = (12 + 5) + 5 + 2;
	rtp_pld_req.pep.MsgHdr = 0x02;
	rtp_pld_req.pep.opcode[0] = 0x19;
	rtp_pld_req.pep.opcode[1] = 0x04;
	rtp_pld_req.pep.num_of_bytes[0] = 5;
	rtp_pld_req.pep.num_of_bytes[1] = 0;
	rtp_pld_req.pep.pld = (uint8_t*) malloc(rtp_pld_req.pep.num_of_bytes[0]);
	rtp_pld_req.pep.pld[0] = 0x78; // PCM
	rtp_pld_req.pep.Checksum = 0x00;
	rtp_pld_req.pep.MsgEnd = 0x03;

	dig_audio_rx_req.header = 0x7E;
	dig_audio_rx_req.version = 0x04;
	dig_audio_rx_req.block = 0;
	dig_audio_rx_req.opcode = DATA;
	dig_audio_rx_req.source = 0x20;
	dig_audio_rx_req.dest = 0x10;
	dig_audio_rx_req.packet_num[0] = 0;
	dig_audio_rx_req.packet_num[1] = 3;
	dig_audio_rx_req.length[0] = 0;
	dig_audio_rx_req.length[1] = (12 + 5) + 2 + 2;
	dig_audio_rx_req.pep.MsgHdr = 0x02;
	dig_audio_rx_req.pep.opcode[0] = 0xDF;
	dig_audio_rx_req.pep.opcode[1] = 0x00;
	dig_audio_rx_req.pep.num_of_bytes[0] = 2;
	dig_audio_rx_req.pep.num_of_bytes[1] = 0;
	dig_audio_rx_req.pep.pld = (uint8_t*) malloc(
			dig_audio_rx_req.pep.num_of_bytes[0]);
	dig_audio_rx_req.pep.pld[0] = 0x01; //  rx
	dig_audio_rx_req.pep.pld[1] = 0x00; //  from air  change to 0 from 1
	dig_audio_rx_req.pep.Checksum = 0x00;
	dig_audio_rx_req.pep.MsgEnd = 0x03;
	
	
	

//		dig_audio_rx_req.header = 0x7E;
//    dig_audio_rx_req.version = 0x04;
//    dig_audio_rx_req.block = 0;
//    dig_audio_rx_req.opcode = DATA;
//    dig_audio_rx_req.source = 0x20;
//    dig_audio_rx_req.dest = 0x10;
//    dig_audio_rx_req.packet_num[0] = 0;
//    dig_audio_rx_req.packet_num[1] = 0;
//    dig_audio_rx_req.length[0] = 0;
//    dig_audio_rx_req.length[1] = (12 + 5) + 2 + 1;
//    dig_audio_rx_req.pep.MsgHdr = 0x02;
//    dig_audio_rx_req.pep.opcode[0] = 0xD6;
//    dig_audio_rx_req.pep.opcode[1] = 0x00;
//    dig_audio_rx_req.pep.num_of_bytes[0] = 1;
//    dig_audio_rx_req.pep.num_of_bytes[1] = 0;
//    dig_audio_rx_req.pep.pld = (uint8_t*) malloc(dig_audio_rx_req.pep.num_of_bytes[0]);
//    dig_audio_rx_req.pep.pld[0] = 0x00; //  no audio signal is sent to the peripheral equipment.
//    //dig_audio_rx_req.pep.pld[1] = 0x00; //  from air
//    dig_audio_rx_req.pep.Checksum = 0x00;
//    dig_audio_rx_req.pep.MsgEnd = 0x03;

	dig_audio_tx_req.header = 0x7E;
	dig_audio_tx_req.version = 0x04;
	dig_audio_tx_req.block = 0;
	dig_audio_tx_req.opcode = DATA;
	dig_audio_tx_req.source = 0x20;
	dig_audio_tx_req.dest = 0x10;
	dig_audio_tx_req.packet_num[0] = 0;
	dig_audio_tx_req.packet_num[1] = 0;
	dig_audio_tx_req.length[0] = 0;
	dig_audio_tx_req.length[1] = (12 + 5) + 2 + 2;
	dig_audio_tx_req.pep.MsgHdr = 0x02;
	dig_audio_tx_req.pep.opcode[0] = 0xDF;
	dig_audio_tx_req.pep.opcode[1] = 0x00;
	dig_audio_tx_req.pep.num_of_bytes[0] = 2;
	dig_audio_tx_req.pep.num_of_bytes[1] = 0;
	dig_audio_tx_req.pep.pld = (uint8_t*) malloc(
			dig_audio_tx_req.pep.num_of_bytes[0]);
	dig_audio_tx_req.pep.pld[0] = 0x00; //  tx
	dig_audio_tx_req.pep.pld[1] = 0x01; //  from air
	dig_audio_tx_req.pep.Checksum = 0x00;
	dig_audio_tx_req.pep.MsgEnd = 0x03;

//		dig_audio_tx_req.header = 0x7E;
//    dig_audio_tx_req.version = 0x04;
//    dig_audio_tx_req.block = 0;
//    dig_audio_tx_req.opcode = DATA;
//    dig_audio_tx_req.source = 0x20;
//    dig_audio_tx_req.dest = 0x10;
//    dig_audio_tx_req.packet_num[0] = 0;
//    dig_audio_tx_req.packet_num[1] = 0;
//    dig_audio_tx_req.length[0] = 0;
//    dig_audio_tx_req.length[1] = (12 + 5) + 2 + 1;
//    dig_audio_tx_req.pep.MsgHdr = 0x02;
//    dig_audio_tx_req.pep.opcode[0] = 0xD7;
//    dig_audio_tx_req.pep.opcode[1] = 0x00;
//    dig_audio_tx_req.pep.num_of_bytes[0] = 1;
//    dig_audio_tx_req.pep.num_of_bytes[1] = 0;
//    dig_audio_tx_req.pep.pld = (uint8_t*) malloc(dig_audio_tx_req.pep.num_of_bytes[0]);
//    dig_audio_tx_req.pep.pld[0] = 0x06; //  0x06: Switch external MIC output to RF.
//    //dig_audio_tx_req.pep.pld[1] = 0x00; //  from air
//    dig_audio_tx_req.pep.Checksum = 0x00;
//    dig_audio_tx_req.pep.MsgEnd = 0x03;

}

