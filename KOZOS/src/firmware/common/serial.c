/*****************************************************************************
	オーダー	: シリアルデバイスドライバ
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: serial.c
	接頭辞		: srl
	作成日時	: 2013-09-03
*****************************************************************************/
/*
	改訂履歴
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include	"defines.h"
#include	"serial.h"

/*****************************************************************************
	定義
*****************************************************************************/
#define	SCI_NUM	(3)	/* SCIの数 */

#if 1	/* TODO defineではなくポインタ変数に変更 */
#define	SCI_0	((volatile H8_SCI*)0xffffb0)
#define	SCI_1	((volatile H8_SCI*)0xffffb8)
#define	SCI_2	((volatile H8_SCI*)0xffffc0)
#endif	/* TODO defineではなくポインタ変数に変更 */

/* SMR(Serial Mode Register)のビット定義 */
#define	SCI_SMR_CKS_PER01	(0<<0)	/* クロックセレクト */
#define	SCI_SMR_CKS_PER04	(1<<0)	/* クロックセレクト */
#define	SCI_SMR_CKS_PER16	(2<<0)	/* クロックセレクト */
#define	SCI_SMR_CKS_PER64	(3<<0)	/* クロックセレクト */
#define	SCI_SMR_MP			(1<<2)	/* マルチプロセッサモード */
#define	SCI_SMR_STOP		(1<<3)	/* ストップビット */
#define	SCI_SMR_OE			(1<<4)	/* パリティの種類 */
#define	SCI_SMR_PE			(1<<5)	/* パリティの有効・無効 */
#define	SCI_SMR_CHR			(1<<6)	/* データ長 */
#define	SCI_SMR_CA			(1<<7)	/* 同期式モード */

/* SCR(Serial Control Register)のビット定義 */ 
#define	SCI_SCR_CKE0		(1<<0)	/* クロックイネーブル */
#define	SCI_SCR_CKE1		(1<<1)	/* クロックイネーブル */
#define	SCI_SCR_TEIE		(1<<2)	/* TODO(調査) */
#define	SCI_SCR_MPIE		(1<<3)	/* TODO(調査) */
#define	SCI_SCR_RE			(1<<4)	/* 受信イネーブル */
#define	SCI_SCR_TE			(1<<5)	/* 送信イネーブル */
#define	SCI_SCR_RIE			(1<<6)	/* 受信割り込みイネーブル */
#define	SCI_SCR_TIE			(1<<7)	/* 送信割り込みイネーブル */

/* BRR(Bit Rate Register)のビット定義 */
#ifdef _20MHZ_CPU_
#define	SCI_BRR_DEFAULT		(64)	/*  9600 bps */
#define	SCI_BRR_19200		(32)	/* 19200 bps */
#define	SCI_BRR_38400		(15)	/* 38400 bps */
#else	/* _20MHZ_CPU_ */
#define	SCI_BRR_DEFAULT		(80)	/*  9600 bps */
#define	SCI_BRR_19200		(40)	/* 19200 bps */
#define	SCI_BRR_38400		(19)	/* 38400 bps */
#endif	/* _20MHZ_CPU_ */

/* SSR(Serial Status Register)のビット定義 */
#define	SCI_SSR_MPBT		(1<<0)	/* TODO */
#define	SCI_SSR_MPB			(1<<1)	/* TODO */
#define	SCI_SSR_TEND		(1<<2)	/* TODO */
#define	SCI_SSR_PER			(1<<3)	/* TODO */
#define	SCI_SSR_FERERS		(1<<4)	/* TODO */
#define	SCI_SSR_ORER		(1<<5)	/* TODO */
#define	SCI_SSR_RDRF		(1<<6)	/* 受信完了ビット */
#define	SCI_SSR_TDRE		(1<<7)	/* 送信完了ビット */



/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
/* H8 3069FのSCIレジスタアクセス用構造体 */
typedef	struct	_H8_SCI	{
	volatile	uint08	ucSmr;	/* SMR(シリアルモードレジスタ) */
	volatile	uint08	ucBrr;
	volatile	uint08	ucScr;
	volatile	uint08	ucTdr;
	volatile	uint08	ucSsr;
	volatile	uint08	ucRdr;
	volatile	uint08	ucScmr;
} H8_SCI;

/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/
volatile	H8_SCI*	s_pSciRegs[SCI_NUM] = {
	SCI_0,
	SCI_1,
	SCI_2
};

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/

