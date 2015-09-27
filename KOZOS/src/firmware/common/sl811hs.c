/*****************************************************************************
	オーダー	: SL811HSドライバ
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: sl811hs.c
	接頭辞		: sl811
	作成日時	: 2015-05-08
	メモ		:
				 SL811HSはチップセレクト2に設定されており、0x200001にアドレス
				 レジスタ、0x200003にデータレジスタが接続されている。
				 ここに値を渡すことで、デバイス側のアドレスを設定し、その位置
				 のデータを取得することができる。
				 例えば、
				 *(volatile char*)0x200001 = 0x10;
				 とやると、
				 data = *(volatile char*)0x200003;
				 とすることで、USBホストデバイスからデータを取得することが
				 できる。
				 0x200001と0x200003の根拠は、USBが接続されているアドレスが
				 チップセレクト1のデータバスの上位8ビット(D8〜D15)だからである。
				 また、USBアドレスバスのA0が、H8アドレスバスのA1に接続されて
				 いて、この意味は、アドレス自動インクリメントモードのオンオフ
				 である。

				 USB-AとUSB-Bは、おそらく接続する部分の端子の違いだと思う。
				 ホスト側とデバイス側を分けるために、形に区別があるようだ。
				 したがって、ホストとデバイスが両方共USB-A、というようなことは
				 ないらしい。
				 ちなみに、USB-Aは一般的な平べったい形の端子で、USB-Bは正方形
				 に近いような端子。
*****************************************************************************/
/*
	作成日時
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include "reg3069.h"
#include "usbhlib.h"
#include "sl811hs.h"
#include "lib.h"

/*****************************************************************************
	定義
*****************************************************************************/
/****** レジスタ定義 *****/
#define SL811REG_HOST_CTRL		(0x00)	/* USB-A Host Control Register */
/*
	BIT0 : Arm
	BIT1 : Enable
	BIT2 : Direction
	BIT3 : Reserve
	BIT4 : ISO
	BIT5 : SyncSOF
	BIT6 : Data Toggle Bit
	BIT7 : Preamble
*/

#define SL811REG_BASE_ADDR		(0x01)	/* USB-A Host Base Address */
/*
	BIT0-BIT7 : Base Address
*/

#define SL811REG_BASE_LEN		(0x02)	/* USB-A Host Base Length */
/*
	BIT0-BIT7 : Maximam packe size 
*/

#define SL811REG_PKT_STAT		(0x03)	/* USB Packet Status(Read) */
#define SL811REG_PID_EP			(0x03)	/* USB-A Host PID, Device Endpoint (Write) */
/*
	(When Read)
	BIT0 : ACK
	BIT1 : Error
	BIT2 : Time-out
	BIT3 : Sequence
	BIT4 : Setup
	BIT5 : Overflow
	BIT6 : NAK
	BIT7 : STALL

	(When Write)
	BIT0 : End Point 0
	BIT1 : End Point 1
	BIT2 : End Point 2
	BIT3 : End Point 3
	BIT4 : Packet IDentifier 0
	BIT5 : Packet IDentifier 1
	BIT6 : Packet IDentifier 2
	BIT7 : Packet IDentifier 3
*/

	/* PID(上位4ビット)に設定(書き込み)する値。上位4ビット */
	#define PID_TYPE_SETUP			(0xD0) /* 1101 */
	#define PID_TYPE_IN				(0x90) /* 1001 */
	#define PID_TYPE_OUT			(0x10) /* 0001 */
	#define PID_TYPE_SOF			(0x50) /* 0101 */
	#define PID_TYPE_PREAMBLE		(0xC0) /* 1100 */
	#define PID_TYPE_NAK			(0xA0) /* 1010 */
	#define PID_TYPE_STALL			(0xE0) /* 1110 */
	#define PID_TYPE_DATA0			(0x30) /* 0011 */
	#define PID_TYPE_DATA1			(0xB0) /* 1011 */

