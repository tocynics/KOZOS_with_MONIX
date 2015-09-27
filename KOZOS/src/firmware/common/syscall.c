/*****************************************************************************
	オーダー	: システムコール
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: syscall.c
	接頭辞		: 
	作成日時	: 2013-10-24
*****************************************************************************/
/*
	改訂履歴
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include	"defines.h"
#include	"kozos.h"
#include	"interrupt.h"
#include	"syscall.h"

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

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/

/*****************************************************************************
	DISCRIPTION	: スレッド起動(システムコール)
	ARGUMENT	: pFunc			= スレッドとして起動する関数
				  pcName		= スレッド名
				  iStackSize	= スタックサイズ
				  argc			= システムコールの引数
				  argv			= システムコールの引数
	RETURN		: 0			= 異常終了
				  0以外		= スレッドID
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
KZ_ID_T	kzRun(
	KZ_FUNC_T*	pFunc,
	char*		pcName,
	int			iPriority,
	int			iStackSize,
	int			argc,
	char**		argv
)
{
	/*
		スタックはスレッドごとに確保されるので、パラメータ域は
		自動変数としてスタック上に確保する。
	*/
	KZ_SYSPARAM_T	param;
	
	param.run.pFunc = pFunc;
	param.run.pcName = pcName;
	param.run.iPriority = iPriority;
	param.run.iStackSize = iStackSize;
	param.run.argc = argc;
	param.run.argv = argv;
	
	kzSyscall(KZ_SYSCALL_TYPE_RUN, &param);
	
	/* システムコールの応答が構造体に格納されて返るので、戻り値として返す */
	return (param.run.ret);
}

/*****************************************************************************
	DISCRIPTION	: スレッド終了(システムコール)
	ARGUMENT	: -
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
void kzExit(void)
{
	kzSyscall(KZ_SYSCALL_TYPE_EXIT, NULL);
}

/*****************************************************************************
	DISCRIPTION	: スレッドのウェイト(システムコール)
	ARGUMENT	: -
	RETURN		: threadWait()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
int kzWait(void)
{
	KZ_SYSPARAM_T	param;
	
	kzSyscall(KZ_SYSCALL_TYPE_WAIT, &param);
	return (param.wait.ret);
}

/*****************************************************************************
	DISCRIPTION	: スレッドのスリープ(システムコール)
	ARGUMENT	: -
	RETURN		: threadSleep()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
int kzSleep(void)
{
	KZ_SYSPARAM_T	param;
	
	kzSyscall(KZ_SYSCALL_TYPE_SLEEP, &param);
	return (param.sleep.ret);
}

/*****************************************************************************
	DISCRIPTION	: 指定スレッドのウェイクアップ(システムコール)
	ARGUMENT	: id	= ウェイクアップするスレッドのID
	RETURN		: threadWakeup()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
int kzWakeup(KZ_ID_T id)
{
	KZ_SYSPARAM_T	param;
	param.wakeup.id = id;
	kzSyscall(KZ_SYSCALL_TYPE_WAKEUP, &param);
	return (param.wakeup.ret);
}

/*****************************************************************************
	DISCRIPTION	: スレッドIDの取得(システムコール)
	ARGUMENT	: -
	RETURN		: threadWait()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
KZ_ID_T kzGetId(void)
{
	KZ_SYSPARAM_T	param;
	
	kzSyscall(KZ_SYSCALL_TYPE_GETID, &param);
	return (param.getid.ret);
}

/*****************************************************************************
	DISCRIPTION	: スレッドIDの取得(システムコール)
	ARGUMENT	: -
	RETURN		: threadWait()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
int kzChgPri(int iPriority)
{
	KZ_SYSPARAM_T	param;
	
	param.chgpri.iPriority = iPriority;
	kzSyscall(KZ_SYSCALL_TYPE_CHGPRI, &param);
	return (param.chgpri.ret);
}

/*****************************************************************************
	DISCRIPTION	: 動的メモリの獲得(システムコール)
	ARGUMENT	: iSize	= 獲得するメモリサイズ
	RETURN		: 確保したメモリの先頭アドレス
	NOTE		: kzAlloc()の戻り値
	UPDATED		: 2014-06-20
*****************************************************************************/
void* kzMalloc(int iSize)
{
	KZ_SYSPARAM_T	param;
	param.malloc.iSize = iSize;
	kzSyscall(KZ_SYSCALL_TYPE_MALLOC, &param);
	return (param.malloc.ret);
}

