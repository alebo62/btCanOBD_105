#ifndef MESSAGES_H
#define MESSAGES_H
#include "stdint.h"
//#include "usbclass.h"
//#include "usbclass.h"
typedef enum {
    INIT_STATE = 0,
    CONN,
    PWR_UP,
    KEY_NOTIFIC,
	  BRDCST_NOTIFIC,
	  RTP_PAYLOAD,
	  ROUTE_RX,
		ROUTE_TX,
    TXT_NOTIFIC, // 7
    WAIT_CALL_REPLY, // 8
		WAIT_CALL_REPORT, // 9
		WAIT_STOP_CALL_REQ, // 10
		WAIT_STOP_CALL_HANG,  // 11 
		WAIT_STOP_CALL_END, // 12
	  CALL_RX_HANG,
	  CALL_RX_END	
} Device_State;



typedef enum {
    DATA = 0x00,
    CONNECT = 0xFE,//Slave uses this Opcode to connect to HRNP radio network.
    ACCEPT = 0xFD,// Master uses this Opcode to accept the connection request from slave.
    REJECT = 0xFC, //Master uses this Opcode to reject the connection request sent from slave if the data verification failed.
    CLOSE = 0xFB, // Slave/Master uses this Opcode to Close connection request
    CLOSE_ACK = 0xFA, // Answer to CLOSE request. NULL DATA 0x00 Send a HRNP data packet. Data[N]
    DATA_ACK = 0x10, // Answer to a HRNP data packet. NULL
    VOICE = 0x01
} HRNP_opcode_types;

#pragma pack(1)
typedef struct __attribute__((__packed__))
{
    uint8_t MsgHdr;
    uint8_t opcode[2];
    uint8_t num_of_bytes[2];
    uint8_t *pld;
    uint8_t Checksum;
    uint8_t MsgEnd;
} hdpep_t;

//typedef struct __attribute__((__packed__))
//{
//    uint8_t MsgHdr;
//    uint8_t opcode[2];
//    uint8_t num_of_bytes[2];
//    uint8_t pld[17];
//    uint8_t Checksum;
//    uint8_t MsgEnd;
//} hdpep17_t;

typedef struct
{
    uint8_t MsgHdr;
    uint8_t opcode[2];
    uint8_t num_of_bytes[2];
    uint8_t Checksum;
    uint8_t MsgEnd;
} __attribute__((__packed__)) hdpep_no_pld_t;

typedef struct
{
    uint8_t header;// = 0x7E;
    uint8_t version;// = 0x04;
    uint8_t block;
    uint8_t opcode;
    uint8_t source;
    uint8_t dest;
    uint8_t packet_num[2];
    uint8_t length[2];
    uint8_t check_sum[2];
    hdpep_t pep;// 19+...
}__attribute__((__packed__)) hrnp_t;

//typedef struct __attribute__((__packed__))
//{
//    uint8_t header;// = 0x7E;
//    uint8_t version;// = 0x04;
//    uint8_t block;
//    HRNP_opcode_types opcode;
//    uint8_t source;
//    uint8_t dest;
//    uint8_t packet_num[2];
//    uint8_t length[2];
//    uint8_t check_sum[2];
//    hdpep17_t pep17;// 19+...
//} hrnp17_t;


typedef struct
{
    uint8_t header;// = 0x7E;
    uint8_t version;// = 0x04;
    uint8_t block;
    uint8_t opcode;
    uint8_t source;
    uint8_t dest;
    uint8_t packet_num[2];
    uint8_t length[2];
    uint8_t check_sum[2];
    hdpep_no_pld_t pep_npld; // 19
} __attribute__((__packed__)) hrnp_no_pld_t;

typedef struct
{
    uint8_t header;// = 0x7E;
    uint8_t version;// = 0x04;
    uint8_t block;
    uint8_t opcode;
    uint8_t source;
    uint8_t dest;
    uint8_t packet_num[2];
    uint8_t length[2];
    uint8_t check_sum[2];
} __attribute__((__packed__)) hrnp_no_pep_t; // 12

typedef struct __attribute__((__packed__))
{
    uint16_t call_req;//0x0841
    uint8_t  type;
    uint32_t radio_id;

} call_req_t;
typedef struct __attribute__((__packed__))
{
    uint16_t call_rep;//0x0841;//
    uint8_t  result;
} call_rep_t;

//uint16_t call_req_remove = 0x0842;

