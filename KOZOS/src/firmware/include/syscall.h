/*****************************************************************************
	オーダー	: システムコール ヘッダ
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: syscall.h
	接頭辞		: kzSyscall
	作成日時	: 2013-10-24
*****************************************************************************/
/*
	改訂履歴
*/

#ifndef _KZ_SYSCALL_H_
#define _KZ_SYSCALL_H_

/*****************************************************************************
	インクルード
*****************************************************************************/
#include	"defines.h"
#include	"interrupt.h"

/*****************************************************************************
	定義
*****************************************************************************/

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
/* システム・コール番号の定義 */
typedef	enum {
	KZ_SYSCALL_TYPE_RUN	= 0,
	KZ_SYSCALL_TYPE_EXIT,
	KZ_SYSCALL_TYPE_WAIT,
	KZ_SYSCALL_TYPE_SLEEP,
	KZ_SYSCALL_TYPE_WAKEUP,
	KZ_SYSCALL_TYPE_GETID,
	KZ_SYSCALL_TYPE_CHGPRI,
	KZ_SYSCALL_TYPE_MALLOC,
	KZ_SYSCALL_TYPE_MFREE,
	KZ_SYSCALL_TYPE_SEND,
	KZ_SYSCALL_TYPE_RECV,
	KZ_SYSCALL_TYPE_SETINTR,
} KZ_SYSCALL_TYPE_T;

typedef struct _run_thread_ {
	KZ_FUNC_T*	pFunc;		/* メイン関数 */
	char		*pcName;	/* スレッド名 */
	int			iPriority;	/* 優先度 */
	int			iStackSize;	/* スタックサイズ */
	int			argc;
	char**		argv;
	KZ_ID_T		ret;		/* kzRun()の戻り値 */
} KZ_THREAD_STATUS_RUN;

typedef struct _exit_thread_ {
	int iDummy;
} KZ_THREAD_STATUS_EXIT;

typedef struct _wait_thread_ {
	int	ret;
} KZ_THREAD_STATUS_WAIT;

typedef struct _sleep_thread_ {
	int	ret;
} KZ_THREAD_STATUS_SLEEP;

typedef struct _wakeup_thread_ {
	KZ_ID_T	id;
	int		ret;
} KZ_THREAD_STATUS_WAKEUP;

typedef struct _get_id_thread_ {
	KZ_ID_T	id;
	int		ret;
} KZ_THREAD_STATUS_GETID;

typedef struct _change_priority_thread_ {
	int	iPriority;
	int ret;
} KZ_THREAD_STATUS_CHGPRI;

typedef struct _memory_alloc_ {
	int		iSize;
	void	*ret;
} KZ_MEMORY_ALLOC;

typedef struct _memory_free_ {
	void*	pvAddr;
	int		ret;
} KZ_MEMORY_FREE;

typedef struct _massage_send_ {
	KZ_MSGBOX_ID_T	id;
	int				iSize;
	char*			pcBuf;
	int				ret;
} KZ_MSG_SEND;

typedef struct _message_recv_ {
	KZ_MSGBOX_ID_T	id;
	int*			psSize;
	char**			ppcBuf;
	KZ_ID_T			ret;
} KZ_MSG_RECV;

typedef struct _set_interrupt_ {
	SV_T			type;
	KZ_HANDLER_T*	pHandler;
	int				ret;
} KZ_SET_INTR;

typedef union _kz_sysparameter_type{
	KZ_THREAD_STATUS_RUN	run;
	KZ_THREAD_STATUS_EXIT	exit;
	KZ_THREAD_STATUS_WAIT	wait;
	KZ_THREAD_STATUS_SLEEP	sleep;
	KZ_THREAD_STATUS_WAKEUP	wakeup;
	KZ_THREAD_STATUS_GETID	getid;
	KZ_THREAD_STATUS_CHGPRI	chgpri;
	KZ_MEMORY_ALLOC			malloc;
	KZ_MEMORY_FREE			mfree;
	KZ_MSG_SEND				send;
	KZ_MSG_RECV				recv;
	KZ_SET_INTR				setintr;
} KZ_SYSPARAM_T;

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/
/* スレッドを生成 */
KZ_ID_T	kzRun(
	KZ_FUNC_T*	pFunc,
	char*		pcName,
	int			iPriority,
	int			iStackSize,
	int			argc,
	char** argv
);
void	kzExit(void);				/* スレッド終了 */
int		kzWait(void);				/* カレントスレッドの切り替え */
int		kzSleep(void);				/* スレッドのスリープ */
int		kzWakeup(KZ_ID_T id);		/* スレッドのウェイクアップ */
KZ_ID_T	kzGetId(void);				/* スレッドIDの取得 */
int		kzChgPri(int iPriority);	/* 優先度変更 */
void*	kzMalloc(int iSize);		/* 動的メモリの獲得 */
int		kzMfree(void *pvAddr);		/* 動的メモリの解放 */
/* メッセージ送信 */
int		kzSend(KZ_MSGBOX_ID_T id, int iSize, char* pcBuf);	
KZ_ID_T	kzRecv(KZ_MSGBOX_ID_T id, int* psSize, char** ppcBuf);
int		kzSetIntr(SV_T type, KZ_HANDLER_T* pHandler);

/* サービスコール */
int		srvWakeup(KZ_ID_T id);
void*	srvMalloc(int iSize);
int		srvMfree(void *pvAddr);
int		srvSend(KZ_MSGBOX_ID_T id, int iSize, char* pcBuf);

/*****************************************************************************
	外部変数 
*****************************************************************************/

#endif /* _KZ_SYSCALL_H_ */


/***** End Of File *****/