/*****************************************************************************
	DISCRIPTION	: 動的メモリの解放(システムコール)
	ARGUMENT	: pvAddr	= 解放するメモリアドレスの先頭
	RETURN		: kzFree()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-20
*****************************************************************************/
int kzMfree(void *pvAddr)
{
	KZ_SYSPARAM_T	param;
	param.mfree.pvAddr = pvAddr;
	kzSyscall(KZ_SYSCALL_TYPE_MFREE, &param);
	
	return (param.mfree.ret);
}

/*****************************************************************************
	DISCRIPTION	: メッセージ送信(システムコール)
	ARGUMENT	: id	= メッセージボックスのID
				  iSize	= バッファサイズ
				  pcBuf	= バッファの先頭ポインタ
	RETURN		: sendMsg()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-21
*****************************************************************************/
int kzSend(KZ_MSGBOX_ID_T id, int iSize, char* pcBuf)
{
	KZ_SYSPARAM_T	param;
	param.send.id = id;
	param.send.iSize = iSize;
	param.send.pcBuf = pcBuf;
	kzSyscall(KZ_SYSCALL_TYPE_SEND, &param);
	
	return (param.send.ret);
}

/*****************************************************************************
	DISCRIPTION	: メッセージ受信(システムコール)
	ARGUMENT	: id		= メッセージボックスのID
				  psSize	= バッファサイズのポインタ
				  ppcBuf	= バッファの先頭のアドレス
	RETURN		: recvMsg()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-21
*****************************************************************************/
KZ_ID_T kzRecv(KZ_MSGBOX_ID_T id, int* psSize, char** ppcBuf)
{
	KZ_SYSPARAM_T	param;
	param.recv.id = id;
	param.recv.psSize = psSize;
	param.recv.ppcBuf = ppcBuf;
	kzSyscall(KZ_SYSCALL_TYPE_RECV, &param);
	
	return (param.recv.ret);
}

/*****************************************************************************
	DISCRIPTION	: 割込みハンドラ設定(システムコール)
	ARGUMENT	: type		= 割込種別
				  pHandler	= 割込ハンドラ
	RETURN		: threadSetIntr()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
int kzSetIntr(SV_T type, KZ_HANDLER_T* pHandler)
{
	KZ_SYSPARAM_T	param;
	param.setintr.type = type;
	param.setintr.pHandler = pHandler;
	kzSyscall(KZ_SYSCALL_TYPE_SETINTR, &param);
	
	return (param.setintr.ret);
}

/* 以下はサービスコール */
/*****************************************************************************
	DISCRIPTION	: 指定スレッドのウェイクアップ(サービスコール)
	ARGUMENT	: id	= ウェイクアップするスレッドのID
	RETURN		: threadWakeup()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
int srvWakeup(KZ_ID_T id)
{
	KZ_SYSPARAM_T	param;
	param.wakeup.id = id;
	kzSrvcall(KZ_SYSCALL_TYPE_WAKEUP, &param);
	return (param.wakeup.ret);
}

/*****************************************************************************
	DISCRIPTION	: 動的メモリの獲得(サービスコール)
	ARGUMENT	: iSize	= 獲得するメモリサイズ
	RETURN		: 確保したメモリの先頭アドレス
	NOTE		: kzAlloc()の戻り値
	UPDATED		: 2014-06-20
*****************************************************************************/
void* srvMalloc(int iSize)
{
	KZ_SYSPARAM_T	param;
	param.malloc.iSize = iSize;
	kzSrvcall(KZ_SYSCALL_TYPE_MALLOC, &param);
	return (param.malloc.ret);
}

/*****************************************************************************
	DISCRIPTION	: 動的メモリの解放(サービスコール)
	ARGUMENT	: pvAddr	= 解放するメモリアドレスの先頭
	RETURN		: kzFree()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-20
*****************************************************************************/
int srvMfree(void *pvAddr)
{
	KZ_SYSPARAM_T	param;
	param.mfree.pvAddr = pvAddr;
	kzSrvcall(KZ_SYSCALL_TYPE_MFREE, &param);
	
	return (param.mfree.ret);
}

/*****************************************************************************
	DISCRIPTION	: メッセージ送信(サービスコール)
	ARGUMENT	: id	= メッセージボックスのID
				  iSize	= バッファサイズ
				  pcBuf	= バッファの先頭ポインタ
	RETURN		: sendMsg()の戻り値
	NOTE		: -
	UPDATED		: 2014-06-21
*****************************************************************************/
int srvSend(KZ_MSGBOX_ID_T id, int iSize, char* pcBuf)
{
	KZ_SYSPARAM_T	param;
	param.send.id = id;
	param.send.iSize = iSize;
	param.send.pcBuf = pcBuf;
	kzSrvcall(KZ_SYSCALL_TYPE_SEND, &param);
	
	return (param.send.ret);
}

/***** End Of File *****/

