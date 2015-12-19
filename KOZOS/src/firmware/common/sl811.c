/*****************************************************************************
	オーダー	: sl811ドライバ
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: sl8ll.c
	接頭辞		: static->sl8 API->sl811
	作成日時	: 2015-12-18
*****************************************************************************/
/*
	改訂履歴
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include "reg3069.h"
#include "usb.h"
#include "lib.h"

/*****************************************************************************
	定義
*****************************************************************************/

/* --- USB-A,B Host Control Register --- */
#define SL8REG_HOST_CTL		(0x00)
#define SL8REG_HOST_CTL_B	(0x08)
//	BIT0 : Arm
//	BIT1 : Enable
//	BIT2 : Direction
//	BIT3 : Reserve
//	BIT4 : ISO
//	BIT5 : SyncSOF
//	BIT6 : Data Toggle Bit
//	BIT7 : Preamble

/* --- USB-A,B Host Base Address --- */
#define SL8REG_BASE_ADDR	(0x01)
#define SL8REG_BASE_ADDR_B	(0x09)
//	BIT0-BIT7 : Base Address

/* --- USB-A,B Host Base Length --- */
#define SL8REG_BASE_LEN		(0x02)
#define SL8REG_BASE_LEN_B	(0x0A)
//	BIT0-BIT7 : Maximam packe size 

/* --- USB-A,B Packet Status (When Read) --- */
#define SL8REG_PKT_STAT		(0x03)
#define SL8REG_PKT_STAT_B	(0x0B)
//	(When Read)
//	BIT0 : ACK
//	BIT1 : Error
//	BIT2 : Time-out
//	BIT3 : Sequence
//	BIT4 : Setup
//	BIT5 : Overflow
//	BIT6 : NAK
//	BIT7 : STALL

/* --- USB-A,B Host PID, Device Endpoint (When Write) --- */
#define SL8REG_PID_EP		(0x03)
#define SL8REG_PID_EP_B		(0x0B)
//	(When Write)
//	BIT0 : End Point 0
//	BIT1 : End Point 1
//	BIT2 : End Point 2
//	BIT3 : End Point 3
//	BIT4 : Packet IDentifier 0
//	BIT5 : Packet IDentifier 1
//	BIT6 : Packet IDentifier 2
//	BIT7 : Packet IDentifier 3

/* --- USB-A,BTransfer Count(When Read) --- */
#define SL8REG_TRAN_CNT		(0x04)
#define SL8REG_TRAN_CNT_B	(0x0C)
//	(When Read)
//	BIT0-BIT7 : Transfer Count

/* --- USB-A Host Device Address (Write) --- */
#define SL8REG_DEV_ADDR		(0x04)
#define SL8REG_DEV_ADDR_B	(0x0C)
//	(When Write)
//	BIT0-BIT6	: USB Address
//	BIT7		: Reserved. must be set to zero.

/* --- Control Register1 --- */
#define SL8REG_CTL1			(0x05)
//	BIT0 : SOF enable/disable
//	BIT1 : Reserved
//	BIT2 : Reserved
//	BIT3 : USB Engine Reset
//	BIT4 : J-K state force
//	BIT5 : USB Speed
//	BIT6 : Suspend
//	BIT7 : Reserved

/* --- Interrupt Enable Register --- */
#define SL8REG_INTR_ENB		(0x06)
//	BIT0 : USB-A Done
//	BIT1 : USB-B Done
//	BIT2 : Reserved
//	BIT3 : Reserved
//	BIT4 : SOF Timer
//	BIT5 : Inserted/Removed
//	BIT6 : Device Detect/Resume
//	BIT7 : Reserved

/* #define SL8REG_0x07		(0x07)	Reserved */

/* --- Interrupt Status Register. write clears bitwise --- */
#define SL8REG_INTR_STAT	(0x0D)
//	BIT0 : USB-A
//	BIT1 : USB-B
//	BIT2 : Reserved
//	BIT3 : Reserved
//	BIT4 : SOF Timer
//	BIT5 : Inserted/Removed
//	BIT6 : Device Detect/Resume
//	BIT7 : D+