/*****************************************************************************
	DISCRIPTION	: SCI初期化
	ARGUMENT	: iIdx	= SCIのインデックス
	RETURN		: 0		= 正常終了
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
int srlInit(int iIdx)
{
	volatile	H8_SCI*	pSci = s_pSciRegs[iIdx];
	pSci->ucScr = 0;
	pSci->ucSmr = 0;
	pSci->ucBrr = SCI_BRR_DEFAULT;
	pSci->ucScr = (SCI_SCR_RE | SCI_SCR_TE);
	pSci->ucSsr = 0;

	return 0;
}

/*****************************************************************************
	DISCRIPTION	: 送信可否判定
	ARGUMENT	: iIdx	= SCIのインデックス
	RETURN		: TRUE	= 送信可能
				  FALSE	= 送信不可
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
KOS_BOOL srlIsSendEnable(int iIdx)
{
	volatile	H8_SCI*	pSci = s_pSciRegs[iIdx];
	KOS_BOOL	bRet = TRUE;

	if((pSci->ucSsr & SCI_SSR_TDRE) == 0){
		bRet = FALSE;
	}
	
	return bRet;
}

/*****************************************************************************
	DISCRIPTION	: 受信可否判定
	ARGUMENT	: iIdx	= SCIのインデックス
	RETURN		: TRUE	= 受信可能
				  FALSE	= 受信不可
	NOTE		: -
	UPDATED		: 2013-10-16
*****************************************************************************/
KOS_BOOL srlIsRecvEnable(int iIdx)
{
	volatile	H8_SCI*	pSci = s_pSciRegs[iIdx];
	KOS_BOOL	bRet = TRUE;

	if((pSci->ucSsr & SCI_SSR_RDRF) == 0){
		bRet = FALSE;
	}
	
	return bRet;
}

/*****************************************************************************
	DISCRIPTION	: キャラクタ送信
	ARGUMENT	: iIdx		= SCIのインデックス
				  ucChar	= 送信文字
	RETURN		: 0			= 正常終了
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
int srlSendByte(int iIdx, uint08 ucChar)
{
	volatile	H8_SCI*	pSci = s_pSciRegs[iIdx];

	while(srlIsSendEnable(iIdx) == FALSE){
		/* 何もせずに待機 */;
	}

	pSci->ucTdr = ucChar;
	pSci->ucSsr &= ~SCI_SSR_TDRE;
	
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: キャラクタ受信
	ARGUMENT	: iIdx	= SCIのインデックス
	RETURN		: 受信キャラクタ
	NOTE		: -
	UPDATED		: 2013-10-16
*****************************************************************************/
uint08 srlRecvByte(int iIdx)
{
	volatile	H8_SCI*	pSci = s_pSciRegs[iIdx];
	uint08		ucChar;

	while(srlIsRecvEnable(iIdx) == FALSE){
		/* 何もせずに待機 */;
	}

	ucChar = pSci->ucRdr;
	pSci->ucSsr &= ~SCI_SSR_RDRF;
	
	return ucChar;
}

/*****************************************************************************
	DISCRIPTION	: 送信割り込み可否チェック
	ARGUMENT	: iIdx	= SCIのインデックス
	RETURN		: TRUE	= 送信可能
				  FALSE	= 送信不可
	NOTE		: -
	UPDATED		: 2014-06-03
*****************************************************************************/
KOS_BOOL srlIntrIsSendEnable(int iIdx)
{
	volatile H8_SCI* pSci = s_pSciRegs[iIdx];
	KOS_BOOL	bRet = TRUE;

	if((pSci->ucScr & SCI_SCR_TIE) == 0){
		bRet = FALSE;
	}
	
	return bRet;
}

/*****************************************************************************
	DISCRIPTION	: 送信割り込み有効化
	ARGUMENT	: iIdx	= SCIのインデックス
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-03
*****************************************************************************/
void srlIntrSetSendEnable(int iIdx)
{
	volatile H8_SCI* pSci = s_pSciRegs[iIdx];
	pSci->ucScr |= SCI_SCR_TIE;
}

/*****************************************************************************
	DISCRIPTION	: 送信割り込み無効化
	ARGUMENT	: iIdx	= SCIのインデックス
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-03
*****************************************************************************/
void srlIntrSetSendDisable(int iIdx)
{
	volatile H8_SCI* pSci = s_pSciRegs[iIdx];
	pSci->ucScr &= ~SCI_SCR_TIE;
}

/*****************************************************************************
	DISCRIPTION	: 受信割り込み可否チェック
	ARGUMENT	: iIdx	= SCIのインデックス
	RETURN		: TRUE	= 受信可能
				  FALSE	= 受信不可
	NOTE		: -
	UPDATED		: 2014-06-03
*****************************************************************************/
KOS_BOOL srlIntrIsRecvEnable(int iIdx)
{
	volatile H8_SCI* pSci = s_pSciRegs[iIdx];
	KOS_BOOL	bRet = TRUE;

	if((pSci->ucScr & SCI_SCR_RIE) == 0){
		bRet = FALSE;
	}
	
	return bRet;
}

/*****************************************************************************
	DISCRIPTION	: 受信割り込み有効化
	ARGUMENT	: iIdx	= SCIのインデックス
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-03
*****************************************************************************/
void srlIntrSetRecvEnable(int iIdx)
{
	volatile H8_SCI* pSci = s_pSciRegs[iIdx];
	pSci->ucScr |= SCI_SCR_RIE;
}

/*****************************************************************************
	DISCRIPTION	: 受信割り込み無効化
	ARGUMENT	: iIdx	= SCIのインデックス
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-03
*****************************************************************************/
void		srlIntrSetRecvDisable(int iIdx)
{
	volatile H8_SCI* pSci = s_pSciRegs[iIdx];
	pSci->ucScr &= ~SCI_SCR_RIE;
}

/***** End Of File *****/