#define SL811REG_TRANS_CNT		(0x04)	/* Transfer Count(Read) */
#define SL811REG_DEV_ADDR		(0x04)	/* USB-A Host Device Address (Write) */
/*
	(When Read)
	BIT0-BIT7 : Transfer Count

	(When Write)
	BIT0-BIT6	: USB Address
	BIT7		: Reserved. must be set to zero.
*/

#define SL811REG_CTL1			(0x05)	/* Control Register1 */
/*
	BIT0 : SOF enable/disable
	BIT1 : Reserved
	BIT2 : Reserved
	BIT3 : USB Engine Reset
	BIT4 : J-K state force
	BIT5 : USB Speed
	BIT6 : Suspend
	BIT7 : Reserved
*/

#define SL811REG_INTR_ENB		(0x06)	/* Interrupt Enable Register */
/*
	BIT0 : USB-A Done
	BIT1 : USB-B Done
	BIT2 : Reserved
	BIT3 : Reserved
	BIT4 : SOF Timer
	BIT5 : Inserted/Removed
	BIT6 : Device Detect/Resume
	BIT7 : Reserved
*/

/* #define SL811REG_0x07		(0x07)	Reserved */
/* USB-B(0x08〜0x0C)のビットポジションは、USB-A(0x00〜0x04)と同じ */
#define SL811REG_HOST_CTRL_B	(0x08)	/* USB-B Host Control Register */
#define SL811REG_BASE_ADDR_B	(0x09)	/* USB-B Host Base Address */
#define SL811REG_BASE_LEN_B		(0x0A)	/* USB-B Host Base Length */
#define SL811REG_PKT_STAT_B		(0x0B)	/* USB Status (Read) */
#define SL811REG_PID_EP_B		(0x0B)	/* USB-B Host Packet ID(PID),Device EndPoint(Write) */
#define SL811REG_TRANS_CNT_B	(0x0C)	/* Transfer Count (Read) */
#define SL811REG_DEV_ADDR_B		(0x0C)	/* USB-B Host Device Address (Write) */

#define SL811REG_INT_STAT		(0x0D)	/* Status Register. write clears bitwise */
/*
	BIT0 : USB-A
	BIT1 : USB-B
	BIT2 : Reserved
	BIT3 : Reserved
	BIT4 : SOF Timer
	BIT5 : Inserted/Removed
	BIT6 : Device Detect/Resume
	BIT7 : D+
*/

#define SL811REG_HW_REV			(0x0E)	/* HW Revision Register (Read) */
#define SL811REG_SOF_LOW		(0x0E)	/* "Start Of Frame" LOW Address(Write) */
/*
	(When Read)
	BIT0-BIT3 : Reserved
	BIT4-BIT7 : Hardware Revision

	(When Write)
	BIT0-BIT7 : SOF Low Address
*/

#define SL811REG_SOF_TMR		(0x0F)	/* SOF Counter HIGH (Read) */
#define SL811REG_CTL2			(0x0F)	/* Control Register2 (Write) */
/*
	(When Read)
	BIT0-BIT7 : SOF High Counter

	(When Write)
	BIT0-BIT5 : SOF High Counter
	BIT6 : D+/D- Data Polarity Swap(Low/High Speed)
	BIT7 : Master/Slave selection
*/

#define SL811REG_DATA_START		(0x10)	/* 0x10-0xFF Memory Buffer */
#define SL811REG_DATA_END		(0xFF)	/* 0x10-0xFF Memory Buffer */

#define SL811_CTL2_HOSTMODE (BIT7 | 0x2E)

/* ホストコントロールレジスタで使用する */
#define DATA0_WR	(BIT0 | BIT1 | BIT2)
#define DATA1_WR	(BIT0 | BIT1 | BIT2 | BIT7)
#define ZDATA0_WR	(BIT0 | BIT2)
#define ZDATA1_WR	(BIT0 | BIT2 | BIT7)
#define DATA0_RD	(BIT0 | BIT1)
#define DATA1_RD	(BIT0 | BIT1 | BIT7)

