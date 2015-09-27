/*****************************************************************************
	オーダー	: 共通ライブラリ関数群
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: lib.c
	接頭辞		: 無し
	作成日時	: 2013-09-03
*****************************************************************************/
/*
	作成日時
*/

/*****************************************************************************
	インクルード
*****************************************************************************/
#include	"defines.h"
#include	"serial.h"
#include	"lib.h"

/*****************************************************************************
	定義
*****************************************************************************/
#define		HEX_DISP_MAX	(8)

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
	DISCRIPTION	: メモリ初期化
	ARGUMENT	: pvBuf	= 初期化するバッファ
				  iSet	= 初期化値
				  lLen	= 初期化するバッファサイズ
	RETURN		: 0		= 正常終了
				  0以外	= 異常終了
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
void *memset(void *pvBuf, int16 iSet, uint32 lLen)
{
	uint08	*pucBuf = pvBuf;
	uint16	usCnt;

	if(lLen <= 0){
		return NULL;
	}

	for(usCnt = 0; lLen > usCnt; usCnt++){
		*pucBuf = iSet;
		pucBuf++;
	}

	return 0;
}

/*****************************************************************************
	DISCRIPTION	: メモリコピー
	ARGUMENT	: pvDst	= コピー先バッファ
				  pvSrc	= コピー元バッファ
				  lLen	= コピーするバッファサイズ
	RETURN		: 0		= 正常終了
				  0以外	= 異常終了
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
void *memcpy(void *pvDst, const void *pvSrc, uint32 lLen)
{
	uint08	*pucDst = (uint08*)pvDst;
	uint08	*pucSrc = (uint08*)pvSrc;
	uint16	usCnt;
	
	for(usCnt = 0; lLen > usCnt; usCnt++) {
		*pucDst = *pucSrc;
		pucDst++;
		pucSrc++;
	}

	return pvDst;
}

/*****************************************************************************
	DISCRIPTION	: メモリ比較
	ARGUMENT	: pvBuf1	= 比較するバッファ1
				  pvBuf2	= 比較するバッファ2
				  lLen		= 比較するバッファサイズ
	RETURN		: TRUE		= 同じ
				  FALSE		= 異なる
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
KOS_BOOL memcmp(const void *pvBuf1, const void *pvBuf2, uint32 lLen)
{
	uint08	*pucBuf1 = (uint08*)pvBuf1;
	uint08	*pucBuf2 = (uint08*)pvBuf2;
	uint16	usCnt;
	
	for(usCnt = 0; lLen > usCnt; usCnt++){
		if(*pucBuf1 != *pucBuf2){
			return FALSE;
		}
		pucBuf1++;
		pucBuf2++;
	}

	return TRUE;
}

/*****************************************************************************
	DISCRIPTION	: 文字列の長さ
	ARGUMENT	: pcStr	= 文字列
	RETURN		: 文字列長
	NOTE		: -
	UPDATED		: 2013-10-01
*****************************************************************************/
int16 strlen(const char *pcStr)
{
	int16			sLen;
	const	char*	pcTmp = pcStr;
	
	for(sLen = 0; *pcTmp != '\0'; pcTmp++) {
		sLen++;
	}

	return sLen;
}

/*****************************************************************************
	DISCRIPTION	: 文字列のコピー
	ARGUMENT	: pcDst		= コピー先文字列バッファ
				  pcStr		= コピー元文字列バッファ
	RETURN		: 文字列コピー後のコピー先文字列バッファの先頭ポインタ
	NOTE		: -
	UPDATED		: 2013-10-01
*****************************************************************************/
char *strcpy(char *pcDst, const char *pcSrc)
{
	char*			pcDstTmp = pcDst;
	const	char*	pcSrcTmp = pcSrc;
	
	while(*pcSrcTmp != '\0'){
		*pcDstTmp = *pcSrcTmp;
		pcDstTmp++;
		pcSrcTmp++;
	}
	*pcDstTmp = *pcSrcTmp;

	return pcDst;
}

/*****************************************************************************
	DISCRIPTION	: 文字列比較
	ARGUMENT	: pcStr1	= 比較文字列1
				  pcStr2	= 比較文字列2
	RETURN		: TRUE		= 同じ
				  FALSE		= 異なる
	NOTE		: -
	UPDATED		: 2013-10-01
*****************************************************************************/
KOS_BOOL strcmp(const char *pcStr1, const char *pcStr2)
{
	const	char*	pcTmp1	= pcStr1;
	const	char*	pcTmp2	= pcStr2;
	KOS_BOOL		bRet	= TRUE;
	
	while(1){
		if(*pcTmp1 != *pcTmp2){ /* 異なる */
			bRet = FALSE;
			break;
		}
		else{ /* 同じ */
			if(*pcTmp1 == '\0'){ /* 終端文字である */
				break;
			}
			else{
				pcTmp1++;
				pcTmp2++;
			}
		}
	}

	return bRet;
}

