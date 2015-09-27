/*****************************************************************************
	オーダー	: XMODEMプロトコル
	CPU			: H8 3069F
	コンパイラ	: gcc version 3.4.6
	ファイル名	: xmodem.c
	接頭辞		: xm
	作成日時	: 2013-10-08
*****************************************************************************/
/*
	改訂履歴
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include	"defines.h"
#include	"serial.h"
#include	"lib.h"
#include	"xmodem.h"

/*****************************************************************************
	定義
*****************************************************************************/
#define		ASC_NUL		(0x00)	/* null文字 */
#define		ASC_SOH		(0x01)	/* ヘッダ開始 */
#define		ASC_STX		(0x02)	/* テキスト開始 */
#define		ASC_ETX		(0x03)	/* テキスト終了 */
#define		ASC_EOT		(0x04)	/* 転送終了 */
#define		ASC_ENQ		(0x05)	/* 照会 */
#define		ASC_ACK		(0x06)	/* 受信OK */
#define		ASC_BEL		(0x07)	/* 警告 */
#define		ASC_BS		(0x08)	/* 後退 */
#define		ASC_HT		(0x09)	/* 水平タブ */
#define		ASC_LF		(0x0a)	/* 改行 */
#define		ASC_VT		(0x0b)	/* 垂直タブ */
#define		ASC_FF		(0x0c)	/* 改頁 */
#define		ASC_CR		(0x0d)	/* 復帰 */
#define		ASC_SO		(0x0e)	/* シフトアウト */
#define		ASC_SI		(0x0f)	/* シフトイン */
#define		ASC_DLE		(0x10)	/* データリンクエスケープ */
#define		ASC_DC1		(0x11)	/* 装置制御1 */
#define		ASC_DC2		(0x12)	/* 装置制御2 */
#define		ASC_DC3		(0x13)	/* 装置制御3 */
#define		ASC_DC4		(0x14)	/* 装置制御4 */
#define		ASC_NAK		(0x15)	/* 受信失敗 */
#define		ASC_SYN		(0x16)	/* 同期 */
#define		ASC_ETB		(0x17)	/* 転送ブロック終了 */
#define		ASC_CAN		(0x18)	/* とりけし */
#define		ASC_EM		(0x19)	/* メディア終了 */
#define		ASC_SUB		(0x1a)	/* 置換 */
#define		ASC_ESC		(0x1b)	/* エスケープ */
#define		ASC_FS		(0x1c)	/* フォーム区切り */
#define		ASC_GS		(0x1d)	/* グループ区切り */
#define		ASC_RS		(0x1e)	/* レコード区切り */
#define		ASC_US		(0x1f)	/* ユニット区切り */
#define		ASC_DEL		(0x7f)	/* 削除 */

#define		BLOCK_SIZE	(128)

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/

/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/
static int16 xmWait(void);
static int16 xmReadBlock(uint08 ucBlockNum, char* pcBuf);

/*****************************************************************************
	DISCRIPTION	: XMODEM待機
	ARGUMENT	: なし
	RETURN		: 0 :正常終了
	NOTE		: -
	UPDATED		: 2013-10-09
*****************************************************************************/
static int16 xmWait(void)
{
	uint32 ulCount = 0;

	/* ループでウェイトする */
	while(srlIsRecvEnable(SERIAL_DEFAULT_DEV) == FALSE){
		ulCount++;

		if(ulCount >= 3000000){ /* 8秒くらいウェイトがかかるらしい */
			ulCount = 0;
			srlSendByte(SERIAL_DEFAULT_DEV, ASC_NAK);
		}
	}

	return 0;
}