/* 以下は、SL811HSを操作するものではなく、USB仕様上のもの */
#define USB_RESET	46
#define USB_SETUP	47
#define USB_GETSPEED	48
 #define USB_LOW	1
 #define USB_FULL	2

#define USB_EP		1
#define USB_TOGGLE	2
#define USB_HEAD_SIZE	2

#define USB_ACK		1
#define USB_NAK		2
#define USB_STALL	4

/* USBホストコントローラ(ADDR,DATA)のH8上でのアドレス */
#define	USB_DEV0_ADDR_REG	((volatile char *)0x200001)
#define	USB_DEV0_DATA_REG	((volatile char *)0x200003)

#define	BUF_SIZE	(256)
#define	IOCTL_BUF_SIZE	(8)


/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
typedef struct _get_desc_struct {
	short type;
	char *data;
	int datasize;
	int epsize;
	char index;
	char cDummy;
	short lang;
} GET_DESCTYPE;


/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/
/* ワークバッファ */
/* 必要に応じて、ちゃんと初期化すること */
static char	s_cWorkBuf[BUF_SIZE];

static char s_cSpeed;
static char s_cAddr;
static volatile char* pSl811AddrReg = USB_DEV0_ADDR_REG;
static volatile char* pSl811DataReg = USB_DEV0_DATA_REG;


/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/
static char sl811RecvByte(char cOffset);
static void sl811SendByte(char cOffset, char cBuf);
static void sl811RecvBuf(char cOffset, char* pcBuf, uint16 usSize);
static void sl811SendBuf(char cOffset, char* pcBuf, uint16 usSize);


/*****************************************************************************
	DISCRIPTION	: SL811HSバイト受信
	ARGUMENT	: cOffcet		= データ取得位置
	RETURN		: 取得データ
	NOTE		: -
	UPDATED		: 2015-05-20
*****************************************************************************/
static char sl811RecvByte(char cOffset)
{
	char	cData;
	*(pSl811AddrReg) = cOffset;
	cData = *(pSl811DataReg);
	return	(cData);
}

/*****************************************************************************
	DISCRIPTION	: SL811HSバイト送信
	ARGUMENT	: cOffcet		= データ書き込み位置
				  cBuf	  		= 書き込み
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-05-20
*****************************************************************************/
static void sl811SendByte(char cOffset, char cBuf)
{
	*(pSl811AddrReg) = cOffset;
	*(pSl811DataReg) = cBuf;
	return;
}

/*****************************************************************************
	DISCRIPTION	: SL811HSデータ受信
	ARGUMENT	: cOffcet		= データ取得位置
				  pcBuf	  		= データ格納バッファ
				  usSize		= 取得データサイズ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-05-20
*****************************************************************************/
static void sl811RecvBuf(char cOffset, char* pcBuf, uint16 usSize)
{
	int	i;
	if(usSize <= 0){
		return;
	}
	*(pSl811AddrReg) = cOffset;
	for(i = 0; usSize > i; i++, pcBuf++){
		*pcBuf = *(pSl811DataReg);
	}
	return;
}

/*****************************************************************************
	DISCRIPTION	: SL811HSデータ送信
	ARGUMENT	: cOffcet		= データ書き込み位置
				  pcBuf	  		= 書き込み用バッファ
				  usSize		= 書き込みデータサイズ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-05-20
*****************************************************************************/
static void sl811SendBuf(char cOffset, char* pcBuf, uint16 usSize)
{
	int i;
	if(usSize <= 0){
		return;
	}
	*(pSl811AddrReg) = cOffset;
	for(i = 0; usSize > i; i++, pcBuf++){
		*(pSl811DataReg) = *pcBuf;
	}
	return;
}