/*****************************************************************************
	DISCRIPTION	: 指定サイズ分の文字列比較
	ARGUMENT	: pcStr1	= 比較文字列1
				  pcStr2	= 比較文字列2
				  iLen		= 比較する文字数
	RETURN		: TRUE		= 同じ
				  FALSE		= 異なる
	NOTE		: -
	UPDATED		: 2013-10-01
*****************************************************************************/
KOS_BOOL strncmp(const char *pcStr1, const char *pcStr2, int16 iLen)
{
	const	char*	pcTmp1	= pcStr1;
	const	char*	pcTmp2	= pcStr2;
	KOS_BOOL		bRet	= TRUE;
	int16			iCnt;

	for(iCnt = 0; iLen > iCnt; iCnt++){
		if(*pcTmp1 != *pcTmp2){ /* 異なる */
			bRet = FALSE;
			break;
		}
		else{ /* 同じ */
			if(*pcTmp1 == '\0'){ /* 終端文字である */
				break;
			}
			else{
				pcTmp1++;
				pcTmp2++;
			}
		}
	}

	return bRet;
}

/*****************************************************************************
	DISCRIPTION	: 1文字送信
	ARGUMENT	: ucChar	= 送信文字
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
int16	putc(uint08	ucChar)
{
	int	iRet;
#if 1 /* Teraterm対策 */
	if(ucChar == '\n'){
		iRet = srlSendByte(SERIAL_DEFAULT_DEV, '\n');
		iRet = srlSendByte(SERIAL_DEFAULT_DEV, '\r');
	}
	else{
#endif /* Teraterm対策 */
		iRet = srlSendByte(SERIAL_DEFAULT_DEV, ucChar);
#if 1 /* Teraterm対策 */
	}
#endif /* Teraterm対策 */

	return iRet;
}

/*****************************************************************************
	DISCRIPTION	: 文字列送信
	ARGUMENT	: pucStr	= 送信する文字列
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
int16	puts(char	*pucStr)
{
	char	*pucTmp = pucStr;
	while(*pucTmp != 0){
		putc(*pucTmp);
		pucTmp++;
	}
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: 1文字受信
	ARGUMENT	: -
	RETURN		: 受信文字
	NOTE		: -
	UPDATED		: 2013-10-16
*****************************************************************************/
uint08	getc(void)
{
	uint08	ucChar = srlRecvByte(SERIAL_DEFAULT_DEV);
	if(ucChar == '\r'){
		ucChar = '\n';
	}
	putc(ucChar);	/* エコーバック */

	return ucChar;
}

/*****************************************************************************
	DISCRIPTION	: 文字列受信
	ARGUMENT	: pucStr	= 受信する文字列
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2013-10-16
*****************************************************************************/
int16	gets(uint08	*pucStr)
{
	int16	iCnt = 0;
	uint08	ucChar;

	do{
		ucChar = getc();
		if(ucChar == '\n'){
			ucChar = '\0';
		}
		pucStr[iCnt] = ucChar;
		iCnt++;
	}while (ucChar != '\0');
		
	return iCnt - 1;
}

/*****************************************************************************
	DISCRIPTION	: 16進数文字列表示
	ARGUMENT	: lVal		= 表示する値
				  iColumn	= 表示桁数指定(1～8,それ以外は有効桁数表示)
					有効桁数以下の数値が指定された場合、上位桁が削られる。
					ゼロサプレスなし。(上位の無効桁には0が表示される)
				  i0xflg	= 文字列の先頭に"0x"を付与するかどうか
				  			   1で付与、0で付与しない。
	RETURN		: 0			= 正常終了
				  0以外		= 異常終了
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
KOS_STS printhex(uint32 ulVal, int16 iColumn, int16 i0xflg)
{
	int16	iCnt;
	uint32	ulTmp;
	int16	iIgnoreDigit;	/* 表示しない上位桁数 */
	char	cTmp[HEX_DISP_MAX + 1];
	char	cStr[2 + HEX_DISP_MAX + 1] = {'0', 'x', '\0'};	/* "0x" + 最大文字数 + 終端文字 */
	char*	pcHexChars = "0123456789ABCDEF";
	
	cTmp[HEX_DISP_MAX] = '\0';

	/* テンポラリ文字列を作成 */
	for(iCnt = 0; iCnt < HEX_DISP_MAX; iCnt++){
		ulTmp = (ulVal & 0x0f);
		/* バッファの末尾から、下位桁の数字を代入する */
		cTmp[HEX_DISP_MAX - iCnt - 1] = pcHexChars[ulTmp];
		ulVal >>= 4;
	}