typedef struct __attribute__((__packed__))
{
    uint16_t call_rep;//0x8842;//
    uint8_t  result;
} call_remove_rep_t;

/*

7  ACK control bit: to determine whether to give an ACK.
         The control bit is valid during private message transmission only.
        0: ACK is required.
        1: ACK is not required.

6  Option field flag: to determine whether to enable “Option field”.
                It is used combine with “option len”.
        0: “option len” and “option field” are disabled.
        1: “option len” and “option field” are enabled.
0~5          Reserved

The low 8 bits is used to define the service type.
Low 8 bits  Payload

0xA1(SendPrivateMsg)      				RequestID[4] + DestIP[4]  + SrcIP[4] + TMData[n]
0xA2(SendPrivateMsg_ACK)  				RequestID[4] + DestIP[4]  + SrcIP[4] + Result[1]

0xB1(SendGroupMsg)        				RequestID[4] + GroupID[4] + SrcIP[4] + TMData[n]
0xB2(SendGroupMsg_ACK)    				RequestID[4] + GroupID [4]+ Result[1]

0xA3(SendPrivatePriorityMsg)  		RequestID[4] + DestIP[4]  + SrcIP[4] + TMData[n]
0xA4(SendPrivatePriorityMsg_ACK)  RequestID[4] + DestIP[4]  + SrcIP[4] + Result[1]

0xB3(SendGroupPriorityFastMsg)  	RequestID[4] + GroupID[4] + SrcIP[4] + TMData[n]
0xB4(SendGroupPriorityFastMsg_ACK)RequestID[4] + GroupID [4]+ Result[1]

0xB5(SendGroupPriorityMsg)  			RequestID[4] + GroupID[4] + SrcIP[4] + TMData[n]
0xB6(SendGroupPriorityMsg_ACK)    RequestID[4] + GroupID [4]+ Result[1]

0xB7(SendGroupFastMsg)  					RequestID[4] + GroupID[4] + SrcIP[4] + TMData[n]
0xB8(SendGroupFastMsg _ACK)  			RequestID[4] + GroupID [4]+ Result[1]

0xAE(SendPrivateShortData)  			RequestID[4] + DestIP[4]  + SrcIP[4] + ShortData[n]
0xAF(SendPrivateShortDataACK)  		RequestID[4] + DestIP[4]  + SrcIP[4] + Result[1]

0xBE(SendGroupShortData)  				RequestID[4] + GroupID[4] + SrcIP[4] + ShortData[n]
0xBF(SendGroupShortDataACK)  			RequestID[4] + GroupID [4]+ Result[1]

*/