/*****************************************************************************
	DISCRIPTION	: XMODEMブロック受信
	ARGUMENT	: ucBlockNum	: 指定ブロック番号
				  pcBuf		: ブロック格納バッファ
	RETURN		: KOS_ER		: 異常終了
				  KOS_ER以外	: 受信ブロックサイズ
	NOTE		: サイズ、チェックサムは、データ部のみ
	UPDATED		: 2013-10-09
*****************************************************************************/
static int16 xmReadBlock(uint08 ucBlockNum, char* pcBuf)
{
	uint08	ucChar;				/* 受信した1キャラクタ */
	uint08	ucRecvBlockNum;		/* 受信したブロック番号 */
	uint08	ucReverseBlockNum;	/* 受信したビット反転ブロック番号 */
	uint08	ucCheckSum = 0;		/* チェックサム */
	uint08	ucRecvCheckSum = 0;	/* 受信した反転チェックサム */
	int16	iCnt;

	/* 受信ブロックの先頭バイト、ブロック番号を取得する */
	ucRecvBlockNum = srlRecvByte(SERIAL_DEFAULT_DEV);
	/* 指定ブロック番号と、受信したブロック番号が異なる */
	if(ucBlockNum != ucRecvBlockNum){
		return -1;
	}

	/* 反転したブロック番号を受信する */
	ucReverseBlockNum = srlRecvByte(SERIAL_DEFAULT_DEV);
	/* 受信したブロック番号と、受信したビット反転ブロック番号の比較 */
	/* '~'をつけて、受信ブロック番号をビット反転してる */
	if((ucRecvBlockNum ^ ucReverseBlockNum) != 0xFF){
		return -2;
	}

	/* ブロックの最大サイズ分ループ */
	for(iCnt = 0; iCnt < BLOCK_SIZE; iCnt++){
		ucChar = srlRecvByte(SERIAL_DEFAULT_DEV);
		*pcBuf = ucChar;
		pcBuf++;
		ucCheckSum += ucChar;
	}

	ucRecvCheckSum = srlRecvByte(SERIAL_DEFAULT_DEV);
	/* 計算したチェックサム値と、受信したチェックサム値が異なる */
	if((ucCheckSum ^ ucRecvCheckSum) != 0){
		return -3;
	}
	else{
		return iCnt;
	}
}

/*****************************************************************************
	DISCRIPTION	: XMODEMデータ受信
	ARGUMENT	: cBuf	: 受信バッファ
	RETURN		: 
	NOTE		: -
	UPDATED		: 2013-10-09
*****************************************************************************/
int32 xmRecv(char* cBuf)
{
	int16	iRecvSize;			/* 受信したサイズ */
	int16	iReceivedSOH = 0;	/* 送信開始通知の有無 */
	uint32	ulSize = 0;			/* 受信した合計サイズ */
	uint08	ucChar;				/* 受信したキャラクタ */
	uint08	ucBlockNum = 1;		/* 指定ブロックナンバー */

	while(TRUE){
		if(iReceivedSOH == 0){
			xmWait();
		}
		
		ucChar = srlRecvByte(SERIAL_DEFAULT_DEV);

		if(ucChar == ASC_EOT){ /* 相手の送信完了 */
			srlSendByte(SERIAL_DEFAULT_DEV, ASC_ACK); /* 受信完了通知 */
			break;
		}
		else if(ucChar == ASC_CAN){ /* 相手の送信中断 */
			srlSendByte(SERIAL_DEFAULT_DEV, ASC_NAK); /* 受信失敗通知 */
			return -1;
		}
		else if(ucChar == ASC_SOH){ /* 相手の送信開始 */
			iReceivedSOH = 1;	/* 送信開始通知を受けた */
			iRecvSize = xmReadBlock(ucBlockNum, cBuf);

			if(iRecvSize < 0){ /* 受信失敗 */
				srlSendByte(SERIAL_DEFAULT_DEV, ASC_NAK); /* 受信失敗通知 */
				return (int32)iRecvSize;
			}
			else{ /* 受信成功 */
				ucBlockNum++;
				ulSize += iRecvSize;
				cBuf += iRecvSize;
				srlSendByte(SERIAL_DEFAULT_DEV, ASC_ACK); /* 受信成功通知 */
			}
		}
		else{ /* その他の値を受信 */
			if(iReceivedSOH != 0){
				return -1;
			}
		}
	}

	return ulSize;
}


/***** End Of File *****/


