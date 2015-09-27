/*****************************************************************************
	オーダー	: エントリポイント
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: main.c
				: 
	作成日時	: 2013-09-03
*****************************************************************************/
/*
	改訂履歴
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include	"defines.h"
#include	"interrupt.h"
#include	"serial.h"
#include	"lib.h"
#include	"xmodem.h"
#include	"elf.h"
#include	"dram.h"

/*****************************************************************************
	定義
*****************************************************************************/

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/

/*****************************************************************************
	外部変数
*****************************************************************************/
extern int rodata_btm;	/* 読み込み専用領域末尾(linker.ldより参照) */
extern int data_top;	/* データ領域先頭(linker.ldより参照) */
extern int data_btm;	/* データ領域末尾(linker.ldより参照) */
extern int bss_top;		/* BSS領域先頭(linker.ldより参照) */
extern int bss_btm;		/* BSS領域末尾(linker.ldより参照) */

/*****************************************************************************
	内部変数
*****************************************************************************/


/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/
static int16 init(void);	/* 初期化関数 */
static int16 dump(char* pcBuf, int32 lSize);
static void wait(void);

/*****************************************************************************
	DISCRIPTION	: 初期化関数(VA != PA対応)
	ARGUMENT	: -
	RETURN		: 0		= 正常終了
				: 0以外	= 異常終了
	NOTE		: -
	UPDATED		: 2013-10-07
*****************************************************************************/
static int16 init(void)
{
	uint32 ulDataSecLen;
	uint32 ulBssSecLen;

	ulDataSecLen = (uint32)&data_btm - (uint32)&data_top;	/* データ領域のサイズ */
	ulBssSecLen = (uint32)&bss_btm - (uint32)&bss_top;		/* BSS領域のサイズ */

	memcpy(&data_top, &rodata_btm, ulDataSecLen);			/* データ領域の初期化 */
	memset(&bss_top, 0, ulBssSecLen);						/* BSS領域の初期化 */
	
	/* ソフトウェア割り込みベクタを初期化 */
	svInit();
	srlInit(SERIAL_DEFAULT_DEV);							/* シリアル初期化 */
	dramInit();

	return 0;
}

/*****************************************************************************
	DISCRIPTION	: バッファダンプ
	ARGUMENT	: pcBuf	= ダンプするバッファ
				  lSize	= ダンプするサイズ
	RETURN		: 0		= 正常終了
				  0以外	= 異常終了
	NOTE		: -
	UPDATED		: 2013-10-10
*****************************************************************************/
static int16 dump(char* pcBuf, int32 lSize)
{
	int16	iBufCnt;

	if(lSize < 0){
		puts((uint08*)(uint08*)">>>>> NO DATA <<<<<\n");
		return KOS_ER;
	}

	for(iBufCnt = 0; iBufCnt < lSize; iBufCnt++){
		printhex(pcBuf[iBufCnt], 2, 0);

		if((iBufCnt & 0x0f) == 15){
			putc('\n');
		}
		else {
			if((iBufCnt & 0x01) == 1){
				putc(' ');
			}
		}
	}
	putc('\n');
	
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: ウェイト
	ARGUMENT	: -
	RETURN		: -
	NOTE		: -
	UPDATED		: 2013-10-10
*****************************************************************************/
static void wait(void)
{
	volatile uint32 ulWait;
	for(ulWait = 0; ulWait < 300000; ulWait++){
		;	/* 何もしない */
	}
}

/*****************************************************************************
	DISCRIPTION	: エントリーポイント
	ARGUMENT	: -
	RETURN		: 0		= 正常終了
				  0以外	= 異常終了
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
int main(int argc, char** argv)
{
	static char		cBuf[16];
	static int32	lSize = -1;
	static uint08	*pucLoadBuf = NULL;
	extern int		buffer_top;	/* リンカスクリプトで定義されている */
	static char*	s_pcEntryPoint;
	int				(*pFuncEntry)(int argc, char** argv);
	
	INTR_DISABLE();

	init();

	puts((uint08*)"Kz Loader started.\n");

	while(1){
		memset(cBuf, 0, 16);
		puts((uint08*)"kzload > ");
		gets((uint08*)cBuf);
		
		/* XMODEMによるファイル受信 */
		if(strcmp(cBuf, "load") == TRUE){
			pucLoadBuf = (uint08*)&buffer_top;
			puts((uint08*)"Please send file...\n");
			lSize = xmRecv((char*)pucLoadBuf);
			wait();

			puts((uint08*)"xmRecv(): return = ");
			printhex(lSize, 8, 1);
			puts((uint08*)"\n");
		
			wait();
			if(lSize < 0){
				puts((uint08*)"\nERROR: XMODEM received error!!\n");
			}
			else{
				puts((uint08*)"\nXMODEM received succeeded.\n");
			}
		}
		/* 受信ファイルのダンプ */
		else if(strcmp(cBuf, "dump") == TRUE){
			puts((uint08*)"\nFile size: ");
			printhex(lSize, 8, 1);
			puts((uint08*)"\n");
			dump((char*)pucLoadBuf, lSize);
		}
		/* 受信したELFの実行 */
		else if(strcmp(cBuf, "run") == TRUE){
			s_pcEntryPoint = elfLoad((char*)pucLoadBuf);
			if(s_pcEntryPoint == NULL){
				puts((uint08*)"ERROR: Run error!\n");
			}
			else{
				puts((uint08*)"Starting from kozos entry point: ");
				printhex((uint32)s_pcEntryPoint, 8, 1);
				puts((uint08*)"\n");

				pFuncEntry = (int (*)(int argc, char** argv))s_pcEntryPoint;
				pFuncEntry(1, (char**)s_pcEntryPoint);
				/* ここには返ってこない */
			}
		}
		else if(cBuf[0] == '\0'){
			/* 何もしない */
		}
		else{
			puts((uint08*)"\nkzload command\n");
			puts((uint08*)">> dump: file dump\n");
			puts((uint08*)">> load: download file.\n");
			puts((uint08*)">> run : execute downloaded excutable file. \n");
		}
	}
	
	return 0;
}


/***** End Of File *****/