/* --- HW Revision Register (When Read) --- */
#define SL8REG_HW_REV		(0x0E)
//	(When Read)
//	BIT0-BIT3 : Reserved
//	BIT4-BIT7 : Hardware Revision

/* "Start Of Frame" LOW Address(When Write) */
#define SL8REG_SOF_LOW		(0x0E)
//	(When Write)
//	BIT0-BIT7 : SOF Low Address

/* --- SOF Counter HIGH (When Read) --- */
#define SL8REG_SOF_TMR		(0x0F)
//	(When Read)
//	BIT0-BIT7 : SOF High Counter

/* --- Control Register2 (When Write) --- */
#define SL8REG_CTL2			(0x0F)
//	(When Write)
//	BIT0-BIT5 : SOF High Counter
//	BIT6 : D+/D- Data Polarity Swap(Low/High Speed)
//	BIT7 : Master/Slave selection

#define SL8REG_DATA_START	(0x10)
#define SL8REG_DATA_END		(0xFF)

#define REG_TEST_BUS_SIZE	(256)

/* Control register 1 bits (addr 5) */
#define SL8_CTL1MASK_DSBLSOF  1
#define SL8_CTL1MASK_NOTXEOF2 4
#define SL8_CTL1MASK_DSTATE   0x18
#define SL8_CTL1MASK_NSPD     0x20
#define SL8_CTL1MASK_SUSPEND  0x40
#define SL8_CTL1MASK_CLK12    0x80

#define SL8_CTL1VAL_RESET     8


/* HW rev and SOF lo register bits (addr 0xE) */

#define SL8_HWRMASK_HWREV     0xF0

/* SOF counter and control reg 2 (addr 0xF) */

#define SL8_CTL2MASK_SOFHI    0x3F
#define SL8_CTL2MASK_DSWAP    0x40
#define SL8_CTL2MASK_HOSTMODE 0xae

#define DATA0_WR    0x07   // (Arm+Enable+tranmist to Host+DATA0)
#define DATA1_WR    0x47   // (Arm+Enable+tranmist to Host on DATA1)
#define ZDATA0_WR   0x05   // (Arm+Transaction Ignored+tranmist to Host+DATA0)
#define ZDATA1_WR   0x45   // (Arm+Transaction Ignored+tranmist to Host+DATA1)
#define DATA0_RD    0x03   // (Arm+Enable+received from Host+DATA0)
#define DATA1_RD    0x43   // (Arm+Enable+received from Host+DATA1)

#define PID_SETUP   0xd0
#define PID_SOF     0x50
#define PID_IN      0x90
#define PID_OUT     0x10


/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
typedef struct{
	volatile char	*pcAddrReg;
	volatile char	*pcDataReg;
	char			cSpeed;
	char			cAddr;
	char			cDummy[6];
} SL811Info;

/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/
static SL811Info s_sl8;

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/
static char sl8Read(char cOffset);
static void sl8Write(char cOffset, char cBuf);
static void sl8BufRead(char cOffset, char* buf, char cSize);
static void sl8BufWrite(char cOffset, char* buf, char cSize);
static int sl8RegTest(void);
static int sl8Reset(void);
static int write_setup(char* pcBuf);

/*****************************************************************************
	テーブル
*****************************************************************************/

/*****************************************************************************
	DISCRIPTION	: sl811の1バイト読み込み
	ARGUMENT	: cOffset = sl811レジスタオフセット
	RETURN		: 読み取りデータ1バイト
	NOTE		: -
	UPDATED		: 2015-12-18
*****************************************************************************/
static char sl8Read(char cOffset)
{
	*(s_sl8.pcAddrReg) = cOffset;
	return *(s_sl8.pcDataReg);
}

