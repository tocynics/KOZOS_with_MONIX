/*****************************************************************************
	オーダー	: sl811ドライバ
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: sl8.c
	接頭辞		: static->sl8 API->sl811
	作成日時	: 2014-06-26
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

#define SL11H_HOSTCTLREG        0
#define SL11H_BUFADDRREG        1
#define SL11H_BUFLNTHREG        2
#define SL11H_PKTSTATREG        3	/* read */
#define SL11H_PIDEPREG          3	/* write */
#define SL11H_XFERCNTREG        4	/* read */
#define SL11H_DEVADDRREG        4	/* write */
#define SL11H_CTLREG1           5
#define SL11H_INTENBLREG        6

#define SL11H_HOSTCTLREG_B      8
#define SL11H_BUFADDRREG_B      9
#define SL11H_BUFLNTHREG_B      10
#define SL11H_PKTSTATREG_B      11	/* read */
#define SL11H_PIDEPREG_B        11	/* write */
#define SL11H_XFERCNTREG_B      12	/* read */
#define SL11H_DEVADDRREG_B      12	/* write */

#define SL11H_INTSTATREG        13	/* write clears bitwise */
#define SL11H_HWREVREG          14	/* read */
#define SL11H_SOFLOWREG         14	/* write */
#define SL11H_SOFTMRREG         15	/* read */
#define SL11H_CTLREG2           15	/* write */
#define SL11H_DATA_START        16

/* Host control register bits (addr 0) */

#define SL11H_HCTLMASK_ARM      1
#define SL11H_HCTLMASK_ENBLEP   2
#define SL11H_HCTLMASK_WRITE    4
#define SL11H_HCTLMASK_ISOCH    0x10
#define SL11H_HCTLMASK_AFTERSOF 0x20
#define SL11H_HCTLMASK_SEQ      0x40
#define SL11H_HCTLMASK_PREAMBLE 0x80

/* Packet status register bits (addr 3) */

#define SL11H_STATMASK_ACK      1
#define SL11H_STATMASK_ERROR    2
#define SL11H_STATMASK_TMOUT    4
#define SL11H_STATMASK_SEQ      8
#define SL11H_STATMASK_SETUP    0x10
#define SL11H_STATMASK_OVF      0x20
#define SL11H_STATMASK_NAK      0x40
#define SL11H_STATMASK_STALL    0x80

/* Control register 1 bits (addr 5) */
#define SL11H_CTL1MASK_DSBLSOF  1
#define SL11H_CTL1MASK_NOTXEOF2 4
#define SL11H_CTL1MASK_DSTATE   0x18
#define SL11H_CTL1MASK_NSPD     0x20
#define SL11H_CTL1MASK_SUSPEND  0x40
#define SL11H_CTL1MASK_CLK12    0x80

#define SL11H_CTL1VAL_RESET     8

/* Interrut enable (addr 6) and interrupt status register bits (addr 0xD) */

#define SL11H_INTMASK_XFERDONE  1
#define SL11H_INTMASK_SOFINTR   0x10
#define SL11H_INTMASK_INSRMV    0x20
#define SL11H_INTMASK_USBRESET  0x40
#define SL11H_INTMASK_DSTATE    0x80	/* only in status reg */

/* HW rev and SOF lo register bits (addr 0xE) */

#define SL11H_HWRMASK_HWREV     0xF0

/* SOF counter and control reg 2 (addr 0xF) */

#define SL11H_CTL2MASK_SOFHI    0x3F
#define SL11H_CTL2MASK_DSWAP    0x40
#define SL11H_CTL2MASK_HOSTMODE 0xae

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
typedef struct {
	volatile char	*addr_reg;
	volatile char	*data_reg;
	char			speed;
	char			addr;
	char			cDummy[6];
} SL811Info;

/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/
static SL811Info sl811[SL_NUM];

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/
static char sl8Read(int num, char offset);
static void sl8Write(int num, char offset, char data);
static void sl8BufRead(int num, short offset, char* buf, short size);
static void sl8BufWrite(int num, short offset, char* buf, short size);
static int regTest(int num);
static int USBReset(int num);
static int write_setup(int num, char* data);