#if 0
	puts(">> debug print\ncTmp = ");
	puts(cTmp);
	puts("\n<<\n");
#endif

	if((1 <= iColumn) && (iColumn <= 8)){ /* 指定桁数が有効範囲内 */
		iIgnoreDigit = HEX_DISP_MAX - iColumn;
		for(iCnt = 0; HEX_DISP_MAX > (iIgnoreDigit + iCnt); iCnt++){
			cStr[2 + iCnt] = cTmp[iIgnoreDigit + iCnt];
		}
		cStr[2 + iCnt] = '\0';
	}
	else{
		for(iCnt = 0; HEX_DISP_MAX > iCnt; iCnt++){ /* 上位桁の0をスキップする */
			if(cTmp[iCnt] != '0'){
				break;
			}
		}

		if(iCnt == HEX_DISP_MAX){ /* 値が0だった */
			cStr[2] = '0';
			cStr[3] = '\0';
		}
		else{
			iIgnoreDigit = iCnt;
			for(iCnt = 0; HEX_DISP_MAX > (iIgnoreDigit + iCnt); iCnt++){
				cStr[2 + iCnt] = cTmp[iIgnoreDigit + iCnt];
			}
			cStr[2 + iCnt] = '\0';
		}
	}

	if(i0xflg == 0){
		puts(&cStr[2]);
	}
	else{
		puts(cStr);
	}

	return KOS_OK;
}

/*****************************************************************************
	DISCRIPTION	: 文字列をHEX値に変換する
	ARGUMENT	: pHexAsc	= 16進数値を示す文字列 (I)
				  usSize	= 変換する文字数 (I)
	RETURN		: -1以外	= 変換HEX値
				  -1		= 変換失敗
	NOTE		: 変換できない場合、-1を返すため、負になる整数は変換
				: できない事に注意。
	UPDATED		: 2009/05/15 SAI
*****************************************************************************/
uint32 asc2hex(const char* pHexAsc, uint16 usStrNum)
{
	uint32 ulHex = 0;
	uint32 i;

	if (0 == pHexAsc[0])
	{
		return(-1);	/* 1[文字]も無いため変換できない */
	}
	
	if((usStrNum < 1) || (8 < usStrNum)){
		return -1;
	}

	for (i = 0; i < usStrNum; i++)
	{
		uint16 value;
		if ('0' <= pHexAsc[i] && pHexAsc[i] <= '9')
		{
			value = pHexAsc[i] - '0';
		}
		else if ('a' <= pHexAsc[i] && pHexAsc[i] <= 'f')
		{
			value = pHexAsc[i] - 'a' + 10;
		}
		else if ('A' <= pHexAsc[i] && pHexAsc[i] <= 'F')
		{
			value = pHexAsc[i] - 'A' + 10;
		}
		else
		{
			return(-1);	/* 変換できない */
		}
		ulHex <<= 4;
		ulHex += value;
	}
	return(ulHex);
}

/*****************************************************************************
	DISCRIPTION	: 10進文字かどうかの判定
	ARGUMENT	: cChar		= 文字
	RETURN		: 0以外		= 真
				  0			= 偽
	NOTE		: -
	UPDATED		: 2014-06-20
*****************************************************************************/
int isdigit(char cChar)
{
	if(('0' <= cChar) && (cChar <= '9')){
		return 1;
	}
	return 0;
}

/*****************************************************************************
	DISCRIPTION	: ミリ秒単位のウェイト
	ARGUMENT	: ms	= ミリ秒
	RETURN		: -
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
void waitms(int ms) {
	volatile int	a, b;

	for(a = 0;a < ms;a++) {
		for(b = 0;b < 2000;b++);
	}
}

/*****************************************************************************
	DISCRIPTION	: マイクロ秒単位のウェイト
	ARGUMENT	: ms	= マイクロ秒
	RETURN		: -
	NOTE		: -
	UPDATED		: 2013-09-03
*****************************************************************************/
void waitus(int us) {
	volatile int	a;

	for(a = 0;a < us*2;a++);
}

/***** End Of File *****/