/*****************************************************************************
	DISCRIPTION	: sl811の1バイト書き込み
	ARGUMENT	: cOffset = sl811レジスタオフセット
				: cBuf = 書き込むデータ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-12-18
*****************************************************************************/
static void sl8Write(char cOffset, char cBuf)
{
	*(s_sl8.pcAddrReg) = cOffset;
	*(s_sl8.pcDataReg) = cBuf;

	return;
}

/*****************************************************************************
	DISCRIPTION	: sl811のバッファ読み込み
	ARGUMENT	: cOffset = sl811レジスタオフセット
				: pcBuf = 読み出しデータ
				: cSize = 読み出しサイズ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-12-18
*****************************************************************************/
static void sl8BufRead(char cOffset, char* pcBuf, char cSize)
{
	int i;

	*(s_sl8.pcAddrReg) = cOffset;

	for(i = 0; i < cSize; i++, pcBuf++){
		*pcBuf = *(s_sl8.pcDataReg);
	}

	return;
}

/*****************************************************************************
	DISCRIPTION	: sl811のバッファ書き込み
	ARGUMENT	: cOffset = sl811レジスタオフセット
				: pcBuf = 書き込みデータ
				: cSize = 書き込みサイズ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-12-18
*****************************************************************************/
static void sl8BufWrite(char cOffset, char* pcBuf, char cSize)
{
	int i;

	*(s_sl8.pcAddrReg) = cOffset;
	for(i = 0; i < cSize; i++, pcBuf++){
		*(s_sl8.pcDataReg) = *pcBuf;
	}

	return;
}

/*****************************************************************************
	DISCRIPTION	: sl811レジスタテスト
	ARGUMENT	: -
	RETURN		: (-1) = 異常終了(レジスタなし)
				: 0 = 正常終了
	NOTE		: -
	UPDATED		: 2015-12-18
*****************************************************************************/
static int sl8RegTest(void)
{
	int		i, iRet = 0;
	char	cData;
	char	cBuf[REG_TEST_BUS_SIZE];

	for(i = SL8REG_DATA_START; i < REG_TEST_BUS_SIZE; i++){
		cBuf[i] = sl8Read(i);
		sl8Write(i, i);
	}

	for(i = SL8REG_DATA_START; i < REG_TEST_BUS_SIZE; i++){
		cData = sl8Read(i);
		if(cData != i){
			iRet = -1;
		}
	}

	for(i = SL8REG_DATA_START; i < REG_TEST_BUS_SIZE; i++){
		sl8Write(i, cBuf[i]);
	}
	return iRet;
}

