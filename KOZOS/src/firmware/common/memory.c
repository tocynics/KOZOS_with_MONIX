/*****************************************************************************
	オーダー	: メモリ管理ライブラリ
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: memory.c
	接頭辞		: mem
	作成日時	: 2014-06-20
*****************************************************************************/
/*
	改訂履歴
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include	"defines.h"
#include	"kozos.h"
#include	"lib.h"
#include	"memory.h"

/*****************************************************************************
	定義
*****************************************************************************/
/* メモリプールの種類の数 */
#define MEMORY_AREA_NUM (sizeof(s_Pool) / sizeof(*s_Pool))

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
/*
	メモリブロック構造体(メモリ管理のヘッダ)
	(獲得された各領域は、先頭に以下の構造体を持っている)
*/
typedef struct _kzmem_block {
	struct	_kzmem_block *pNext;
	int		iSize;
} MEM_BLOCK;

/* メモリプール */
typedef struct _kzmem_pool {
	int			iSize;
	int			iNum;
	MEM_BLOCK	*pFree;
} MEM_POOL;

/*****************************************************************************
	外部変数
*****************************************************************************/
extern	char freearea;

/*****************************************************************************
	内部変数
*****************************************************************************/
static	MEM_POOL	s_Pool[] = {
	{16, 0xFF, NULL},	/* 16 * 256 = 4096 (4KB) */
	{32, 0xFF, NULL},	/* 32 * 256 = 8192 (8KB) */
	{64, 0xFF, NULL},	/* 64 * 256 = 16384(16KB) */
};	/* 28KB */

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/
static int memPoolInit(MEM_POOL *pPool);

/*****************************************************************************
	DISCRIPTION	: メモリプールの初期化(ライブラリ)
	ARGUMENT	: -
	RETURN		: 0		= 正常終了
				  0以外	= 異常終了
	NOTE		: -
	UPDATED		: 2014-06-20
*****************************************************************************/
int kzMemInit(void)
{
	int i;
	for(i = 0; i < MEMORY_AREA_NUM; i++){
		memPoolInit(&s_Pool[i]);
	}
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: 動的メモリの獲得(ライブラリ)
	ARGUMENT	: iSize	= 確保するメモリサイズ
	RETURN		: 0		= 正常終了
				  0以外	= 異常終了
	NOTE		: -
	UPDATED		: 2014-06-20
*****************************************************************************/
void *kzAlloc(int iSize)
{
	int			i;
	MEM_BLOCK	*pMem;
	MEM_POOL	*pPool;
	
	for(i = 0; i < MEMORY_AREA_NUM; i++){
		pPool = &s_Pool[i];
		/* 要求されたサイズが16,32,64で収まる */
		if(iSize <= (pPool->iSize - sizeof(MEM_BLOCK))){
			if(pPool->pFree == NULL){
				kzSysdown();
				return NULL;
			}
			
			/* 解放済みリストからメモリ領域を取得する */
			pMem = pPool->pFree;
			pPool->pFree = pPool->pFree->pNext;
			pMem->pNext = NULL;
			
			/*
				実際に利用可能な領域はメモリブロック構造体の直後の領域に
				なるので、直後のアドレスを返す。
			*/
			return (pMem + 1);
		}
	}

	/* 指定されたサイズの領域を格納できるメモリプールが無い */
	kzSysdown();
	return NULL;
}

/*****************************************************************************
	DISCRIPTION	: 動的メモリの解放(ライブラリ)
	ARGUMENT	: pvMemAddr	= 解放するメモリアドレス
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-20
*****************************************************************************/
void kzFree(void *pvMemAddr)
{
	int		i;
	MEM_BLOCK	*pMem;
	MEM_POOL	*pPool;
	
	pMem = ((MEM_BLOCK*)pvMemAddr - 1);
	if((uint32)&freearea <= (uint32)pvMemAddr){
		for(i = 0; i < MEMORY_AREA_NUM; i++){
			pPool = &s_Pool[i];
			if(pMem->iSize == pPool->iSize){
				/* 領域を解放済みリンクリストに戻す */
				pMem->pNext = pPool->pFree;
				pPool->pFree = pMem;
				return ;
			}
		}
	}
	
	kzSysdown();
}

/*****************************************************************************
	DISCRIPTION	: メモリプールの初期化
	ARGUMENT	: pPool	= メモリプールヘッダ
	RETURN		: 0		= 正常終了
				  0以外	= 異常終了
	NOTE		: -
	UPDATED		: 2014-06-20
*****************************************************************************/
static int memPoolInit(MEM_POOL *pPool)
{
	int			i;
	MEM_BLOCK	*pMem;
	MEM_BLOCK	**ppMem;
	static		char *pcArea = &freearea;
/* メモリ空き領域の先頭を格納 */
	pMem = (MEM_BLOCK*)pcArea;
	
	/* 個々の領域を全て解放済みリンクリストにつなぐ */
	/*
		引数のメモリプールヘッダのpFreeのアドレスを格納。
		初期化処理では、メモリプールの解放済みエリアを指す
		pFreeと、次のメモリ領域を指すpNextは同じ。
	*/
	ppMem = &pPool->pFree;
	for(i = 0; i < pPool->iNum; i++){
		/* ネクストポインタに次のメモリ領域のアドレスを格納 */
		*ppMem = pMem;
		/* pMemのpNextとiSizeをクリア */
		memset(pMem, 0, sizeof(*pMem));
		/* サイズ情報を格納 */
		pMem->iSize = pPool->iSize;
		/* ネクストポインタのアドレスを格納 */
		ppMem = &(pMem->pNext);
		/* メモリサイズ分ポインタをずらす */
		pMem = (MEM_BLOCK*)((char*)pMem + pPool->iSize);
		pcArea += pPool->iSize;
	}
	
	return 0;
}
/***** End Of File *****/