typedef enum
{
    RRS_RegistWithPassword = 0x0000,
    RRS_Offline_Request = 0x0001,
    RRS_CheckOnline = 0x0002,
    RRS_Regist = 0x0003,
    RRS_RegistACK = 0x0080,
    RRS_CheckOnlineACK = 0x0082,

    LP_SLIS_Request = 0xA001,
    LP_SLIS_Answer = 0xA002,
    LP_ELRS_ReportStopReq = 0xB001,
    LP_ELRS_ReportAnswer = 0xb002,
    LP_ELRS_Report = 0xB003 ,
    LP_TLRS_ReportReq = 0xc001,
    LP_TLRS_ReportAnswer = 0xc002,
    LP_TLRS_Report = 0xC003,
    LP_TLRS_ReportStopReq = 0xC004,
    LP_TLRS_ReportStopAnswer = 0xC005,
    LP_ConditionTriggeredReportReq = 0xD001,
    LP_ConditionTriggeredReportAnswer = 0xD002,
    LP_ConditionTriggeredLocationReport = 0xD003,
    LP_QuickGPSReportReq = 0xD011,
    LP_QuickGPSReportAnswer = 0xD012,

    TM_Private_No_ACK = 0x80A1,
    TM_Private_Need_ACK = 0x00A1,
    TM_Group = 0x00B1,
    TM_Group_No_Need_ACK = 0x80B1, //only for receive data, don't send this command
    TM_PrivateAck = 0x00A2,
    TM_GroupAck = 0x00B2,
    TM_Private_ShortData_Need_ACK = 0x00AE,//for test only
    TM_Private_ShortData_No_ACK = 0x80AE,//for test only
    TM_Private_ShortData_ACK = 0x00AF,//for test only
    TM_Group_ShortData = 0x00BE,//for test only
    TM_Group_ShortData_No_ACK = 0x80BE,//for test only
    TM_Group_ShortData_ACK = 0x00BF,//for test only

    //0x00B7/0x00B8?©????????±???0x00B3/0x00B4
    TM_Group_QuickTextMessage = 0x00B7,//for test only
    TM_Group_QuickTextMessage_ACK = 0x00B8,//for test only

    TM_Group_PriorTextMessage = 0x00B3,//for test only
    TM_Group_PriorTextMessage_ACK = 0x00B4,//for test only


    TM_Private_WorkOrders = 0x00AC,//for test only
    TM_Private_WorkOrders_ACK = 0x00AD,//for test only
    TM_Group_WorkOrders = 0x00BC,//for test only
    TM_Group_WorkOrders_ACK = 0x00BD,//for test only


    CC_SetCallTarget = 0x0841,
    CC_SetCallTargetACK = 0x8841,
    CC_DelRadioReq = 0x0842,
    CC_DelRadioReqACK = 0x8842,
    CC_ConfigRXTX = 0x10C9,
    CC_ConfigRXTXACK = 0x80C9,
    CC_TX_Status = 0xB843,
    CC_RX_Status = 0xB844,
    CC_BTN_Operation = 0x0041,
    CC_BTN_OperationACK = 0x8041,
    CC_RadioMessageQuery = 0x0201,
    CC_RadioMessageQueryACK = 0x8201,
    CC_DisableRadio = 0x0849,
    CC_DisableRadioACK = 0x8849,
    CC_EnableRadio = 0x084A,
    CC_EnableRadioACK = 0x884A,
    CC_RemoteMonitor = 0x0834,
    CC_RemoteMonitorACK = 0x8834,
    CC_RadioCheck = 0x0833,
    CC_RadioCheckACK = 0x8833,
    CC_AlertCall = 0x0835,
    CC_AlertCallACK = 0x8835,
    CC_ZoneAndChannelAliasRequest = 0x0838,
    CC_ZoneAndChannelAliasRequestACK = 0x8838,
    CC_ChannelStatusCheckRequest = 0x00E7,
    CC_ChannelStatusCheckRequestACK = 0x80E7,
    CC_RadioStatusConfigureRequest = 0x10C7,
    CC_RadioStatusConfigureRequestACK = 0x80C7,
    CC_RadioStatusNotify = 0xB0C8,
    CC_ZoneAndChanneOperationRequest = 0x00C4,
    CC_ZoneAndChanneOperationRequestACK = 0x80C4,
    CC_FunctionStatusCheckRequest = 0x00ED,
    CC_FunctionStatusCheckRequestACK = 0x80ED,
    CC_FunctionEnableDisableRequest = 0x00EE,
    CC_FunctionEnableDisableRequestACK = 0x80EE,
    CC_MICEnableDisableRequest = 0x0043,
    CC_MICEnableDisableRequestACK = 0x8043,
    CC_MICGainCheckControlRequest = 0x0044,
    CC_MICGainCheckControlRequestACK = 0x8044,
    CC_SpeakerEnableDisableRequest = 0x0045,
    CC_SpeakerEnableDisableRequestACK = 0x8045,
    CC_VolumeCheckControlRequest = 0x0046,
    CC_VolumeCheckControlRequestACK = 0x8046,
    CC_BroadcastStatus = 0xB845,
    CC_ChannelAliasRequest = 0x0131,
    CC_ChannelAliasRequestACK = 0x8131,
    CC_GetChannelNumOfZoneRequest = 0x0450,
    CC_GetChannelNumOfZoneACK = 0x8450,
    CC_OverTheAirConfigRequest = 0x00C0,
    CC_OverTheAirConfigACK = 0x80C0,

    CC_ConfigRTPTypeRequest = 0x0419,
    CC_ConfigRTPTypeACK = 0x8419,

    CC_UpdateAuthenticationKeyRequest = 0x0451,
    CC_UpdateAuthenticationKeyACK = 0x8451,

    CC_DigitalVoiceRoutingSwitchRequest = 0x00DF,
    CC_DigitalVoiceRoutingSwitchACK = 0x80DF,

    CC_CallTargetWithConfig = 0x0848,
    CC_CallTargetWithConfigACK = 0x8848,

    CC_TitleMessageRequest = 0x0846,
    CC_TitleMessageACK = 0x8846,

    Analog_BroadcastStatus_Configure = 0x1A00,
    //Function??Configure the radio whether to notify slave when special target status change.

    Analog_BroadcastStatus_Configure_Reply = 0x8A00,//	-> Result(1)
    // Function??Responsion of the command HRCP_Analog_BroadcastStatus_Configure

    Analog_BroadcastVoiceStatus = 0xBA01, //		-> Process(2), Source(2)
    //	?¦????Radio???????????????????¦µ?·?????¶?
    //	??????Process±??????¦µ?·?????¶??¬Source?»??Process??TX Fail?±?????§?¬±???TX Fail?­????

    Analog_HDC1200_BroadcastReceiveInfo = 0xBA02,//		-> CallType(2),TargetID(4),SenderID(4)

    TMP_SRS_Request = 0xA001,
    TMP_SRS_Answer  = 0xA081,
    TMP_RCS_TopologyAnswer = 0xA082 ,
    TMP_RCS_Request = 0xB001,
    TMP_RCS_Answer  = 0xB081,

    DT_Transmit_Request = 0xA001,
    DT_Transmit_Answer = 0xA011,
    DT_Slice_Request = 0xA002,
    DT_Slice_Answer = 0xA012,
    DT_Last_Slice_Request = 0xA003,
    DT_Last_Slice_Answer = 0xA013,

    SM_ChannelChange = 0xFFF1,
    SM_RouteChange = 0xFFF2,

    COMMAND_NONE = 0xFFFF
}CommandType;