/*****************************************************************************
	DISCRIPTION	: sl811リセット
	ARGUMENT	: -
	RETURN		: -1 = 異常終了
				: 0 = 正常終了
	NOTE		: -
	UPDATED		: 2015-12-18
*****************************************************************************/
#if 0	/* Constant number rewrite */
static int sl8Reset(void)
{
	int	iStat;
	
	sl8Write(SL8REG_CTL2, 0xae);
	sl8Write(SL8REG_CTL1, 0x08);	// reset USB
	waitms(20);				// 20ms
	sl8Write(SL8REG_CTL1, 0);	// remove SE0

	for(iStat = 0; iStat < 100; iStat++){
		sl8Write(SL8REG_INTR_STAT, 0xff); // clear all interrupt bits
	}
	iStat = sl8Read(SL8REG_INTR_STAT);
	if(iStat & 0x40){  // Check if device is removed
		s_sl8.cSpeed = 0;	// None
		sl8Write(SL8REG_INTR_ENB, BIT0 | BIT4 | BIT5);
		return -1;
	}

	sl8Write(SL8REG_BASE_LEN_B, 0);	//zero lenth
	sl8Write(SL8REG_PID_EP_B, 0x50);	//send SOF to EP0
	sl8Write(SL8REG_DEV_ADDR_B, 0x01);	//address0
	sl8Write(SL8REG_SOF_LOW, 0xe0);

	if(!(iStat & 0x80)){
		s_sl8.cSpeed = USB_LOW;	// Low
		sl8Write(SL8REG_CTL1, 0x8);
		waitms(20);
		sl8Write(SL8REG_SOF_TMR, 0xee);
		sl8Write(SL8REG_CTL1, 0x21);
		sl8Write(SL8REG_HOST_CTL_B, 0x01);
		for(iStat = 0; iStat < 20; iStat++){
			sl8Write(SL8REG_INTR_STAT, 0xff);
		}
	}
	else{
		s_sl8.cSpeed = USB_FULL;	// Full
		sl8Write(SL8REG_CTL1, 0x8);
		waitms(20);
		sl8Write(SL8REG_SOF_TMR, 0xae);
		sl8Write(SL8REG_CTL1, 0x01 );
		sl8Write(SL8REG_HOST_CTL_B, 0x01);
		sl8Write(SL8REG_INTR_STAT, 0xff);
	}

	sl8Write(SL8REG_INTR_ENB, BIT0 | BIT4 | BIT5);
	return 0;
}
#else	/* Constant number rewrite */
static int sl8Reset(void)
{
	int	iStat = 0;
	int	iRet = 0;
//	int	i;
	
	sl8Write(SL8REG_CTL2, BIT1 | BIT2 | BIT3 | BIT5 | BIT7);
	sl8Write(SL8REG_CTL1, BIT3);	// reset USB
//	waitms(20);				// 20ms
	waitms(50);				// 50ms
	sl8Write(SL8REG_CTL1, 0);	// remove SE0

//	for(i = 0; i < 100; i++){
		sl8Write(SL8REG_INTR_STAT, 0xFF); // clear all interrupt bits
//	}

	iStat = sl8Read(SL8REG_INTR_STAT);
	if(iStat & BIT6){  // Check if device is removed
		s_sl8.cSpeed = 0;	// None
		sl8Write(SL8REG_INTR_ENB, BIT0 | BIT4 | BIT5);
		iRet = -1;
	}
	else{

		sl8Write(SL8REG_BASE_LEN_B, 0);	//zero lenth
		sl8Write(SL8REG_PID_EP_B, BIT4 | BIT6);	//send SOF to EP0
		sl8Write(SL8REG_DEV_ADDR_B, 0x01);	//address0
		sl8Write(SL8REG_SOF_LOW, 0xE0);

		if(!(iStat & BIT7)){
			s_sl8.cSpeed = USB_LOW;	// Low
			sl8Write(SL8REG_CTL1, BIT3);
//			waitms(20);
			waitms(50);
			sl8Write(SL8REG_SOF_TMR, 0xEE);
			sl8Write(SL8REG_CTL1, BIT0 | BIT5);
			sl8Write(SL8REG_HOST_CTL_B, 0x01);

//			for(i = 0; i < 100; i++){
				sl8Write(SL8REG_INTR_STAT, 0xFF);
//			}
		}
		else{
			s_sl8.cSpeed = USB_FULL;	// Full
			sl8Write(SL8REG_CTL1, BIT3);
//			waitms(20);
			waitms(50);
			sl8Write(SL8REG_SOF_TMR, 0xAE);
			sl8Write(SL8REG_CTL1, BIT0);
			sl8Write(SL8REG_HOST_CTL_B, 0x01);

//			for(i = 0; i < 100; i++){
				sl8Write(SL8REG_INTR_STAT, 0xFF);
//			}
		}

		sl8Write(SL8REG_INTR_ENB, BIT0 | BIT4 | BIT5);
	}

	return iRet;
}

