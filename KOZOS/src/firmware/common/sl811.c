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

#define SL8REG_HOST_CTL        0
#define SL8REG_BUF_ADDR        1
#define SL8REG_BUF_LEN        2
#define SL8REG_PKT_STAT        3	/* read */
#define SL8REG_PID_EP          3	/* write */
#define SL8_XFERCNTREG        4	/* read */
#define SL8_DEVADDRREG        4	/* write */
#define SL8_CTLREG1           5
#define SL8_INTENBLREG        6

#define SL8REG_HOST_CTL_B      8
#define SL8REG_BUF_ADDR_B      9
#define SL8REG_BUF_LEN_B      10
#define SL8REG_PKT_STAT_B      11	/* read */
#define SL8REG_PID_EP_B        11	/* write */
#define SL8_XFERCNTREG_B      12	/* read */
#define SL8_DEVADDRREG_B      12	/* write */

#define SL8_INTSTATREG        13	/* write clears bitwise */
#define SL8_HWREVREG          14	/* read */
#define SL8_SOFLOWREG         14	/* write */
#define SL8_SOFTMRREG         15	/* read */
#define SL8_CTLREG2           15	/* write */
#define SL8_DATA_START        16

/* Host control register bits (addr 0) */

#define SL8_HCTLMASK_ARM      1
#define SL8_HCTLMASK_ENBLEP   2
#define SL8_HCTLMASK_WRITE    4
#define SL8_HCTLMASK_ISOCH    0x10
#define SL8_HCTLMASK_AFTERSOF 0x20
#define SL8_HCTLMASK_SEQ      0x40
#define SL8_HCTLMASK_PREAMBLE 0x80

/* Packet status register bits (addr 3) */

#define SL8_STATMASK_ACK      1
#define SL8_STATMASK_ERROR    2
#define SL8_STATMASK_TMOUT    4
#define SL8_STATMASK_SEQ      8
#define SL8_STATMASK_SETUP    0x10
#define SL8_STATMASK_OVF      0x20
#define SL8_STATMASK_NAK      0x40
#define SL8_STATMASK_STALL    0x80

/* Control register 1 bits (addr 5) */
#define SL8_CTL1MASK_DSBLSOF  1
#define SL8_CTL1MASK_NOTXEOF2 4
#define SL8_CTL1MASK_DSTATE   0x18
#define SL8_CTL1MASK_NSPD     0x20
#define SL8_CTL1MASK_SUSPEND  0x40
#define SL8_CTL1MASK_CLK12    0x80

#define SL8_CTL1VAL_RESET     8

/* Interrut enable (addr 6) and interrupt status register bits (addr 0xD) */

#define SL8_INTMASK_XFERDONE  1
#define SL8_INTMASK_SOFINTR   0x10
#define SL8_INTMASK_INSRMV    0x20
#define SL8_INTMASK_USBRESET  0x40
#define SL8_INTMASK_DSTATE    0x80	/* only in status reg */

/* HW rev and SOF lo register bits (addr 0xE) */

#define SL8_HWRMASK_HWREV     0xF0

/* SOF counter and control reg 2 (addr 0xF) */

