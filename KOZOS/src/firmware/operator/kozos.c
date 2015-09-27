/*****************************************************************************
	オーダー	: KOZOS本体
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: kozos.c
	接頭辞		: kz
	作成日時	: 2014-06-04
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
#include	"syscall.h"
#include	"memory.h"
#include	"lib.h"

/*****************************************************************************
	定義
*****************************************************************************/
/* TCB(タスク・コントロール・ブロックの個数 */
#define	THREAD_NUM			(6)
/* 優先度数 */
#define	PRIORITY_NUM		(16)
/* スレッド名の最大長 */
#define	THREAD_NAME_SIZE	(15)

/* スレッドフラグ */
#define	KZ_THREAD_FLAG_READY	(1 << 0)	/* レディフラグ */

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
/* スレッドのコンテキスト保存用の構造体 */
/* TCBのKZ_THREAD構造体で利用 */
typedef struct _kz_context {
	uint32	ulSp;	/* スタックポインタ */
} KZ_CONTEXT;

/* スレッドのスタートアップ(thread_init())に渡すパラメータ */
/* TCBのKZ_THREAD構造体で利用 */
typedef struct _kz_thread_init {
	KZ_FUNC_T*	pFunc;	/* スレッドのメイン関数 */
	int			argc;
	char**		argv;
} KZ_THREAD_INIT;

/* システムコール用バッファ */
/* TCBのKZ_THREAD構造体で利用 */
typedef	struct _kz_syscall {
	KZ_SYSCALL_TYPE_T	type;
	KZ_SYSPARAM_T	*pParam;
} KZ_SYSCALL_T;

/* タスク・コントロール・ブロック(TCB) */
typedef struct _kz_thread {
	/* レディー・キューへの接続に利用するpNextポインタ リンクリスト */
	struct _kz_thread	*pNext;
	char	cName[THREAD_NAME_SIZE + 1];	/* スレッド名 */
	int		iPriority;			/* 優先度 */
	char	*pStack;			/* スタックのアドレス */
	uint32	ulFlags;			/* 各種フラグ */
	KZ_THREAD_INIT	init;		/* スレッドの初期化関数 */
	KZ_SYSCALL_T	syscall;	/* システムコールの種別 */
	KZ_CONTEXT		context;	/* コンテキスト */
	char 	dummy[8];			/* パディングがうざすぎる */
} KZ_THREAD;

/* メッセージのパラメータ */
typedef struct _kz_msg_param {
	int	iSize;
	char*	pcBuf;
} KZ_MSG_PARAM;

/* メッセージバッファ */
typedef struct _kz_msg_buf {
	struct _kz_msg_buf	*pNext;
	KZ_THREAD			*pSender;
	KZ_MSG_PARAM		param;
} KZ_MSG_BUF;

/* メッセージボックス */
typedef struct _kz_msg_box {
	KZ_THREAD*	pReceiver;
	KZ_MSG_BUF*	pHead;
	KZ_MSG_BUF*	pTail;
	
	/*
		H8は16ビットCPUなので、32ビット整数に対しての乗算命令が無い。
		したがって、構造体のサイズが2の累乗になっていないと、構造体の
		配列のインデックス計算で乗算が使用されて、「__mulsi3が無い」
		などのリンクエラーが発生する場合がある。
		対策として、サイズが2の累乗になるようにダミーメンバで調整する。
		他構造体で同様のエラーが発生した場合は同様に対処すること。
	*/
	uint32	ulDummy[1];
} KZ_MSG_BOX;
	

/* レディー・キュー */
typedef struct _ready_queue{
	KZ_THREAD	*pHead;
	KZ_THREAD	*pTail;
} KZ_READY_Q;