typedef enum _CALL_TYPE
{
    n_CALL_TYPE_Private = 0x00,
    n_CALL_TYPE_Group  = 0x01,
    n_CALL_TYPE_All	 = 0x02,
    n_CALL_TYPE_Emergency = 0x03,
    n_CALL_TYPE_Remote_Monitor = 0x04,
    n_CALL_TYPE_Reserve,
    n_CALL_TYPE_PRI_Private,
    n_CALL_TYPE_PRI_Group,
    n_CALL_TYPE_PRI_All,
}CALL_TYPE;

//typedef enum  _RESULT
//{
//    SUCCESS = 0x00,
//    FAILURE = 0x01,
//}RESULT;

typedef enum

{
    BUTTONTYPE_PTT = 0x03,
    BUTTONTYPE_EXTERNAL_PTT = 0x1E,
    BUTTONTYPE_CHANNEL_UP = 0x22,
    BUTTONTYPE_CHANNEL_DOWN = 0x23
}ButtonType;
typedef enum
{
    OPERATYPE_RELEASE = 0x00,//Release
    OPERATYPE_PRESS = 0x01,//       Press
    OPERATYPE_SHORT_PRESS_AND_RELEASE = 0x02, //       Short Press and Release
    OPERATYPE_LONG_PRESS_AND_RELEASE = 0x03,//       Long Press and Release
}OperationType;

typedef enum
{
    RADIOINFO_MODEL_NUMBER = 0,
    RADIOINFO_LAST_PRAGRAMING_DATA,
    RADIOINFO_SERIAL_NUMBER,
    RADIOINFO_FREQUENCY_RAGNE,
    RADIOINFO_RADIO_FIRMWARE_VERSION,
    RADIOINFO_CPS_HANDLE_VERSION,
    RADIOINFO_BOOTLOADER_VERSION,
    RADIOINFO_RADIO_TYPE,
    RADIOINFO_RCDB_VERSION = 9,
}RadioInfoType;
//typedef enum
//	{
//		PROCESS_UNAVAILABLE = 0x00,
//		PROCESS_VOICE_RX,
//		PROCESS_HANG_TIME,
//		PROCESS_CALL_END
//	}CallProcessType;

//	typedef enum
//	{
//		CALLTYPE_PRIVATE = 0x00,//Private Call
//		CALLTYPE_GROUP,			//Group Call
//		CALLTYPE_ALL,			//All Call
//		CALLTYPE_EMERGENCY_GROUP,//Emergency Group Call
//		CALLTYPE_REMOTE_MONITOR,//Remote Monitor Call
//		CALLTYPE_EMERGENCY_ALARM//Emergency Alarm
//	}CallType;

