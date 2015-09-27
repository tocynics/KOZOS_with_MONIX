/*****************************************************************************
	オーダー	: ELF解析
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: elf.c
	接頭辞		: elf
	作成日時	: 2013-10-24
*****************************************************************************/
/*
	改訂履歴
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include	"defines.h"
#include	"elf.h"
#include	"lib.h"

/*****************************************************************************
	定義
*****************************************************************************/

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
/* 16バイトのELF形式ファイルの識別情報構造体 */
/* Executable and Linkable Format */
typedef struct _ELF_ID {
	uint08	ucMagic[4];		/* マジックナンバ */
	uint08	ucClass;		/* 32/64ビットの区別 */
	uint08	ucFormat;		/* エンディアン情報 */
	uint08	ucVer;			/* ELFフォーマットのバージョン */
	uint08	ucAbi;			/* OSの区別(Application Binary Interface) */
	uint08	ucAbiVer;		/* OSのバージョン */
	uint08	ucReserve[7];	/* 予約領域 */
} ELF_ID;

/* ELFヘッダ情報構造体 */
typedef struct _ELF_HEAD {
	ELF_ID	Id;				/* ELF識別情報 */
	int16	sType;			/* ファイルの種別 */
	int16	sArch;			/* CPUの種別(Architecture) */
	int32	lVer;			/* ELF形式のバージョン */
	int32	lEntryPoint;	/* 実行開始アドレス */
	int32	lProgHeadOfs;	/* --以降はだいたいわかると思います。-- */
	int32	lSectHeadOfs;	/* --ちゃんと変数名を読んでください。-- */
	int32	lFlags;
	int16	sHeadSize;
	int16	sProgHeadSize;
	int16	sProgHeadNum;
	int16	sSectHeadSize;
	int16	sSectHeadNum;
	int16	sSectNameIdx;
} ELF_HEAD;

/* ELFプログラムヘッダ情報構造体 */
typedef struct _ELF_PROG_HEAD {
	int32	lType;			/* セグメントの種別 */
	int32	lOfs;			/* ファイル先頭からのオフセット */
	int32	lVirtAddr;		/* 論理アドレス(VA) */
	int32	lPhysAddr;		/* 物理アドレス(PA) */
	int32	lFileSize;		/* ファイル中のサイズ */
	int32	lMemSize;		/* メモリ上でのサイズ */
	int32	lFlags;			/* 各種フラグ */
	int32	lAlign;			/* アラインメント(Alignment) */
} ELF_PROG_HEAD;

/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/
static int16 elfCheck(ELF_HEAD* pElfHead);
static int16 elfLoadProgram(ELF_HEAD* pElfHead);

/*****************************************************************************
	DISCRIPTION	: ELFファイルのヘッダ情報チェック
	ARGUMENT	: pElfHead	= ELF構造体
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
static int16 elfCheck(ELF_HEAD* pElfHead)
{
	/* マジックナンバのチェック */
	/* \x7fは文字列内に16進数で値を表現する方法 */
	if(memcmp(pElfHead->Id.ucMagic, "\x7f""ELF", 4) != TRUE){
		return -1;
	}

	if(pElfHead->Id.ucClass != 1){
		return -2;
	}
	if(pElfHead->Id.ucFormat != 2){
		return -3;
	}
	if(pElfHead->Id.ucVer != 1){
		return -4;
	}
	if(pElfHead->sType != 2){
		return -5;
	}
	if(pElfHead->lVer != 1){
		return -6;
	}

	/* H8/300 or H8/300H */
	if((pElfHead->sArch != 46) &&
			(pElfHead->sArch != 47)){
		return -7;
	}

	return 0;
}

/*****************************************************************************
	DISCRIPTION	: ELFをセグメント単位でロード
	ARGUMENT	: pElfHead	= ELF構造体
	RETURN		: 0			= 正常終了
				: 0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
static int16 elfLoadProgram(ELF_HEAD* pElfHead)
{
	int16			iCnt;
	ELF_PROG_HEAD*	pElfProgHead;	/* ELFプログラムヘッダ */
	char*			pcBuf;			/* テンポラリポインタバッファ */
	char*			pcPhysAddr;		/* プログラムを展開する物理アドレス */
	int32			lOfs;			/* オフセット */
	int32			lFileSize;		/* ヘッダ領域を省いたファイルサイズ? */
	int32			lMemSize;		/* 使用するメモリサイズ */

	for(iCnt = 0; iCnt < pElfHead->sProgHeadNum; iCnt++){
		pcBuf = (char*)pElfHead;
		pcBuf += pElfHead->lProgHeadOfs;
		pcBuf += pElfHead->sProgHeadSize * iCnt;

		pElfProgHead = (ELF_PROG_HEAD*)pcBuf;

		/* ロード可能なセグメントかどうかをチェック */
		if(pElfProgHead->lType != 1){
			continue;
		}

		pcPhysAddr = (char*)pElfProgHead->lPhysAddr;	/* 物理アドレス */
		lOfs = pElfProgHead->lOfs;						/* オフセット */
		lFileSize = pElfProgHead->lFileSize;			/* ファイルサイズ */
		lMemSize = pElfProgHead->lMemSize;				/* メモリサイズ */

		/* ファイルサイズ分メモリにコピー */
		memcpy(pcPhysAddr, (char*)pElfHead + lOfs, lFileSize);

		/* コピーしたメモリ以降を0で初期化 */
		/* BSS領域の準備のため */
		memset(pcPhysAddr + lFileSize, 0, lMemSize - lFileSize);
#if 0
		DPUTS("----- Program Header -----\n");
		DPUTS("Offset           > ");
		DPHEX(pElfProgHead->lOfs, 8, 1);
		DPUTS("\n");
		DPUTS("Vitrual Address  > ");
		DPHEX(pElfProgHead->lVirtAddr, 8, 1);
		DPUTS("\n");
		DPUTS("Physical Address > ");
		DPHEX(pElfProgHead->lPhysAddr, 8, 1);
		DPUTS("\n");
		DPUTS("File Size        > ");
		DPHEX(pElfProgHead->lFileSize, 8, 1);
		DPUTS("\n");
		DPUTS("Memory Size      > ");
		DPHEX(pElfProgHead->lMemSize, 8, 1);
		DPUTS("\n");
		DPUTS("Flags            > ");
		DPHEX(pElfProgHead->lFlags, 8, 1);
		DPUTS("\n");
		DPUTS("Alignment        > ");
		DPHEX(pElfProgHead->lAlign, 8, 1);
		DPUTS("\n");
#endif
	}

	return 0;
}

/*****************************************************************************
	DISCRIPTION	: ELFロード
	ARGUMENT	: pElfHead	= ELF構造体
	RETURN		: 0			= 正常終了
				: 0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
char* elfLoad(char* pcBuf)
{
	ELF_HEAD* pElfHead = (ELF_HEAD*)pcBuf;
	int16 iRet;
	if((iRet = elfCheck(pElfHead)) < 0){
		DPUTS((uint08*)"elfCheck() error!! > ");
		DPHEX(iRet, 8, 1);
		DPUTS((uint08*)"\n");
		return NULL;
	}

	if((iRet = elfLoadProgram(pElfHead)) < 0){
		DPUTS((uint08*)"elfLoadProgram() error!!\n");
		DPHEX(iRet, 8, 1);
		DPUTS((uint08*)"\n");
		return NULL;
	}
	return (char*)pElfHead->lEntryPoint;
}


/***** End Of File *****/