#define SL8_CTL2MASK_SOFHI    0x3F
#define SL8_CTL2MASK_DSWAP    0x40
#define SL8_CTL2MASK_HOSTMODE 0xae

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
typedef struct{
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
	*(s_sl8.addr_reg) = cOffset;
	return *(s_sl8.data_reg);
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
	*(s_sl8.addr_reg) = cOffset;
	*(s_sl8.data_reg) = cBuf;

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

	*(s_sl8.addr_reg) = cOffset;

	for(i = 0; i < cSize; i++, pcBuf++){
		*pcBuf = *(s_sl8.data_reg);
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

	*(s_sl8.addr_reg) = cOffset;
	for(i = 0; i < cSize; i++, pcBuf++){
		*(s_sl8.data_reg) = *pcBuf;
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
	char	cBuf[256];

	for(i = 16;i < 256; i++){
		cBuf[i] = sl8Read(i);
		sl8Write(i, i);
	}

	for(i = 16;i < 256; i++){
		cData = sl8Read(i);
		if(cData != i){
			iRet = -1;
		}
	}

	for(i = 16; i < 256; i++){
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
static int sl8Reset(void)
{
	int	iStat;
	
	sl8Write(SL8_CTLREG2, 0xae);
	sl8Write(SL8_CTLREG1, 0x08);	// reset USB
	waitms(20);				// 20ms
	sl8Write(SL8_CTLREG1, 0);	// remove SE0

	for(iStat = 0; iStat < 100; iStat++){
		sl8Write(SL8_INTSTATREG, 0xff); // clear all interrupt bits
	}
	iStat = sl8Read(SL8_INTSTATREG);
	if(iStat & 0x40){  // Check if device is removed
		s_sl8.speed = 0;	// None
		sl8Write(SL8_INTENBLREG, SL8_INTMASK_XFERDONE | 
			   SL8_INTMASK_SOFINTR | SL8_INTMASK_INSRMV);
		return -1;
	}

	sl8Write(SL8REG_BUF_LEN_B, 0);	//zero lenth
	sl8Write(SL8REG_PID_EP_B, 0x50);	//send SOF to EP0
	sl8Write(SL8_DEVADDRREG_B, 0x01);	//address0
	sl8Write(SL8_SOFLOWREG, 0xe0);

	if(!(iStat & 0x80)){
		s_sl8.speed = USB_LOW;	// Low
		sl8Write(SL8_CTLREG1, 0x8);
		waitms(20);
		sl8Write(SL8_SOFTMRREG, 0xee);
		sl8Write(SL8_CTLREG1, 0x21);
		sl8Write(SL8REG_HOST_CTL_B, 0x01);
		for(iStat = 0; iStat < 20; iStat++){
			sl8Write(SL8_INTSTATREG, 0xff);
		}
	}
	else{
		s_sl8.speed = USB_FULL;	// Full
		sl8Write(SL8_CTLREG1, 0x8);
		waitms(20);
		sl8Write(SL8_SOFTMRREG, 0xae);
		sl8Write(SL8_CTLREG1, 0x01 );
		sl8Write(SL8REG_HOST_CTL_B, 0x01);
		sl8Write(SL8_INTSTATREG, 0xff);
	}

	sl8Write(SL8_INTENBLREG, SL8_INTMASK_XFERDONE | 
		   SL8_INTMASK_SOFINTR|SL8_INTMASK_INSRMV);
	return 0;
}

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

	sl8BufWrite(0x10, pcBuf, 8);
	sl8Write(SL8REG_BUF_ADDR, 0x10);
	sl8Write(SL8REG_BUF_LEN, 8);
	sl8Write(SL8_DEVADDRREG, s_sl8.addr);
	sl8Write(SL8REG_PID_EP, PID_SETUP);
	sl8Write(SL8REG_HOST_CTL, DATA0_WR);
	waitus(200);
	while((sl8Read(SL8_INTSTATREG) & SL8_INTMASK_XFERDONE) == 0){
		//do nothing
	}

	cPktStat = sl8Read(SL8REG_PKT_STAT);

	iRet = 0;

	if(cPktStat & SL8_STATMASK_ACK){
		iRet |= USB_ACK;
	}
	if(cPktStat & SL8_STATMASK_NAK){
		iRet |= USB_NAK;
	}
	if(cPktStat & SL8_STATMASK_STALL){
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
	P1DDR = 0xff;	/* A0-7    is enable */
	P2DDR = 0xff;	/* A8-15   is enable */
	P5DDR = 0x01;	/* A16     is enable */
	P8DDR = 0x0e;	/* CS1-3 is enable */

	s_sl8.addr_reg = (volatile char *)0x200001;
	s_sl8.data_reg = (volatile char *)0x200003;
	s_sl8.addr = 0;

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
	int		iRet;
	char	cPktStat;

	if(iSize < USB_HDRSIZ){
		return 0;
	}
	sl8BufWrite(0x10, &pcBuf[USB_HDRSIZ], iSize - USB_HDRSIZ);
	sl8Write(SL8REG_BUF_ADDR, 0x10);
	sl8Write(SL8REG_BUF_LEN, iSize - USB_HDRSIZ);
	sl8Write(SL8_DEVADDRREG, s_sl8.addr);
	sl8Write(SL8REG_PID_EP, PID_OUT | pcBuf[USB_EP]);
	sl8Write(SL8REG_HOST_CTL, (pcBuf[USB_TOGGLE] == 0) ? DATA0_WR : DATA1_WR);
	waitus(200);
	while((sl8Read(SL8_INTSTATREG) & SL8_INTMASK_XFERDONE) == 0){
		//do nothing
	}
	cPktStat = sl8Read(SL8REG_PKT_STAT);
	iRet = 0;

	if(cPktStat & SL8_STATMASK_ACK){
		iRet |= USB_ACK;
	}
	if(cPktStat & SL8_STATMASK_NAK){
		iRet |= USB_NAK;
	}
	if(cPktStat & SL8_STATMASK_STALL){
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
	int	timovr;

	if(iSize < 0){
		return 0;
	}
	sl8Write(SL8REG_BUF_ADDR, 0x10);
	sl8Write(SL8REG_BUF_LEN, iSize);
	sl8Write(SL8_DEVADDRREG, s_sl8.addr);
	sl8Write(SL8REG_PID_EP, PID_IN | pcBuf[USB_EP]);

	for(timovr = 0;timovr < 200;timovr++){
		sl8Write(SL8REG_HOST_CTL, (pcBuf[USB_TOGGLE] == 0) ? DATA0_RD : DATA1_RD);
		waitus(200);

		while((sl8Read(SL8_INTSTATREG) & SL8_INTMASK_XFERDONE) == 0){
			//do nothing
		}

		cPktStat= sl8Read(SL8REG_PKT_STAT);
		if(!(cPktStat & SL8_STATMASK_NAK)){
			break;
		}
	}

	if(cPktStat & SL8_STATMASK_ACK){
		sl8BufRead(0x10, pcBuf, iSize);
	}
	iRet = 0;
	if(cPktStat & SL8_STATMASK_ACK){
		iRet |= USB_ACK;
	}
	if(cPktStat & SL8_STATMASK_NAK){
		iRet |= USB_NAK;
	}
	if(cPktStat & SL8_STATMASK_STALL){
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
	s_sl8.addr = iPos;
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
		s_sl8.addr = 0;
		if(sl8Reset() != 0){
			break;
		}
		if((write_setup(getdesc) & USB_ACK) == 0){
			break;
		}
		pcBuf[USB_EP] = 0, pcBuf[USB_TOGGLE] = 1;
		if((read_sl811(num, pcBuf, 8) & USB_ACK) == 0){
			break;
		}
		pcBuf[USB_EP] = 0, pcBuf[USB_TOGGLE] = 1;
		if((write_sl811(num, pcBuf, 2) & USB_ACK) == 0){
			break;
		}
		if((write_setup(setaddr) & USB_ACK) == 0){
			break;
		}
		pcBuf[USB_EP] = 0, pcBuf[USB_TOGGLE] = 1;
		if((read_sl811(num, pcBuf, 0) & USB_ACK) == 0){
			break;
		}

		s_sl8.addr = lData;
		iRet = pcBuf[7];
		break;

	case USB_GETSPEED:
		iRet = s_sl8.speed;
		break;

	case USB_SETUP:
		iRet = write_setup((char*)lData);
		break;

	default:
		iRet = -1;
	}

	return iRet;
}

/***** End Of File *****/

