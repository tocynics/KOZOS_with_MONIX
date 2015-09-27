/*****************************************************************************
	オーダー	: USBライブラリ ヘッダ
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 4.7.0
	ファイル名	: usb.h
	接頭辞		: usb 
	作成日時	: 2015-05-07
*****************************************************************************/
/*
	改訂履歴
*/

#ifndef _KZ_USB_H_
#define _KZ_USB_H_

/*****************************************************************************
	インクルード
*****************************************************************************/
#include	"defines.h"

/*****************************************************************************
	定義
*****************************************************************************/
#define	bswap16(x)	((((x) & 0xff00) >>  8) | (((x) & 0x00ff) <<  8))
#define	htole16(x)	bswap16((x))
#define	le16toh(x)	bswap16((x))

#define TRANS_TO_DEV	(0x00)
#define TRANS_TO_HOST	(0x80)
#define TRANS_TO_IF		(0x01)
#define TRANS_TO_EP		(0x02)

#define GET_STATUS      (0x00)
#define CLEAR_FEATURE   (0x01)
#define SET_FEATURE     (0x03)
#define SET_ADDRESS     (0x05)
#define GET_DESCRIPTOR  (0x06)
#define SET_DESCRIPTOR  (0x07)
#define GET_CONFIG      (0x08)
#define SET_CONFIG      (0x09)
#define GET_INTERFACE   (0x0a)
#define SET_INTERFACE   (0x0b)
#define SYNCH_FRAME     (0x0c)

#define DEVICE_TYPE		(0x01)
#define CONFIG_TYPE		(0x02)
#define STRING_TYPE		(0x03)
#define INTERFACE_TYPE	(0x04)

#define ENDPOINT_TYPE	(0x05)
#define STDCLASS        (0x00)
#define HIDCLASS        (0x03)
#define HUBCLASS		(0x09)	// bDeviceClass, bInterfaceClass

#define	MAX_EP			(8)		/* 最大エンドポイント数 */

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
typedef struct {
	uint16	wVID, wPID;	// Vendor ID and Product ID
	uint08	bClass;	
	uint08	bNumOfEPs;	// actual number endpoint from slave

	uint08	iMfg;		// Manufacturer ID
	uint08	iPdt;		// Product ID
	uint08	bId1;
	uint08	bId2;

	uint08	bEPAddr[MAX_EP];	// bit 7 = 1 = use PID_IN, 
	uint08	bAttr[MAX_EP];		// ISO | Bulk | Interrupt | Control
	uint16	wPayLoad[MAX_EP];	// ISO range: 1-1023, Bulk: 1-64, etc

	uint16	bInterval[MAX_EP];	// polling interval (for LS)
	uint08	bData1[MAX_EP];		// DataToggle value
	uint08	ucDummy[1];			// DataToggle value
} USBDEV;

//typedef struct {
//	uint08	bmRequest	;
//	uint08	bRequest	;
//	uint16	wValue		;
//	uint16	wIndex		;
//	uint16	wLength		;
//} SetupPKG;

typedef struct {
	uint08	bmRequest;
	uint08	bRequest;
	uint16	wValue;
	uint16	wIndex;
	uint16	wLength;
} SetupPKG;

typedef struct {
	uint08	bLength				;
	uint08	bDescriptorType		;
	uint16	bcdUSB				;
	
	uint08	bDeviceClass		;
	uint08	bDeviceSubClass		;
	uint08	bDeviceProtocol		;
	uint08	bMaxPacketSize0		;
	
	uint16	idVendor			;
	uint16	idProduct			;
	
	uint16	bcdDevice			;
	uint08	iManufacturer		;
	uint08	iProduct			;
	uint08	iSerialNumber		;
	uint08	bNumConfigurations	;
	uint08	ucDummy[2];
} DevDesc;

// Standard Configuration Descriptor
typedef struct {
	uint08	bLength		;	// Size of descriptor in unsigned char
	uint08	bType		;	// Configuration
	uint16	wLength		;	// Total length
	uint08	bNumIntf	;	// Number of interface
	uint08	bCV			;	// bConfigurationValue
	uint08	bIndex		;	// iConfiguration
	uint08	bAttr		;	// Configuration Characteristic
	uint08	bMaxPower	;	// Power config
} CfgDesc;

// Standard Interface Descriptor
typedef struct {
	uint08	bLength		;
	uint08	bType		;
	uint08	iNum		;
	uint08	iAltString	;

	uint08	bEndPoints	;
	uint08	iClass		;
	uint08	iSub		; 
	uint08	iProto		;

	uint08	iIndex		; 
	uint08	ucDummy[3];
} IntfDesc;

// Standard EndPoint Descriptor
typedef struct{
	uint08	bLength		;
	uint08	bType		;
	uint08	bEPAdd		;
	uint08	bAttr		;

	uint16	wPayLoad	;	// low-speed this must be 0x08
	uint08	bInterval	;
	uint08	ucDummy[1];
} EPDesc;

// Standard String Descriptor
typedef struct {
	uint08	bLength;
	uint08	bType;
	uint16	wLang;
} StrDesc;

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/

/*****************************************************************************
	外部変数 
*****************************************************************************/

#endif /* _KZ_USB_H_ */

/***** End Of File *****/