/*****************************************************************************
	DISCRIPTION	: sl811hsが取り付けられているかどうかのテスト
	ARGUMENT	: usDevNum	= デバイス番号
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2015-05-20
*****************************************************************************/
static int sl811RegTest(void)
{
	int		iRet = 0;
	int		i;
	char	cData;

	for(i = SL811REG_DATA_START; i < SL811REG_DATA_END; i++){
		/* データレジスタの全ての値のバックアップを取る */
		s_cWorkBuf[i] = sl811RecvByte(i);
		/* 全てのデータレジスタに値をセット */
		sl811SendByte(i, i);
	}

	for(i = SL811REG_DATA_START; i < SL811REG_DATA_END; i++){
		cData = sl811RecvByte(i);
		if(cData != i){
			iRet = -1;
		}
	}

	/* データの書き戻し */
	for(i = SL811REG_DATA_START; i < SL811REG_DATA_END; i++){
		sl811SendByte(i, s_cWorkBuf[i]);
	}
	return iRet;
}

/*****************************************************************************
	DISCRIPTION	: sl811の初期化
	ARGUMENT	: -
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2015-07-18
*****************************************************************************/
static int sl811Reset(void)
{
	int		iStatus;
	int		i;

	/* USBホストモードにする */
	sl811SendByte(SL811REG_CTL2, SL811_CTL2_HOSTMODE);

	/* USBリセット */
	sl811SendByte(SL811REG_CTL1, BIT3);
	/* cypress仕様"USB Reset Sequence"より、50ms待つ */
	waitms(50);

	/* cypress仕様"USB Reset Sequence"より、0セット */
	sl811SendByte(SL811REG_CTL1, 0);

	for(i = 0;i < 100;i++) {
		sl811SendByte(SL811REG_INT_STAT, 0xff); // clear all interrupt bits
	}
	/* すべてのビットを立てることにより初期化 */
	//sl811SendByte(SL811REG_INT_STAT, 0xFF);

	/* 初期化後、値を取得 */
	iStatus = sl811RecvByte(SL811REG_INT_STAT);
	/* デバイスが外されてないか確認 */
	if(iStatus & BIT6){	/* 外されてた */
		s_cSpeed = 0;
		sl811SendByte(SL811REG_INTR_ENB, BIT0 | BIT4 | BIT5);
		return(-1);
	}

	/* もう一回リセットしてる */
	sl811SendByte(SL811REG_CTL1, BIT3);
	waitms(50);

	/* D+ピンの値ビット。フルorロースピードの検出 */
	if(iStatus & BIT7){
		puts("Full Speed\n");
		s_cSpeed = USB_FULL;	// フルスピード 

		sl811SendByte(SL811REG_SOF_TMR, SL811_CTL2_HOSTMODE);
		sl811SendByte(SL811REG_SOF_LOW, 0xE0);
		sl811SendByte(SL811REG_CTL1, BIT0);
		sl811SendByte(SL811REG_HOST_CTRL_B, BIT0);
		sl811SendByte(SL811REG_INT_STAT, 0xFF);
	}
	else {
		puts("Low Speed\n");
		s_cSpeed = USB_LOW;	// ロースピード

		sl811SendByte(SL811REG_CTL2, BIT1 | BIT2 | BIT3 | BIT5 | BIT6 | BIT7);
		sl811SendByte(SL811REG_CTL1, BIT0 | BIT5);
		sl811SendByte(SL811REG_HOST_CTRL_B, BIT0);
		sl811SendByte(SL811REG_INT_STAT, 0xFF);
		for(i = 0;i < 20;i++) {
			sl811SendByte(SL811REG_INT_STAT, 0xff);
		}
	}

	/* 割り込みの何かを設定している */
	sl811SendByte(SL811REG_INTR_ENB, BIT0 | BIT4 | BIT5);
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: sl811書き込み設定
	ARGUMENT	: cData		= 書き込みデータ
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2015-07-18
*****************************************************************************/
static int sl811WriteSetup(char* cBuf)
{
	char	cPacketStatus;
	char	cIntrStatus;
	char	cRet = 0;

	sl811SendBuf(SL811REG_DATA_START, cBuf, IOCTL_BUF_SIZE);

	sl811SendByte(SL811REG_BASE_ADDR, 0x10);
	sl811SendByte(SL811REG_BASE_LEN, IOCTL_BUF_SIZE);
	sl811SendByte(SL811REG_DEV_ADDR, s_cAddr);
	sl811SendByte(SL811REG_PID_EP, PID_TYPE_SETUP);
	sl811SendByte(SL811REG_HOST_CTRL, DATA0_WR);
	waitus(200);

	/* なんか値を取得するまでループ待ち */
	do{
		cIntrStatus = sl811RecvByte(SL811REG_INT_STAT);
	}while((cIntrStatus & BIT0) == 0);

	cPacketStatus = sl811RecvByte(SL811REG_PKT_STAT);
	if(cPacketStatus & BIT0){
		cRet |= USB_ACK;
	}
	if(cPacketStatus & BIT6){
		cRet |= USB_NAK;
	}
	if(cPacketStatus & BIT7){
		cRet |= USB_STALL;
	}
	return (int)cRet;
}

/*****************************************************************************
	DISCRIPTION	: sl811書き込み(公開関数)
	ARGUMENT	: pcBuf		= 書き込みバッファ
				  iSize		= バッファサイズ
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2015-07-18
*****************************************************************************/
int usbPutBuf(char* pcBuf, int iSize)
{
	char	cPacketStatus;
	char	cIntrStatus;
	char	cRet = 0;
	int		iSendSize = iSize - USB_HEAD_SIZE;

	if(iSize < USB_HEAD_SIZE){
		return 0;
	}
	sl811SendBuf(SL811REG_DATA_START, &pcBuf[USB_HEAD_SIZE], iSendSize);

	sl811SendByte(SL811REG_BASE_ADDR, 0x10);
	sl811SendByte(SL811REG_BASE_LEN, iSendSize);
	sl811SendByte(SL811REG_DEV_ADDR, s_cAddr);
	sl811SendByte(SL811REG_PID_EP, PID_TYPE_OUT | pcBuf[USB_EP]);
	sl811SendByte(SL811REG_HOST_CTRL, (pcBuf[USB_TOGGLE] == 0) ? DATA0_WR : DATA1_WR);
	waitus(200);

	/* なんか値を取得するまでループ待ち */
	do{
		cIntrStatus = sl811RecvByte(SL811REG_INT_STAT);
	}while(!(cIntrStatus & BIT0));
	//}while((cIntrStatus & BIT0) == 0);

	cPacketStatus = sl811RecvByte(SL811REG_PKT_STAT);
	if(cPacketStatus & BIT0){
		cRet |= USB_ACK;
	}
	if(cPacketStatus & BIT6){
		cRet |= USB_NAK;
	}
	if(cPacketStatus & BIT7){
		cRet |= USB_STALL;
	}
	return (int)cRet;
}

/*****************************************************************************
	DISCRIPTION	: sl811読み込み(公開関数)
	ARGUMENT	: pcBuf		= 書き込みデータ
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2015-07-21
*****************************************************************************/
int usbGetBuf(char* pcBuf, int iSize) {
	char	cPacketStatus;
	char	cIntrStatus;
	char	cRet = 0;
	int		iTimeOver;

	if(iSize < 0){
		return 0;
	}
	sl811SendByte(SL811REG_BASE_ADDR, 0x10);
	sl811SendByte(SL811REG_BASE_LEN, iSize);
	sl811SendByte(SL811REG_DEV_ADDR, s_cAddr);
	sl811SendByte(SL811REG_PID_EP, PID_TYPE_IN | pcBuf[USB_EP]);
	for(iTimeOver = 0; iTimeOver < 200;  iTimeOver++) {
		sl811SendByte(SL811REG_HOST_CTRL, (pcBuf[USB_TOGGLE] == 0) ? DATA0_RD : DATA1_RD);
		waitus(200);
		/* 割り込みフラグが立つまでループ待ち */
		//do{
		//	cIntrStatus = sl811RecvByte(SL811REG_INT_STAT);
		//	puts("do while\n");
		//}while((cIntrStatus & BIT0) == 0);
		while((sl811RecvByte(SL811REG_INT_STAT) & BIT0) == 0);

		cPacketStatus = sl811RecvByte(SL811REG_PKT_STAT);

		if(!(cPacketStatus & BIT6)){
			puts("break\n");
			break;
		}
	}

	printhex(cPacketStatus,2,1);
	puts("\n");

	if(cPacketStatus & BIT0){
		sl811RecvBuf(SL811REG_DATA_START, pcBuf, iSize);
	}

	//cPacketStatus = sl811RecvByte(SL811REG_PKT_STAT);
	if(cPacketStatus & BIT0){
		cRet |= USB_ACK;
	}
	if(cPacketStatus & BIT6){
		cRet |= USB_NAK;
	}
	if(cPacketStatus & BIT7){
		cRet |= USB_STALL;
	}
	return (int)cRet;
}

/*****************************************************************************
	DISCRIPTION	: USBリセット
	ARGUMENT	: iPos		= バッファ位置
	RETURN		: -
	NOTE		: ハードコーディングですねぇ。。。
	UPDATED		: 2015-07-24
*****************************************************************************/
int usbReset(char cData)
{
	static char	cGetDescCmdBuf[IOCTL_BUF_SIZE] = {0x80, 0x06, 0, 1, 0, 0, 64, 0};
	static char	cSetAddrCmdBuf[IOCTL_BUF_SIZE] = {0x00, 0x05, 2, 0, 0, 0, 0, 0};
	char		cBuf[10];
	int		iRet = -1;

	s_cAddr = 0;

	if(sl811Reset() != 0){
		puts("0 reset\n");
		return iRet;
	}
	if((sl811WriteSetup(cGetDescCmdBuf) & USB_ACK) == 0){
		puts("1 reset\n");
		return iRet;
	}

	cBuf[USB_EP] = 0;
	cBuf[USB_TOGGLE] = 1;

	if((usbGetBuf(cBuf, 8) & USB_ACK) == 0){
		puts("2 reset\n");
		return iRet;
	}

	cBuf[USB_EP] = 0;
	cBuf[USB_TOGGLE] = 1;

	if((usbPutBuf(cBuf, 2) & USB_ACK) == 0){
		puts("3 reset\n");
		return iRet;
	}

	if((sl811WriteSetup(cSetAddrCmdBuf) & USB_ACK) == 0){
		puts("4 reset\n");
		return iRet;
	}

	cBuf[USB_EP] = 0;
	cBuf[USB_TOGGLE] = 1;

	if((usbGetBuf(cBuf, 0) & USB_ACK) == 0){
		puts("5 reset\n");
		return iRet;
	}

	s_cAddr = cData;
	iRet = cBuf[7];

	return iRet;
}

/*****************************************************************************
	DISCRIPTION	: USB書き込みセットアップ
	ARGUMENT	: 		= 
	RETURN		: -
	NOTE		: ハードコーディングですねぇ。。。
	UPDATED		: 2015-07-24
*****************************************************************************/
int usbWriteSetup(char* pcBuf)
{
	int		iRet = 0;
	iRet = sl811WriteSetup(pcBuf);
	return iRet;
}

/*****************************************************************************
	DISCRIPTION	: USB書き込みセットアップ
	ARGUMENT	: 		= 
	RETURN		: -
	NOTE		: ハードコーディングですねぇ。。。
	UPDATED		: 2015-07-24
*****************************************************************************/
int usbGetSpeed(void)
{
	return s_cSpeed;
}

/*****************************************************************************
	DISCRIPTION	: ディスクリプタ取得
	ARGUMENT	: pDesc(O)		= ディスクリプタ格納バッファ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-07-24
*****************************************************************************/
void get_desc(GET_DESCTYPE* pDesc)
{
	SetupPKG	command;
	char		cToggle;
	char		cBuf[3];
	int			iRemain;
	int			iSize;
	int			iOffset;

	/* コマンド作成 */
	command.bmRequest = TRANS_TO_HOST;
	command.bRequest = GET_DESCRIPTOR;
	command.wValue = htole16((pDesc->type << 8) | pDesc->index);
	command.wIndex = pDesc->lang;
	command.wLength = htole16(pDesc->datasize);

	usbWriteSetup((char*)&command);

	iOffset = 0;
	cToggle = 1;
	cBuf[USB_TOGGLE] = 1;

	for(iRemain = pDesc->datasize; iRemain > 0; iRemain -= iSize){
		//iSize = (iRemain > pdesc->epsize) ? pDesc->epsize : iRemain;
		if(iRemain > pDesc->epsize){
			iSize = pDesc->epsize;
		}
		else{
			iSize = iRemain;
		}

		if(iSize < USB_HEAD_SIZE) {
			cBuf[USB_EP] = 0;
			usbGetBuf(cBuf, iSize);
			pDesc->data[iOffset] = cBuf[0];
		} else {
			pDesc->data[iOffset + USB_EP] = 0;
			pDesc->data[iOffset + USB_TOGGLE] = cToggle;
			usbGetBuf(&pDesc->data[iOffset], iSize);
		}

		if((iOffset == 0) && (pDesc->data[1] != CONFIG_TYPE)){
			iRemain = (int)pDesc->data[0] & 0xff;
		}

		cToggle = (cToggle == 0) ? 1 : 0;
		iOffset += iSize;
	}

	cBuf[USB_EP] = 0;
	cBuf[USB_TOGGLE] = 1;
	usbPutBuf(cBuf, 2);

	return;
}


/*****************************************************************************
	DISCRIPTION	: sl811バッファシーク
	ARGUMENT	: iPos		= バッファ位置
	RETURN		: -
	NOTE		: -
	UPDATED		: 2015-07-21
*****************************************************************************/
static void sl811Seek(int iPos)
{
	s_cAddr = iPos;
	return;
}




int usb_main()
{
	int		i, ep, cfgsiz, offset;
	char		buffer[128], *p;
	DevDesc		device;
	CfgDesc		config;
	StrDesc		string;
	IntfDesc	interface;
	EPDesc		endpoint[MAX_EP];
	int		epsize, epnum;
	short		vendor, product;
	char		class, subclass, protocol;
	GET_DESCTYPE	desctype;

	/* BUS  initialize */
	puts((uint08*)(uint08*)"usb_main...\n");
	P1DDR = 0xff;	/* A0-7    is enable */
	P2DDR = 0xff;	/* A8-15   is enable */
	P5DDR = 0x01;	/* A16     is enable */
	P8DDR = 0x0e;	/* CS1-3 is enable */
	s_cAddr = 0;
	if(sl811RegTest() != 0) {
		puts((uint08*)"USB not found!\n");
		return -1;
	}
	epsize = usbReset(2);
	if(epsize == -1) {
		puts((uint08*)"Device not found!\n");
		return -1;
	}
	puts((uint08*)"Size of endpoint is ");
	printhex(epsize, 2 ,1);
	puts((uint08*)"\n");
	desctype.type = DEVICE_TYPE;
	desctype.data = (char*)&device;
	desctype.datasize = sizeof(DevDesc);
	desctype.epsize = epsize;
	desctype.index = 0;
	desctype.lang = 0;

	get_desc(&desctype);
	vendor = le16toh(device.idVendor);
	product = le16toh(device.idProduct);
	puts((uint08*)"\nDevice desc : ");
	p = (char*)&device;
	for(i = 0;i < sizeof(DevDesc);i++){
		printhex("%02x ", (int)p[i] & 0xff, 0);
		printhex((uint32)(p[i] & 0xff), 2, 0);
		puts((uint08*)" ");
	}

	desctype.type = CONFIG_TYPE;
	desctype.data = (char*)&config;
	desctype.datasize = sizeof(CfgDesc);
	desctype.epsize = epsize;
	desctype.index = 0;
	desctype.lang = 0;

	get_desc(&desctype);
	cfgsiz = le16toh(config.wLength);
	if(cfgsiz > 128) cfgsiz = 128;
	puts((uint08*)"\nConfiguration desc : ");
	p = (char*)&config;
	for(i = 0;i < sizeof(CfgDesc);i++){
		printhex((uint32)(p[i] & 0xff), 2, 0);
		puts((uint08*)" ");
	}

	if(device.iProduct != 0){

		desctype.type = STRING_TYPE;
		desctype.data = (char*)&string;
		desctype.datasize = sizeof(StrDesc);
		desctype.epsize = epsize;
		desctype.index = 0;
		desctype.lang = 0;

		get_desc(&desctype);
		puts((uint08*)"\nString desc : ");
		p = (char*)&string;
		for(i = 0;i < sizeof(StrDesc);i++){
			printhex((uint32)(p[i] & 0xff), 2, 0);
			puts((uint08*)" ");
		}


		desctype.type = STRING_TYPE;
		desctype.data = buffer;
		desctype.datasize = 128;
		desctype.epsize = epsize;
		desctype.index = device.iProduct;
		desctype.lang = le16toh(string.wLang);

		get_desc(&desctype);
		puts((uint08*)"\n[");
		p = (char*)&string;
		for(i = 2;i < ((int)buffer[0] & 0xff);i+=2){
			putc(buffer[i]);
		}
		puts((uint08*)"]");
	}

		desctype.type = CONFIG_TYPE;
		desctype.data = buffer;
		desctype.datasize = cfgsiz;
		desctype.epsize = epsize;
		desctype.index = 0;
		desctype.lang = 0;

//	get_desc(CONFIG_TYPE, buffer, cfgsiz, epsize, 0, 0);
	get_desc(&desctype);
	offset = sizeof(CfgDesc);
	memcpy((char*)&interface, &buffer[offset], sizeof(IntfDesc));
	puts((uint08*)"\nInterface desc : ");
	p = (char*)&interface;
	for(i = 0;i < sizeof(IntfDesc);i++){
		printhex((uint32)(p[i] & 0xff), 2, 0);
		puts((uint08*)" ");
	}
	offset += sizeof(IntfDesc);
	epnum = interface.bEndPoints;
	class = interface.iClass;
	subclass = interface.iSub;
	protocol = interface.iProto;
	for(ep = 0;ep < epnum;ep++) {
		if(ep == MAX_EP) break;
		memcpy((char*)&endpoint[ep], &buffer[offset], sizeof(EPDesc));
		offset += sizeof(EPDesc);
		puts((uint08*)"\nEndpoint desc(%d) : ");
		printhex(ep, 2, 0);
		p = (char*)&endpoint[ep];
		for(i = 0;i < sizeof(EPDesc);i++){
			printhex((uint32)(p[i] & 0xff), 2, 0);
			puts((uint08*)" ");
		}
	}
	puts((uint08*)"\nEndpoint number is \n");
	printhex(epnum, 2, 0);
	puts((uint08*)"\n");

	puts((uint08*)"Vendor ");
	printhex((uint32)vendor,2,0);
	puts((uint08*)"product ");
	printhex((uint32)product,2,0);
	puts((uint08*)"Class ");
	printhex((uint32)(class & 0xff),2,0);
	puts((uint08*)"Sub ");
	printhex((uint32)(subclass & 0xff),2,0);
	puts((uint08*)"Proto ");
	printhex((uint32)(protocol & 0xff),2,0);
	puts((uint08*)"\n");
	return 0;
}

/***** End Of File *****/