/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/
static KZ_READY_Q		s_readyQ[PRIORITY_NUM];	/* スレッドのレディキュー */
static KZ_THREAD		*s_pCurrentThread;		/* カレントスレッド */
/* タスク・コントロール・ブロック(TCB) */
static KZ_THREAD		s_threads[THREAD_NUM];
static KZ_HANDLER_T*	s_ppHandlers[SV_TYPE_NUM];	/* 割り込みハンドラ */
static KZ_MSG_BOX		s_msgBoxes[MSGBOX_ID_NUM];	/* メッセージボックス */

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/
extern	void	dispatch(KZ_CONTEXT *context);	/* 実体はstartup.sにある */
static	int		getCurrentThread(void);
static	int		putCurrentThread(void);
static	void	threadEnd(void);
static	void	threadInit(KZ_THREAD* pThread);
static	KZ_ID_T	threadRun(
	KZ_FUNC_T*	pFunc,
	char*		pcName,
	int			iPriority,
	int			iStackSize,
	int			argc,
	char**		argv
);
static	int		threadExit(void);
static	int		threadWait(void);
static	int		threadSleep(void);
static	int		threadWakeup(KZ_ID_T id);
static	KZ_ID_T	threadGetId(void);
static	int		threadChgPri(int iPriority);
static	void	threadIntr(SV_T type, uint32 ulSp);
static	int		threadSetIntr(SV_T type, KZ_HANDLER_T* pHandler);
static	void*	threadMalloc(int iSize);
static	int		threadMfree(void* pvAddr);
static	int		threadSendMsg(KZ_MSGBOX_ID_T id, int iSize, char* pcBuf);
static	KZ_ID_T	threadRecvMsg(KZ_MSGBOX_ID_T id, int* psSize, char** ppcBuf);

static	void	sendMsg(
	KZ_MSG_BOX *pMsgBox,
	KZ_THREAD *pThread,
	int iSize,
	char *pcBuf
);
static	void	recvMsg(KZ_MSG_BOX *pMsgBox);

static	void	callFunctions(KZ_SYSCALL_TYPE_T type, KZ_SYSPARAM_T *pParam);
static	void	syscallProc(KZ_SYSCALL_TYPE_T type, KZ_SYSPARAM_T *pParam);
static	void	srvcallProc(KZ_SYSCALL_TYPE_T type, KZ_SYSPARAM_T *pParam);

static	void	schedule(void);
static	void	syscallIntr(void);
static	void	softerrIntr(void);

/*****************************************************************************
	テーブル
*****************************************************************************/

/*****************************************************************************
	DISCRIPTION	: 初期スレッドの起動
	ARGUMENT	: pFunc			= 初期スレッドの関数
				  pcName		= スレッド名
				  iPriority		= 優先度
				  iStackSize	= スタックサイズ
				  argc			= 初期スレッドの引数
				  argv			= 初期スレッドの引数
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
void kzStart(
	KZ_FUNC_T*	pFunc,
	char*		pcName,
	int			iPriority,
	int			iStackSize,
	int			argc,
	char**		argv
)
{
	/* 動的メモリの初期化 */
	kzMemInit();
	/*
		以降で呼び出すスレッド関連のライブラリ関数の内部で
		s_sCurrentThreadを見ている場合があるので、
		s_sCurrentThreadをNULLで初期化しておく。
	*/
	s_pCurrentThread = NULL;
	
	/* 各設定値を初期化 */
	memset(s_readyQ, 0, sizeof(s_readyQ));
	memset(s_threads, 0, sizeof(s_threads));
	memset(s_ppHandlers, 0, sizeof(s_ppHandlers));
	memset(s_msgBoxes, 0, sizeof(s_msgBoxes));
	
	/* 割込ハンドラの登録 */
	threadSetIntr(SV_SYSCALL, syscallIntr);
	threadSetIntr(SV_SOFTERR, softerrIntr);
	
	/* システムコール発行不可なので直接関数を呼び出してスレッドを作成する */
	s_pCurrentThread = (KZ_THREAD*)threadRun(
		pFunc, pcName, iPriority, iStackSize, argc, argv);
	
	/* 最初のスレッドを起動 */
	dispatch(&s_pCurrentThread->context);
	/* ここには返ってこない */
}

/*****************************************************************************
	DISCRIPTION	: 致命的エラー発生時のシステムダウン
	ARGUMENT	: -
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
void kzSysdown(void)
{
	puts("system error!\n");
	while(1){
		/* 無限ループで停止 */;
	}
}