#endif	/* Constant number rewrite */

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-12-18
*****************************************************************************/
static int write_setup(char* pcBuf)
{
	int		iRet;
	char	cPktStat;

	sl8BufWrite(SL8REG_DATA_START, pcBuf, 8);
	sl8Write(SL8REG_BASE_ADDR, SL8REG_DATA_START);
	sl8Write(SL8REG_BASE_LEN, 8);
	sl8Write(SL8REG_DEV_ADDR, s_sl8.cAddr);
	sl8Write(SL8REG_PID_EP, PID_SETUP);
	sl8Write(SL8REG_HOST_CTL, DATA0_WR);
	waitus(250);
	while((sl8Read(SL8REG_INTR_STAT) & BIT0) == 0){
		//do nothing
	}

	cPktStat = sl8Read(SL8REG_PKT_STAT);

	iRet = 0;

	if(cPktStat & BIT0){
		iRet |= USB_ACK;
	}
	if(cPktStat & BIT6){
		iRet |= USB_NAK;
	}
	if(cPktStat & BIT7){
		iRet |= USB_STALL;
	}
	return iRet;
}

/*****************************************************************************
	DISCRIPTION	: sl811初期化
	ARGUMENT	: -
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-12-18
*****************************************************************************/
int sl811Init(void)
{
	P1DDR = 0xFF;	/* A0-7    is enable */
	P2DDR = 0xFF;	/* A8-15   is enable */
	P5DDR = 0x01;	/* A16     is enable */
	P8DDR = 0x0E;	/* CS1-3 is enable */

	s_sl8.pcAddrReg = (volatile char *)0x200001;
	s_sl8.pcDataReg = (volatile char *)0x200003;
	s_sl8.cAddr = 0;

	return sl8RegTest();
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: sl8Sendにする。
	UPDATED		: 2015-12-18
*****************************************************************************/
int write_sl811(int num, char* pcBuf, int iSize)
{
	int		iRet = 0;
	char	cPktStat;

	if(iSize < USB_HDRSIZ){
		return 0;
	}
	sl8BufWrite(SL8REG_DATA_START, &pcBuf[USB_HDRSIZ], iSize - USB_HDRSIZ);
	sl8Write(SL8REG_BASE_ADDR, SL8REG_DATA_START);
	sl8Write(SL8REG_BASE_LEN, iSize - USB_HDRSIZ);
	sl8Write(SL8REG_DEV_ADDR, s_sl8.cAddr);
	sl8Write(SL8REG_PID_EP, PID_OUT | pcBuf[USB_EP]);
	sl8Write(SL8REG_HOST_CTL, (pcBuf[USB_TOGGLE] == 0) ? DATA0_WR : DATA1_WR);
	waitus(250);
	while((sl8Read(SL8REG_INTR_STAT) & BIT0) == 0){
		//do nothing
	}

	cPktStat = sl8Read(SL8REG_PKT_STAT);
	if(cPktStat & BIT0){
		iRet |= USB_ACK;
	}
	if(cPktStat & BIT6){
		iRet |= USB_NAK;
	}
	if(cPktStat & BIT7){
		iRet |= USB_STALL;
	}
	return iRet;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: sl8Recvにする。
	UPDATED		: 2015-12-18
*****************************************************************************/
int read_sl811(int num, char* pcBuf, int iSize)
{
	int		iRet;
	char	cPktStat;
	int		timovr;

	if(iSize < 0){
		return 0;
	}
	sl8Write(SL8REG_BASE_ADDR, 0x10);
	sl8Write(SL8REG_BASE_LEN, iSize);
	sl8Write(SL8REG_DEV_ADDR, s_sl8.cAddr);
	sl8Write(SL8REG_PID_EP, PID_IN | pcBuf[USB_EP]);

	for(timovr = 0 ;timovr < 200; timovr++){
		sl8Write(SL8REG_HOST_CTL, (pcBuf[USB_TOGGLE] == 0) ? DATA0_RD : DATA1_RD);
		waitus(250);

		while((sl8Read(SL8REG_INTR_STAT) & BIT0) == 0){
			//do nothing
		}

		cPktStat= sl8Read(SL8REG_PKT_STAT);
		if((cPktStat & BIT6) == 0){
			break;
		}
	}

	if(cPktStat & BIT0){
		sl8BufRead(SL8REG_DATA_START, pcBuf, iSize);
	}
	iRet = 0;
	if(cPktStat & BIT0){
		iRet |= USB_ACK;
	}
	if(cPktStat & BIT6){
		iRet |= USB_NAK;
	}
	if(cPktStat & BIT7){
		iRet |= USB_STALL;
	}
	return iRet;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: sl8Seekにする
	UPDATED		: 2015-12-18
*****************************************************************************/
int seek_sl811(int num, int iPos)
{
	if(num >= SL_NUM){
		return -1;
	}
	s_sl8.cAddr = iPos;
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-12-18
*****************************************************************************/
int ioctl_sl811(int num, long lData, int op)
{
	static char	getdesc[8] ={0x80, 0x06, 0, 1, 0, 0, 64, 0};
	static char	setaddr[8] ={0x00, 0x05, 2, 0, 0, 0, 0, 0};
	char		pcBuf[10];
	int			iRet;

	if(num >= SL_NUM){
		return -1;
	}

	iRet = 0;
	switch(op){
	case USB_RESET:
		iRet = -1;
		s_sl8.cAddr = 0;
		if(sl8Reset() != 0){
			break;
		}

		if((write_setup(getdesc) & USB_ACK) == 0){
			break;
		}

		pcBuf[USB_EP] = 0;
		pcBuf[USB_TOGGLE] = 1;
		if((read_sl811(num, pcBuf, 8) & USB_ACK) == 0){
			break;
		}

		pcBuf[USB_EP] = 0;
		pcBuf[USB_TOGGLE] = 1;
		if((write_sl811(num, pcBuf, 2) & USB_ACK) == 0){
			break;
		}

		if((write_setup(setaddr) & USB_ACK) == 0){
			break;
		}
		pcBuf[USB_EP] = 0;
		pcBuf[USB_TOGGLE] = 1;
		if((read_sl811(num, pcBuf, 0) & USB_ACK) == 0){
			break;
		}

		s_sl8.cAddr = lData;
		iRet = (int)pcBuf[7];
		break;

	case USB_GETSPEED:
		iRet = (int)s_sl8.cSpeed;
		break;

	case USB_SETUP:
		iRet = write_setup((char*)lData);
		break;

	default:
		iRet = -1;
	}

	return iRet;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-12-18
*****************************************************************************/
int sl811Reset(long lData)
{
	static char	getdesc[8] ={0x80, 0x06, 0, 1, 0, 0, 64, 0};
	static char	setaddr[8] ={0x00, 0x05, 2, 0, 0, 0, 0, 0};
	char		pcBuf[10];
	int			iRet;
	int			iEpSize;
	int			num = 0;

	s_sl8.cAddr = 0;
	iRet = sl8Reset();
	if(iRet != 0){
		return -1;
	}

	iRet = write_setup(getdesc);
	if((iRet & USB_ACK) == 0){
		return -1;
	}

	pcBuf[USB_EP] = 0;
	pcBuf[USB_TOGGLE] = 1;
	iRet = read_sl811(num, pcBuf, 8);
	if((iRet & USB_ACK) == 0){
		return -1;
	}

	pcBuf[USB_EP] = 0;
	pcBuf[USB_TOGGLE] = 1;
	iRet = write_sl811(num, pcBuf, 2);
	if((iRet & USB_ACK) == 0){
		return -1;
	}

	iRet =write_setup(setaddr);
	if((iRet & USB_ACK) == 0){
		return -1;
	}
	pcBuf[USB_EP] = 0;
	pcBuf[USB_TOGGLE] = 1;
	iRet = read_sl811(num, pcBuf, 0);
	if((iRet & USB_ACK) == 0){
		return -1;
	}

	s_sl8.cAddr = lData;
	iEpSize = (int)pcBuf[7];

	return iEpSize;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-12-18
*****************************************************************************/
int sl811Setup(char* pcBuf)
{
	return write_setup(pcBuf); 
}

/***** End Of File *****/

