/*****************************************************************************
	オーダー	: コマンドスレッド
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: command.c
	接頭辞		: cmd
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
#include	"serial.h"
#include	"consdrv.h"
#include	"lib.h"

/*****************************************************************************
	定義
*****************************************************************************/

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/

/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/
static	int	(*pFuncEntry)(int argc, char** argv);

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/
static	void	sendUse(int iIdx);
static	void	sendWrite(char* pcStr);

/*****************************************************************************
	DISCRIPTION	: コンソールドライバへの使用開始依頼1
	ARGUMENT	: iIdx	= シリアルID
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
static void sendUse(int iIdx)
{
	char*	pcBuf;
	
	pcBuf	= kzMalloc(3);
	pcBuf[0]	= '0';
	pcBuf[1]	= CONSDRV_CMD_USE;
	pcBuf[2]	= '0' + iIdx;
	kzSend(MSGBOX_ID_CONS_OUTPUT, 3, pcBuf);
	
	return;
}

/*****************************************************************************
	DISCRIPTION	: コンソールドライバへの文字列出力依頼
	ARGUMENT	: pcStr	= 出力する文字列バッファ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
static void sendWrite(char* pcStr)
{
	char*	pcBuf;
	int		iLen;
	
	iLen = strlen(pcStr);
	pcBuf = kzMalloc(iLen + 2);
	pcBuf[0] = '0';
	pcBuf[1] = CONSDRV_CMD_WRITE;
	memcpy(&pcBuf[2], pcStr, iLen);
	kzSend(MSGBOX_ID_CONS_OUTPUT, iLen + 2, pcBuf);
	
	return;
}

/*****************************************************************************
	DISCRIPTION	: コマンドスレッドメイン1
	ARGUMENT	: argc
				  argv
	RETURN		: 0		= 正常終了
				  0以外	= 異常終了
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
int command_main(int argc, char** argv)
{
	char*	pcBuf;
	int		iSize;
	uint32	ulAddr = 0x00003034;
	
	sendUse(SERIAL_DEFAULT_DEV);
	
	while(1){
		sendWrite("cmd> ");
		
		/* コンソールからの受信文字列を受け取る */
		kzRecv(MSGBOX_ID_CONS_INPUT, &iSize, &pcBuf);
		pcBuf[iSize] = '\0';
		
		if(strncmp(pcBuf, "echo", 4) == TRUE){
			sendWrite(&pcBuf[4]);
			sendWrite("\n");
		}
#if 1
		else if(strncmp(pcBuf, "usb", 3) == TRUE){
			usb_main();
		}
#endif
		else if(pcBuf[0] == '\0'){
			/* 何もしない */
		}
		else if(strcmp(pcBuf, "0000") == TRUE){
			//pFuncEntry = (int (*)(int argc, char** argv))ppcOsEntryFp;
			//pFuncEntry(1, ppcOsEntryFp);

			((void (*)())ulAddr)();
		}
		else{
			sendWrite("unknown.\n");
		}
		
		kzMfree(pcBuf);
	}
	
	return 0;
}


/***** End Of File *****/

