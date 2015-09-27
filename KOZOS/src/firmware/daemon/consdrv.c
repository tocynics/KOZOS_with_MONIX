/*****************************************************************************
	オーダー	: コンソールドライバスレッド
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: consdrv.c
	接頭辞		: csd
	作成日時	: 2014-06-22
*****************************************************************************/
/*
	改訂履歴
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include	"defines.h"
#include	"kozos.h"
#include	"intr.h"
#include	"interrupt.h"
#include	"serial.h"
#include	"lib.h"
#include	"consdrv.h"

/*****************************************************************************
	定義
*****************************************************************************/
#define	CONS_BUF_SIZE	(24)

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
/* コンソール管理用構造体 */
typedef struct _console_register_ {
	KZ_ID_T	id;			/* コンソールを利用するスレッド */
	int		iIdx;		/* 利用するシリアルの番号 */
	char*	pcSendBuf;	/* 送信バッファ */
	char*	pcRecvBuf;	/* 受信バッファ */
	int		iSendLen;	/* 送信バッファ中のデータサイズ */
	int		iRecvLen;	/* 受信バッファ中のデータサイズ */
	uint32	ulDummy[3];
} CONS_DRV_REG;

/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/
static	CONS_DRV_REG	s_consReg[CONSDRV_DEVICE_NUM];

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/
static	void	sendChar(CONS_DRV_REG* pConsReg);
static	void	sendString(CONS_DRV_REG* pConsReg, char* pcStr, int iLen);
static	int		consdrvIntrProc(CONS_DRV_REG* pConsReg);
static	void	consdrvIntr(void);
static	int		consdrvInit(void);
static	int		consdrvCommand(
	CONS_DRV_REG*	pConsReg,
	KZ_ID_T			id,
	int				iIdx,
	int				iSize,
	char*			pcCommand
);

/*****************************************************************************
	DISCRIPTION	: コンソールドライバスレッドメイン
	ARGUMENT	: argc
				  argv
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
int consdrv_main(int argc, char** argv)
{
	int		iSize;
	int		iIdx;
	KZ_ID_T	id;
	char*	pcBuf;
	
//	puts("consdrv!\n");
	consdrvInit();
	kzSetIntr(SV_SERIAL, consdrvIntr);
	
	while(1){
		id = kzRecv(MSGBOX_ID_CONS_OUTPUT, &iSize, &pcBuf);
		iIdx = pcBuf[0] - '0';
		consdrvCommand(&s_consReg[iIdx], id, iIdx, iSize - 1, pcBuf + 1);
		kzMfree(pcBuf);
	}
	
	return 0;
}


/*****************************************************************************
	DISCRIPTION	: 文字送信
	ARGUMENT	: pConsReg	= コンソールバッファ
	RETURN		: -
	NOTE		: 割込処理とスレッドから呼ばれるが送信バッファを操作しており
				  再入不可のため、スレッドから呼び出す場合は排他のため
				  割込禁止状態で呼ぶこと。
	UPDATED		: 2014-06-22
*****************************************************************************/
static void sendChar(CONS_DRV_REG* pConsReg)
{
	int	i;
	
	srlSendByte(pConsReg->iIdx, pConsReg->pcSendBuf[0]);
	if(pConsReg->pcSendBuf[0] == '\n'){
		srlSendByte(pConsReg->iIdx, '\r');
	}
	pConsReg->iSendLen--;
	/* 先頭文字を送信したので1文字分ずらす */
	for(i = 0; i < pConsReg->iSendLen; i++){
		pConsReg->pcSendBuf[i] = pConsReg->pcSendBuf[i + 1];
	}
	return;
}

/*****************************************************************************
	DISCRIPTION	: 文字列送信
	ARGUMENT	: pConsReg	= コンソールバッファ
				  pcStr		= 送信文字列
				  iLen		= 送信する文字列の長さ
	RETURN		: -
	NOTE		: 割込処理とスレッドから呼ばれるが送信バッファを操作しており
				  再入不可のため、スレッドから呼び出す場合は排他のため
				  割込禁止状態で呼ぶこと。
	UPDATED		: 2014-06-22
*****************************************************************************/
static void sendString(CONS_DRV_REG* pConsReg, char* pcStr, int iLen)
{
	int	i;
	
	for(i = 0; i < iLen; i++){
		pConsReg->pcSendBuf[pConsReg->iSendLen] = pcStr[i];
		pConsReg->iSendLen++;
	}
	
	/*
		送信割込無効ならば、送信開始されていないので送信開始する。
		送信割込有効ならば、送信開始されており、送信割込の延長で
		送信バッファ内のデータが順次送信されるので、何もしなくて良い。
	*/
	if(pConsReg->iSendLen != 0){	/* 送信サイズが0でない */
		/* 割込無効である */
		if(srlIntrIsSendEnable(pConsReg->iIdx) == FALSE){
			srlIntrSetSendEnable(pConsReg->iIdx);
			/*
				送信割込を有効化して先頭の1文字を出力する。
				後続の文字は、送信完了後に送信割込の延長で
				送信される。
			*/
			sendChar(pConsReg);
		}
	}
	
	return;
}