/*****************************************************************************
	テーブル
*****************************************************************************/

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-26
*****************************************************************************/
static char sl8Read(int num, char offset) {
	char data;

	*(sl811[num].addr_reg) = offset;
	data = *(sl811[num].data_reg);
	return data;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-26
*****************************************************************************/
static void sl8Write(int num, char offset, char data){
	*(sl811[num].addr_reg) = offset;
	*(sl811[num].data_reg) = data;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-26
*****************************************************************************/
static void sl8BufRead(int num, short offset, char* buf, short size){
	if(size <= 0) return;
	*(sl811[num].addr_reg) = (char)offset;
	while(size--) *buf++ = *(sl811[num].data_reg);
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-26
*****************************************************************************/
static void sl8BufWrite(int num, short offset, char* buf, short size) {
	if(size <= 0) return;
	*(sl811[num].addr_reg) = (char)offset;
	while(size--) {
		*(sl811[num].data_reg) = *buf;
		buf++;
	}
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-26
*****************************************************************************/
static int regTest(int num){
	int	i, data, result = 0;
	char	buf[256];

	for(i = 16;i < 256; i++) {
		buf[i] = (char)sl8Read(num, i);
		sl8Write(num, i, i);
	}
	for(i = 16;i < 256; i++) {
		data = sl8Read(num, i);
		if(data != i) {
			result = -1;
		}
	}
	for(i = 16;i < 256;i++) sl8Write(num, i, buf[i]);
	return result;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-26
*****************************************************************************/
static int USBReset(int num) {
	int	status;

	sl8Write(num, SL11H_CTLREG2, 0xae);
	sl8Write(num, SL11H_CTLREG1, 0x08);	// reset USB
	waitms(20);				// 20ms
	sl8Write(num, SL11H_CTLREG1, 0);	// remove SE0

	for(status = 0;status < 100;status++) {
		sl8Write(num, SL11H_INTSTATREG, 0xff); // clear all interrupt bits
	}
	status = sl8Read(num, SL11H_INTSTATREG);
	if(status & 0x40){  // Check if device is removed
		sl811[num].speed = 0;	// None
		sl8Write(num, SL11H_INTENBLREG, SL11H_INTMASK_XFERDONE | 
			   SL11H_INTMASK_SOFINTR | SL11H_INTMASK_INSRMV);
		return -1;
	}
	sl8Write(num, SL11H_BUFLNTHREG_B, 0);	//zero lenth
	sl8Write(num, SL11H_PIDEPREG_B, 0x50);	//send SOF to EP0
	sl8Write(num, SL11H_DEVADDRREG_B, 0x01);	//address0
	sl8Write(num, SL11H_SOFLOWREG, 0xe0);
	if(!(status & 0x80)) {
		sl811[num].speed = USB_LOW;	// Low
		sl8Write(num, SL11H_CTLREG1, 0x8);
		waitms(20);
		sl8Write(num, SL11H_SOFTMRREG, 0xee);
		sl8Write(num, SL11H_CTLREG1, 0x21);
		sl8Write(num, SL11H_HOSTCTLREG_B, 0x01);
		for(status = 0;status < 20;status++) {
			sl8Write(num, SL11H_INTSTATREG, 0xff);
		}
	} else {
		sl811[num].speed = USB_FULL;	// Full
		sl8Write(num, SL11H_CTLREG1, 0x8);
		waitms(20);
		sl8Write(num, SL11H_SOFTMRREG, 0xae);
		sl8Write(num, SL11H_CTLREG1, 0x01 );
		sl8Write(num, SL11H_HOSTCTLREG_B, 0x01);
		sl8Write(num, SL11H_INTSTATREG, 0xff);
	}
	sl8Write(num, SL11H_INTENBLREG, SL11H_INTMASK_XFERDONE | 
		   SL11H_INTMASK_SOFINTR|SL11H_INTMASK_INSRMV);
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-26
*****************************************************************************/
static int write_setup(int num, char* data) {
	char	pktstat, ret;

	sl8BufWrite(num, 0x10, data, 8);
	sl8Write(num, SL11H_BUFADDRREG, 0x10);
	sl8Write(num, SL11H_BUFLNTHREG, 8);
	sl8Write(num, SL11H_DEVADDRREG, sl811[num].addr);
	sl8Write(num, SL11H_PIDEPREG, PID_SETUP);
	sl8Write(num, SL11H_HOSTCTLREG, DATA0_WR);
	waitus(200);
	while((sl8Read(num, SL11H_INTSTATREG) & SL11H_INTMASK_XFERDONE) == 0);
	pktstat = sl8Read(num, SL11H_PKTSTATREG);
	ret = 0;
	if(pktstat & SL11H_STATMASK_ACK) ret |= USB_ACK;
	if(pktstat & SL11H_STATMASK_NAK) ret |= USB_NAK;
	if(pktstat & SL11H_STATMASK_STALL) ret |= USB_STALL;
	return ret;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-26
*****************************************************************************/
int sl811Init(void) {
	int num;

	P1DDR = 0xff;	/* A0-7    is enable */
	P2DDR = 0xff;	/* A8-15   is enable */
	P5DDR = 0x01;	/* A16     is enable */
	P8DDR = 0x0e;	/* CS1-3 is enable */

	num = 0;
	sl811[num].addr_reg = (volatile char *)0x200001;
	sl811[num].data_reg = (volatile char *)0x200003;
	sl811[num].addr = 0;

	return regTest(num);
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: sl8Sendにする。
	UPDATED		: 2014-06-26
*****************************************************************************/
int write_sl811(int num, char* data, int size) {
	char	pktstat, ret;

	if(size < USB_HDRSIZ) return 0;
	sl8BufWrite(num, 0x10, &data[USB_HDRSIZ], size - USB_HDRSIZ);
	sl8Write(num, SL11H_BUFADDRREG, 0x10);
	sl8Write(num, SL11H_BUFLNTHREG, size - USB_HDRSIZ);
	sl8Write(num, SL11H_DEVADDRREG, sl811[num].addr);
	sl8Write(num, SL11H_PIDEPREG, PID_OUT | data[USB_EP]);
	sl8Write(num, SL11H_HOSTCTLREG, (data[USB_TOGGLE] == 0) ? DATA0_WR : DATA1_WR);
	waitus(200);
	while((sl8Read(num, SL11H_INTSTATREG) & SL11H_INTMASK_XFERDONE) == 0);
	pktstat = sl8Read(num, SL11H_PKTSTATREG);
	ret = 0;
	if(pktstat & SL11H_STATMASK_ACK) ret |= USB_ACK;
	if(pktstat & SL11H_STATMASK_NAK) ret |= USB_NAK;
	if(pktstat & SL11H_STATMASK_STALL) ret |= USB_STALL;
	return ret;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: sl8Recvにする。
	UPDATED		: 2014-06-26
*****************************************************************************/
int read_sl811(int num, char* data, int size) {
	char	pktstat, ret;
	int	timovr;

	if(size < 0) return 0;
	sl8Write(num, SL11H_BUFADDRREG, 0x10);
	sl8Write(num, SL11H_BUFLNTHREG, size);
	sl8Write(num, SL11H_DEVADDRREG, sl811[num].addr);
	sl8Write(num, SL11H_PIDEPREG, PID_IN | data[USB_EP]);
	for(timovr = 0;timovr < 200;timovr++) {
		sl8Write(num, SL11H_HOSTCTLREG, (data[USB_TOGGLE] == 0) ? DATA0_RD : DATA1_RD);
		waitus(200);
		while((sl8Read(num, SL11H_INTSTATREG) & SL11H_INTMASK_XFERDONE) == 0);
		pktstat = sl8Read(num, SL11H_PKTSTATREG);
		if(!(pktstat & SL11H_STATMASK_NAK)) break;
	}
	if(pktstat & SL11H_STATMASK_ACK) sl8BufRead(num, 0x10, data, size);
	ret = 0;
	if(pktstat & SL11H_STATMASK_ACK) ret |= USB_ACK;
	if(pktstat & SL11H_STATMASK_NAK) ret |= USB_NAK;
	if(pktstat & SL11H_STATMASK_STALL) ret |= USB_STALL;
	return ret;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: sl8Seekにする
	UPDATED		: 2014-06-26
*****************************************************************************/
int seek_sl811(int num, int position) {
	if(num >= SL_NUM) return -1;
	sl811[num].addr = position;
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: T.B.D.
	ARGUMENT	: T.B.D.
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-26
*****************************************************************************/
int ioctl_sl811(int num, long data, int op) {
	static char	getdesc[8] = {0x80, 0x06, 0, 1, 0, 0, 64, 0};
	static char	setaddr[8] = {0x00, 0x05, 2, 0, 0, 0, 0, 0};
	char		buf[10];
	int		ret;

	if(num >= SL_NUM) return -1;
	ret = 0;
	switch(op) {
	case USB_RESET:
		ret = -1;
		sl811[num].addr = 0;
		if(USBReset(num) != 0) break;
		if((write_setup(num, getdesc) & USB_ACK) == 0) break;
		buf[USB_EP] = 0, buf[USB_TOGGLE] = 1;
		if((read_sl811(num, buf, 8) & USB_ACK) == 0) break;
		buf[USB_EP] = 0, buf[USB_TOGGLE] = 1;
		if((write_sl811(num, buf, 2) & USB_ACK) == 0) break;
		if((write_setup(num, setaddr) & USB_ACK) == 0) break;
		buf[USB_EP] = 0, buf[USB_TOGGLE] = 1;
		if((read_sl811(num, buf, 0) & USB_ACK) == 0) break;
		sl811[num].addr = data;
		ret = buf[7];
		break;
	case USB_GETSPEED:
		ret = sl811[num].speed;
		break;
	case USB_SETUP:
		ret = write_setup(num, (char*)data);
		break;
	default:
		ret = -1;
	}
	return ret;
}

/***** End Of File *****/