/*****************************************************************************
	DISCRIPTION	: システムコール呼び出し用ライブラリ関数
	ARGUMENT	: type		= システムコールのタイプ
				  pParam	= システムコールに渡すパラメータ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
void kzSyscall(KZ_SYSCALL_TYPE_T type, KZ_SYSPARAM_T *pParam)
{
	s_pCurrentThread->syscall.type = type;
	s_pCurrentThread->syscall.pParam = pParam;
	asm volatile ("trapa #0");	/* トラップ割込発行 */
}

/*****************************************************************************
	DISCRIPTION	: サービスコール呼び出し用ライブラリ関数
	ARGUMENT	: type		= システムコールのタイプ
				  pParam	= システムコールに渡すパラメータ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
void kzSrvcall(KZ_SYSCALL_TYPE_T type, KZ_SYSPARAM_T *pParam)
{
	/* サービスコールは割込ではなく、普通の関数呼び出し */
	srvcallProc(type, pParam);
}

/*****************************************************************************
	DISCRIPTION	: カレントスレッドをレディーキューから抜き出す
	ARGUMENT	: -
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2014-06-04
*****************************************************************************/
static int getCurrentThread(void)
{
	/* カレントスレッドが空である */
	if(s_pCurrentThread == NULL){
		return -1;
	}
	
	/* 既にレディ状態でないならば何もしない */
	if((s_pCurrentThread->ulFlags & KZ_THREAD_FLAG_READY) == 0){
		/* 既にない場合は無視 */
		return 1;
	}
	
	/* カレントスレッドのネクストポインタをレディキューのヘッドに格納 */
	s_readyQ[s_pCurrentThread->iPriority].pHead = s_pCurrentThread->pNext;
	
	/* レディキューのヘッドがNULLである */
	if(s_readyQ[s_pCurrentThread->iPriority].pHead == NULL){
		/* レディキューのテイルにもNULLを格納 */
		s_readyQ[s_pCurrentThread->iPriority].pTail = NULL;
	}
	
	/* カレントスレッドのネクストポインタにNULLを格納 */
	s_pCurrentThread->ulFlags &= ~KZ_THREAD_FLAG_READY;
	s_pCurrentThread->pNext = NULL;
	
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: カレントスレッドをレディーキューにつなげる
	ARGUMENT	: -
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2014-06-04
*****************************************************************************/
static int putCurrentThread(void)
{
	/* カレントスレッドが空である */
	if(s_pCurrentThread == NULL){
		return -1;
	}
	
	/* 既にレディ状態ならば何もしない */
	if((s_pCurrentThread->ulFlags & KZ_THREAD_FLAG_READY) != 0){
		/* 既にある場合は無視 */
		return 1;
	}
	
	/* レディキューのテイルがNULLでない */
	if(s_readyQ[s_pCurrentThread->iPriority].pTail != NULL){
		/* レディキューのテイルのネクストポインタにカレントスレッドを格納 */
		s_readyQ[s_pCurrentThread->iPriority].pTail->pNext = s_pCurrentThread;
	}
	else{
		/* レディキューのヘッドにカレントスレッドを格納 */
		s_readyQ[s_pCurrentThread->iPriority].pHead = s_pCurrentThread;
	}
	
	/* レディキューのテイルにカレントスレッドを格納 */
	s_readyQ[s_pCurrentThread->iPriority].pTail = s_pCurrentThread;
	s_pCurrentThread->ulFlags |= KZ_THREAD_FLAG_READY;
	
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: スレッドの終了
	ARGUMENT	: -
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-04
*****************************************************************************/
static void threadEnd(void)
{
	kzExit();
}

/*****************************************************************************
	DISCRIPTION	: スレッドのスタートアップ
	ARGUMENT	: pThread	= 準備するスレッドのTCB
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-04
*****************************************************************************/
static void threadInit(KZ_THREAD* pThread)
{
	/* 初期化関数にargcとargvを渡す */
	pThread->init.pFunc(pThread->init.argc, pThread->init.argv);
	threadEnd();
}

/*****************************************************************************
	DISCRIPTION	: スレッドの起動
	ARGUMENT	: pFunc			= スレッドとして起動する関数
				  pcName		= スレッド名
				  iPriority		= 優先度
				  iStackSize	= スタックサイズ
				  argc			= 起動するスレッドに渡す引数
				  argv			= 起動するスレッドに渡す引数
	RETURN		: -1		= 異常終了
				  -1以外	= スレッドID
	NOTE		: -
	UPDATED		: 2014-06-04
*****************************************************************************/
static KZ_ID_T threadRun(
	KZ_FUNC_T*	pFunc,
	char*		pcName,
	int			iPriority,
	int			iStackSize,
	int			argc,
	char**		argv
)
{
	int			iIdx;		/* 空TCB検索用 */
	KZ_THREAD*	pThread;	/* TCBバッファ */
	uint32		*pulSp;		/* スタックポインタ */
	extern char	userstack;	/* linker.ldのユーザスタック */
	static char	*pThreadStack = &userstack;	/* ユーザスレッドのスタック */
	
	/* スレッド名長のチェック */
	if(THREAD_NAME_SIZE < strlen(pcName)){
		return -1;
	}
	
	/* 空きTCBのを検索 */
	for(iIdx = 0; iIdx < THREAD_NUM; iIdx++){
		pThread = &s_threads[iIdx];
		/* 初期化関数がNULLである */
		/* これでこのTCBが空きであると判断している */
		if(pThread->init.pFunc == NULL){
			break;
		}
	}
	if(iIdx == THREAD_NUM){
		return -1;
	}
	
	memset(pThread, 0, sizeof(*pThread));
	
	/* TCBの設定 */
	strcpy(pThread->cName, pcName);
	pThread->pNext = NULL;
	pThread->iPriority = iPriority;
	pThread->ulFlags = 0;
	pThread->init.pFunc = pFunc;
	pThread->init.argc = argc;
	pThread->init.argv = argv;
	
	/* スタック領域を0クリア */
	memset(pThreadStack, 0, iStackSize);
	/* スタック確保(サイズ分ポインタ移動) */
	pThreadStack += iStackSize;
	/* スタックのアドレスを格納 */
	pThread->pStack = pThreadStack;
	
	/* スタックの初期化 */
	pulSp = (uint32*)pThread->pStack;
	/*
	   threadInit()からの戻り先として
	   threadEnd()を格納する
	*/
	*(--pulSp) = (uint32)threadEnd;
	
	/*
		プログラムカウンタを設定
		ディスパッチ時にプログラムカウンタに格納される値として
		threadInit()を設定する。
		よってスレッドはthreadInit()から動作を開始する。
		スレッドの優先度がゼロの場合には、割込禁止スレッドととする。
	*/
	*(--pulSp) = ((uint32)threadInit | ((uint32)(iPriority != 0 ? 0 : 0xC0) << 24));
	
	*(--pulSp) = 0;	/* ER6 */
	*(--pulSp) = 0;	/* ER5 */
	*(--pulSp) = 0;	/* ER4 */
	*(--pulSp) = 0;	/* ER3 */
	*(--pulSp) = 0;	/* ER2 */
	*(--pulSp) = 0;	/* ER1 */
	
	/* スレッドのスタートアップ(threadInit())に渡す引数 */
	*(--pulSp) = (uint32)pThread;	/* ER0 */
	
	/* スレッドのコンテキストを設定 */
	pThread->context.ulSp = (uint32)pulSp;
	
	/* システムコールを呼び出したスレッドをレディーキューに戻す */
	putCurrentThread();
	
	/* 新規作成したスレッドをレディーキューに接続する */
	s_pCurrentThread = pThread;
	putCurrentThread();
	
	/* 新規スレッドのアドレスをスレッドIDとして返す */
	return (KZ_ID_T)s_pCurrentThread;
}

/*****************************************************************************
	DISCRIPTION	: スレッドの終了
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-05
*****************************************************************************/
static int threadExit(void)
{
	/*
		本来ならスタックも解放して再利用できるようにすべきだが省略
		このため、スレッドを頻繁に生成、消去するようなことは
		現状ではできない
	*/
	puts(s_pCurrentThread->cName);
	puts(" EXIT.....\n");
	
	/* TCBのクリア */
	memset(s_pCurrentThread, 0, sizeof(*s_pCurrentThread));
	
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: スレッドのウェイト
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static int threadWait(void)
{
	/*
		カレントスレッドをレディキューの末尾に接続し直すことで、
		他のスレッドを動作させる。
	*/
	putCurrentThread();
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: スレッドのスリープ
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static int threadSleep(void)
{
	/* レディキューから外され、スケジューリングされなくなる */
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: スレッドのウェイクアップ
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static int threadWakeup(KZ_ID_T id)
{
	/* ウェイクアップを呼び出したスレッドをレディキューに戻す */
	putCurrentThread();
	
	/* 指定されたスレッドをレディキューに接続してウェイクアップする */
	s_pCurrentThread = (KZ_THREAD*)id;
	putCurrentThread();
	
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: スレッドIDを取得
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static KZ_ID_T threadGetId(void)
{
	/* ウェイクアップを呼び出したスレッドをレディキューに戻す */
	putCurrentThread();

	return (KZ_ID_T)s_pCurrentThread;
}

/*****************************************************************************
	DISCRIPTION	: スレッドの優先度変更
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static int threadChgPri(int iPriority)
{
	int iOld = s_pCurrentThread->iPriority;
	if(iPriority >= 0){
		s_pCurrentThread->iPriority = iPriority;
	}
	
	putCurrentThread();
	
	return iOld;
}

/*****************************************************************************
	DISCRIPTION	: 動的メモリの確保
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-20
*****************************************************************************/
static void* threadMalloc(int iSize)
{
	putCurrentThread();
	return kzAlloc(iSize);
}

/*****************************************************************************
	DISCRIPTION	: 動的メモリの解放
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-20
*****************************************************************************/
static int threadMfree(void* pvAddr)
{
	kzFree(pvAddr);
	putCurrentThread();
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: メッセージ送信
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-21
*****************************************************************************/
static void sendMsg(
	KZ_MSG_BOX *pMsgBox,
	KZ_THREAD *pThread,
	int iSize,
	char *pcBuf
)
{
	KZ_MSG_BUF*	pMsgBuf;
	
	pMsgBuf = (KZ_MSG_BUF*)kzAlloc(sizeof(*pMsgBuf));
	if(pMsgBuf == NULL){
		kzSysdown();
	}
	pMsgBuf->pNext			= NULL;
	pMsgBuf->pSender		= pThread;
	pMsgBuf->param.iSize	= iSize;
	pMsgBuf->param.pcBuf	= pcBuf;
	
	/* メッセージボックスの末尾にメッセージを接続する */
	if(pMsgBox->pTail != NULL){
		pMsgBox->pTail->pNext = pMsgBuf;
	}
	else{
		pMsgBox->pHead = pMsgBuf;
	}
	
	pMsgBox->pTail = pMsgBuf;
	return;
}

/*****************************************************************************
	DISCRIPTION	: メッセージ受信
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
static void recvMsg(KZ_MSG_BOX *pMsgBox)
{
	KZ_MSG_BUF*		pMsgBuf;
	KZ_SYSPARAM_T*	pParam;
	
	/* メッセージボックスの先頭にあるメッセージを抜き出す */
	pMsgBuf = pMsgBox->pHead;
	pMsgBox->pHead = pMsgBuf->pNext;
	if(pMsgBox->pHead == NULL){
		pMsgBox->pTail = NULL;
	}
	pMsgBuf->pNext = NULL;
	
	pParam = pMsgBox->pReceiver->syscall.pParam;
	pParam->recv.ret = (KZ_ID_T)pMsgBuf->pSender;
	if(pParam->recv.psSize != NULL){
		*(pParam->recv.psSize) = pMsgBuf->param.iSize;
	}
	
	if(pParam->recv.ppcBuf != NULL){
		*(pParam->recv.ppcBuf) = pMsgBuf->param.pcBuf;
	}
	
	pMsgBox->pReceiver = NULL;
	
	kzFree(pMsgBuf);
	
	return;
}

/*****************************************************************************
	DISCRIPTION	: メッセージ送信スレッド
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
static int threadSendMsg(KZ_MSGBOX_ID_T id, int iSize, char* pcBuf)
{
	KZ_MSG_BOX*	pMsgBox = &s_msgBoxes[id];
	
	putCurrentThread();
	/* メッセージの送信 */
	sendMsg(pMsgBox, s_pCurrentThread, iSize, pcBuf);
	
	/* 受信待ちスレッドが存在している場合には受信処理をする */
	if(pMsgBox->pReceiver != NULL){
		/* 受信待ちスレッド */
		s_pCurrentThread = pMsgBox->pReceiver;
		/* メッセージ受信 */
		recvMsg(pMsgBox);
		/* 受信により動作可能になったので、ブロック解除する */
		putCurrentThread();
	}
	
	return (iSize);
}

/*****************************************************************************
	DISCRIPTION	: メッセージ受信スレッド
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-22
*****************************************************************************/
static KZ_ID_T threadRecvMsg(KZ_MSGBOX_ID_T id, int* psSize, char** ppcBuf)
{
	KZ_MSG_BOX*	pMsgBox = &s_msgBoxes[id];
	
	/* 他のスレッドが既に受信待ちしている */
	if(pMsgBox->pReceiver != NULL){
		kzSysdown();
	}
	
	/* 受信待ちスレッドに設定 */
	pMsgBox->pReceiver = s_pCurrentThread;
	
	if(pMsgBox->pHead == NULL){
		/*
			メッセージボックスにメッセージがないので、
			スレッドをスリープさせる。
			(システムコールがブロックする)
		*/
		return -1;
	}
	
	/* メッセージ受信 */
	recvMsg(pMsgBox);
	/* メッセージを受信出来たので、レディー状態にする */
	putCurrentThread();
	
	return (s_pCurrentThread->syscall.pParam->recv.ret);
}

/*****************************************************************************
	DISCRIPTION	: 割り込みハンドラの登録
	ARGUMENT	: -
	RETURN		: T.B.D.
	NOTE		: -
	UPDATED		: 2014-06-05
*****************************************************************************/
static int threadSetIntr(SV_T type, KZ_HANDLER_T* pHandler)
{
	/*
		割り込みを受け付けるために、ソフトウェア・割り込みベクタに
		OSの割り込み処理の入口となる関数を登録する。
		
		割り込み時にOSのハンドラが呼ばれるように、
		ソフトウェア・割り込みベクタを設定する。
	*/
	
	svSetHandler(type, threadIntr);
	
	/* OS側から呼び出す割り込みハンドラを登録 */
	s_ppHandlers[type] = pHandler;
	
	putCurrentThread();
	
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: システムコールの処理関数呼び出し
	ARGUMENT	: type		= システムコールの種類
				  pParam	= システムコールのパラメータ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static void callFunctions(KZ_SYSCALL_TYPE_T type, KZ_SYSPARAM_T *pParam)
{
	/*
		システムコールの実行中にkzCurrentThreadが
		書き換わるので注意する
	*/
	switch(type){
		case KZ_SYSCALL_TYPE_RUN:
			pParam->run.ret = threadRun(
								pParam->run.pFunc,
								pParam->run.pcName,
								pParam->run.iPriority,
								pParam->run.iStackSize,
								pParam->run.argc,
								pParam->run.argv);
			break;
		
		case KZ_SYSCALL_TYPE_EXIT:
			threadExit();
			break;
		
		case KZ_SYSCALL_TYPE_WAIT:
			pParam->wait.ret = threadWait();
			break;
			
		case KZ_SYSCALL_TYPE_SLEEP:
			pParam->sleep.ret = threadSleep();
			break;
			
		case KZ_SYSCALL_TYPE_WAKEUP:
			pParam->wakeup.ret = threadWakeup(pParam->wakeup.id);
			break;

		case KZ_SYSCALL_TYPE_GETID:
			pParam->getid.ret = threadGetId();
			break;

		case KZ_SYSCALL_TYPE_CHGPRI:
			pParam->chgpri.ret = threadChgPri(pParam->chgpri.iPriority);
			break;

		case KZ_SYSCALL_TYPE_MALLOC:
			pParam->malloc.ret = threadMalloc(pParam->malloc.iSize);
			break;

		case KZ_SYSCALL_TYPE_MFREE:
			pParam->mfree.ret = threadMfree(pParam->mfree.pvAddr);
			break;

		case KZ_SYSCALL_TYPE_SEND:
			pParam->send.ret = threadSendMsg(
								pParam->send.id,
								pParam->send.iSize,
								pParam->send.pcBuf);
			break;

		case KZ_SYSCALL_TYPE_RECV:
			pParam->recv.ret = threadRecvMsg(
								pParam->recv.id,
								pParam->recv.psSize,
								pParam->recv.ppcBuf);
			break;

		case KZ_SYSCALL_TYPE_SETINTR:
			pParam->setintr.ret = threadSetIntr(
								pParam->setintr.type,
								pParam->setintr.pHandler);
			break;

		default:
			break;
	}
}

/*****************************************************************************
	DISCRIPTION	: システムコールの処理
	ARGUMENT	: type		= システムコールの種類
				  pParam	= システムコールのパラメータ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static void syscallProc(KZ_SYSCALL_TYPE_T type, KZ_SYSPARAM_T *pParam)
{
	/*
		システムコールを呼び出したスレッドをレディーキューから
		外した状態で処理関数を呼び出す。
		このためシステムコールを呼び出したスレッドをそのまま
		動作継続させたい場合には、処理関数の内部で
		putCurrentThread()を行う必要がある。
	*/
	getCurrentThread();
	callFunctions(type, pParam);
}

/*****************************************************************************
	DISCRIPTION	: サービスコールの処理
	ARGUMENT	: type		= システムコールの種類
				  pParam	= システムコールのパラメータ
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static void srvcallProc(KZ_SYSCALL_TYPE_T type, KZ_SYSPARAM_T *pParam)
{
	/*
		システムコールとサービスコールの処理関数の内部で、
		システムコールの実行したスレッドIDを得るために
		s_pCurrentThreadを参照している部分があり
		(例えばthreadSendMsgなど)、
		s_pCurrentThreadが残っていると誤動作するためNULLに設定する。
		サービスコールはthreadIntrVec()内部の割込みハンドラ呼び出しの
		延長で呼ばれているはずなので、呼び出したあとにthreadIntrVec()で
		スケジューリング処理が行われ、s_pCurrentThread()は再設定される。
	*/
	s_pCurrentThread = NULL;
	callFunctions(type, pParam);
}

/*****************************************************************************
	DISCRIPTION	: スレッドのスケジューリング
	ARGUMENT	: -
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static void schedule(void)
{
	int	i;
	
	/*
		優先度の高い順(優先度の数値の小さい順)にレディキューを見て、
		動作可能なスレッドを検索する。
	*/
	for(i = 0; i < PRIORITY_NUM; i++){
		if(s_readyQ[i].pHead != NULL){
			break;
		}
	}
	
	if(i == PRIORITY_NUM){
		kzSysdown();
	}
	
	s_pCurrentThread = s_readyQ[i].pHead;
}

/*****************************************************************************
	DISCRIPTION	: システムコールの呼び出し
	ARGUMENT	: -
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static void syscallIntr(void)
{
	syscallProc(s_pCurrentThread->syscall.type,
					s_pCurrentThread->syscall.pParam);
}

/*****************************************************************************
	DISCRIPTION	: ソフトウェアエラーの発生
	ARGUMENT	: -
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static void softerrIntr(void)
{
	puts(s_pCurrentThread->cName);
	puts(" DOWN.\n");
	getCurrentThread();	/* レディーキューから外す */
	threadExit();		/* スレッドを終了する */
}

/*****************************************************************************
	DISCRIPTION	: 割り込み処理の入口関数
	ARGUMENT	: type	= 割り込みのタイプ
				  ulSp	= コンテキスト(スタックポインタ)
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
static void threadIntr(SV_T type, uint32 ulSp)
{
	/* カレントスレッドのコンテキストを保存する */
	s_pCurrentThread->context.ulSp = ulSp;
	
	/*
		割り込み毎の処理を実行する。
		SV_SOFTERR、SV_SYSCALLの場合は
		softerrIntr()、syscallIntr()がハンドラに登録されているので、
		それらが実行される。
		それ以外の場合は、kzSetIntr()によってユーザ登録された
		ハンドラが実行される。
	*/
	if(s_ppHandlers[type] != NULL){
		s_ppHandlers[type]();
	}
	
	schedule();
	
	/*
		スレッドのディスパッチ。
		dispatch()関数の本体はstartup.sにある。
	*/
	dispatch(&s_pCurrentThread->context);
	/* ここには返ってこない */
}

/***** End Of File *****/