/*****************************************************************************
	DISCRIPTION	: コンソール割込ハンドラの実処理部
	ARGUMENT	: pConsReg	= コンソールバッファ
	RETURN		: 0
	NOTE		: 割込ハンドラから呼ばれる割込処理であり、非同期で呼ばれるので
				  ライブラリ関数などを呼び出す場合には注意が必要。
				  基本として、以下に当てはまる関数しか呼び出してはいけない。
				  ・再入可能である。
				  ・スレッドから呼ばれることはない。
				  ・スレッドから呼ばれることがあるが、割込禁止で呼び出している。
				  また、非コンテキスト状態で呼ばれるため、システムコールを
				  利用してはいけない。
				  (サービスコールを利用すること)
	UPDATED		: 2014-06-22
*****************************************************************************/
static int consdrvIntrProc(CONS_DRV_REG* pConsReg)
{
	uint08	ucBuf;
	char*	pcBuf;
	
	/* 受信割込処理 */
	if(srlIsRecvEnable(pConsReg->iIdx) == TRUE){ /* 受信可能 */
		ucBuf = srlRecvByte(pConsReg->iIdx);
		if(ucBuf == '\r'){
			ucBuf = '\n';
		}
		sendString(pConsReg, (char*)&ucBuf, 1);
		
		if(pConsReg->id != 0){
			if(ucBuf != '\n'){ /* 改行でない */
				/* 受信バッファにバッファリングする */
				pConsReg->pcRecvBuf[pConsReg->iRecvLen] = ucBuf;
				pConsReg->iRecvLen++;
			}
			else{
				/*
					Enterが押されたらバッファの内容をコマンド処理スレッドに
					通知する。
					(割込ハンドラなので、サービスコールを利用する)
				*/
				pcBuf = srvMalloc(CONS_BUF_SIZE);
				memcpy(pcBuf, pConsReg->pcRecvBuf, pConsReg->iRecvLen);
				srvSend(MSGBOX_ID_CONS_INPUT, pConsReg->iRecvLen, pcBuf);
				pConsReg->iRecvLen = 0;
			}
		}
	}
	
	/* 送信割込処理 */
	if(srlIsSendEnable(pConsReg->iIdx) == TRUE){ /* 送信可能 */
		/* IDがある、または、送信文字数が0である */
		if((pConsReg->id == 0) || (pConsReg->iSendLen == 0)){
			/* 送信データが無いならば、送信処理終了 */
			srlIntrSetSendDisable(pConsReg->iIdx);
		}
		else{
			sendChar(pConsReg);
		}
	}
	
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: コンソール割込ハンドラ
	ARGUMENT	: -
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
static void consdrvIntr(void)
{
	int				i;
	CONS_DRV_REG*	pConsReg;
	
	for(i = 0; i < CONSDRV_DEVICE_NUM; i++){
		pConsReg = &s_consReg[i];
		if(pConsReg->id != 0){
			if((srlIsRecvEnable(pConsReg->iIdx) == TRUE)
			|| (srlIsSendEnable(pConsReg->iIdx) == TRUE)){
				/* 割込があるならば、割込処理を呼び出す */
				consdrvIntrProc(pConsReg);
			}
		}
	}

	return;
}

/*****************************************************************************
	DISCRIPTION	: コンソール初期化
	ARGUMENT	: -
	RETURN		: 0
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
static int consdrvInit(void)
{
	memset(s_consReg, 0, sizeof(s_consReg));
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: コンソールコマンド処理
	ARGUMENT	: pConsReg	= コンソールバッファ
				  id		= 呼び出したスレッドのID
				  iIdx		= 使用するシリアルナンバ
				  iSize		= 送信文字数
				  pcCommand	= コマンド文字列
	RETURN		: 0
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
static	int		consdrvCommand(
	CONS_DRV_REG*	pConsReg,
	KZ_ID_T			id,
	int				iIdx,
	int				iSize,
	char*			pcCommand
)
{
	switch(pcCommand[0]){
		case CONSDRV_CMD_USE: /* コンソールドライバの使用開始 */
			pConsReg->id		= id;
			pConsReg->iIdx		= pcCommand[1] - '0';
			pConsReg->pcSendBuf	= kzMalloc(CONS_BUF_SIZE);
			pConsReg->pcRecvBuf	= kzMalloc(CONS_BUF_SIZE);
			pConsReg->iSendLen	= 0;
			pConsReg->iRecvLen	= 0;
//			srlInit(pConsReg->iIdx);
			srlIntrSetRecvEnable(pConsReg->iIdx);
			break;
		
		case CONSDRV_CMD_WRITE: /* コンソールへの文字列出力 */
			/* T.B.D
				送信割込のハンドラでは送信処理は行わず、
				コンソールドライバスレッド(本処理)に送信完了の通知を
				メッセージで送信し、コンソールドライバスレッドで次の文字の
				送信をするようにすることで、割込禁止にする必要を
				なくすことが出来る。
				割込有効、無効の切り替えで排他するよりも、
				上記の処理のほうがスマートである。
			*/
			INTR_DISABLE();
			sendString(pConsReg, pcCommand + 1, iSize - 1);
			INTR_ENABLE();
			break;
		
		default:
			break;
	}

	return 0;
}

/***** End Of File *****/