typedef enum
{
    PROCESS_UNAVAILABLE = 0x00,
    PROCESS_VOICE_TX = 0x01,
    PROCESS_HANG_TIME = 0x02,
    PROCESS_CALL_END = 0x03,
    PROCESS_CALL_FAIL = 0x04,
    PROCESS_TOT = 0x05,
    PROCESS_TOT_PRE_ALERT = 0x06,
    // 		PROCESS_RPT_LOC_REPEATING = 0x20,
    // 		PROCESS_RPT_LOC_HANG_TIME = 0x21,
    // 		PROCESS_RPT_IP_REPEATING = 0x22,
    // 		PROCESS_RPT_IP_HANG_TIME = 0x23,
    // 		PROCESS_RPT_CHANNEL_HANG_TIME = 0x24,
    // 		PROCESS_RPT_SLEEP = 0x25,
    // 		PROCESS_RPT_REMOTE_PTT_TX = 0x26,
    // 		PROCESS_RPT_REMOTE_PTT_HANG_TIME = 0x27,
    // 		PROCESS_RPT_REMOTE_PTT_TX_END = 0x28,
    // 		PROCESS_RPT_REMOTE_PTT_WAIT_ACK = 0x29,
    // 		PROCESS_RPT_REMOTE_PTT_TOT = 0x2A,
    // 		PROCESS_RPT_LOCAL_PTT_TX = 0x2B,
    // 		PROCESS_RPT_LOCAL_PTT_HANG_TIME = 0x2C,
    // 		PROCESS_RPT_LOCAL_PTT_TX_END = 0x2D,
    // 		PROCESS_RPT_LOCAL_PTT_WAIT_ACK = 0x2E,
    // 		PROCESS_RPT_LOCAL_PTT_TOT = 0x2F,
}CallProcessType;
typedef enum
{
    CALLTYPE_PRIVATE = 0x00,//Private Call
    CALLTYPE_GROUP,			//Group Call
    CALLTYPE_ALL,			//All Call
    CALLTYPE_EMERGENCY_GROUP,//Emergency Group Call
    CALLTYPE_REMOTE_MONITOR,//Remote Monitor Call
    CALLTYPE_RESERVE,
    CALLTYPE_PRIORITY_PRIVATE,
    CALLTYPE_PRIORITY_GROUP,
    CALLTYPE_PRIORITY_ALL
}CallType;
// 	typedef enum
// 	{
// 		OPERATION_TYPE_VOICE = 0x41,
// 		OPERATION_TYPE_TMP,
// 		OPERATION_TYPE_LP,
// 		OPERATION_TYPE_RRS,
// 		OPERATION_TYPE_TP,
// 		OPERATION_TYPE_SUPPLEMENTARY,
// 	}OperationType;
typedef enum
{
    PCFLAG_SEND_BY_RADIO,
    PCFLAG_SEND_BY_TELEMETRY_APPLICATION
}PCFlag;

//	typedef enum
//	{
//		CALLTYPE_PRIVATE,
//		CALLTYPE_GROUP,
//		CALLTYPE_ALL
//	}CallType;

typedef enum
{
    STATUS_AND_PARAM_OF_CHANNEL = 0x00,
    SQUELCH_LEVAL =	0x01,
    CTCSS_CDCSS_MATCH_STATUS =	0x02,
    POWER_LEVEL = 0x03,
    TX_FREQUENCY =	0x04,
    RX_FREQUENCY =	0x05,
    TX_ALLOW =	0x06,
    CHANNEL_MODE = 0x07,
    TALK_AROUND_STATUS=	0x08,
    RSSI =0x09,
    CARRIER_STATUS =0x0A,
}Target1;

typedef enum
{
    NONE,//$00	None
    SPK_ON_OFF_CHANGE,//$01	SPK on-off change
    SQUELCH_MATCH_STATUS_CHANGE,//$02	Squelch match status change(Only analog channel)
    CTC_CDC_MATCH_STATUS_CHANGE,//$03	CTC/CDC match status change(Only analog channel)
    VOLUME_CHANGE,//$04	Volume change
    ZONE_CHANGE,//$05	Zone change
    CHANNEL_CHANGE,//$06	Channel change
    TALK_AROUND_ON_OFF,//$07	Talk around on-off
    H_L_POWER_LEVEL_CHANGE,//$08	H/L Power level change
    EMERGENCY_ON_OFF,//$09	Emergency on-off??for channel?©
    SCAN_ON_OFF,//$0A	Scan on-off??for channel?©
    RSSI_CHANGE,//$0B	RSSI change
    CARRIER_MATCH_STATUS_CHANGE,//$0C	Carrier match status change
    BATTERY_SAVE_STATUS_CHANGE,//$0D	Battery save status change
    MAN_DOWN_STATUS_CHANGE,//$0E	Man down status change
    //$0F-FF	Reserved
}Target2;
#endif // MESSAGES_H
