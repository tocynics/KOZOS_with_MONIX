/*****************************************************************************
	オーダー	: DRAM制御
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: dram.h
	接頭辞		: dram
	作成日時	: 2014-06-12
*****************************************************************************/
/*
	改訂履歴
*/
/*****************************************************************************
	インクルード
*****************************************************************************/
#include "defines.h"
#include "lib.h"
#include "dram.h"

/*****************************************************************************
	定義
*****************************************************************************/
//#define	_DRAM_DEBUG_

#ifdef _H8_3069F_ARCH_
#define P1DDR	((volatile uint08 *)0xFEE000)
#define P2DDR	((volatile uint08 *)0xFEE001)
#define P3DDR	((volatile uint08 *)0xFEE002)
#define P4DDR	((volatile uint08 *)0xFEE003)
#define P5DDR	((volatile uint08 *)0xFEE004)
#define P4PCR	((volatile uint08 *)0xFEE03E)
#define P8DDR	((volatile uint08 *)0xFEE007)
#define P8DR	((volatile uint08 *)0xFFFFD7)
#define PBDDR	((volatile uint08 *)0xFEE00A)
#define ABWCR	((volatile uint08 *)0xFEE020)
#define ASTCR	((volatile uint08 *)0xFEE021)
#define WCRH	((volatile uint08 *)0xFEE022)
#define WCRL	((volatile uint08 *)0xFEE023)
#define BRCR	((volatile uint08 *)0xFEE013)
#define CRSR	((volatile uint08 *)0xFEE01F)
#define ADRCR	((volatile uint08 *)0xFEE01E)
#define BCR		((volatile uint08 *)0xFEE024)
#define DRCRA	((volatile uint08 *)0xFEE026)
#define DRCRB	((volatile uint08 *)0xFEE027)
#define RTMCSR	((volatile uint08 *)0xFEE028)
#define RTCOR	((volatile uint08 *)0xFEE02A)
#define	CSCR	((volatile uint08 *)0xFEE01F)
#endif	/* _H8_3069F_ARCH_ */

/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
typedef struct {
	union {
		volatile uint08 ucVal08[4];
		volatile uint16 usVal16[2];
		volatile uint32 ulVal32[1];
	} u;
} val_t;

typedef struct _dram_register{
	volatile uint08*	ucReg;
	char	cRegName[8];
} DRAM_REG;

/*****************************************************************************
	外部変数
*****************************************************************************/

/*****************************************************************************
	内部変数
*****************************************************************************/
#ifdef	_DRAM_DEBUG_
static DRAM_REG dramRegs[] = {
	{P1DDR,		"P1DDR "},
	{P2DDR,		"P2DDR "},
	{P3DDR,		"P3DDR "},
	{P4DDR,		"P4DDR "},
	{P5DDR,		"P5DDR "},
	{P4PCR,		"P4PCR "},
	{P8DDR,		"P8DDR "},
	{PBDDR,		"PBDDR "},
	{ABWCR,		"ABWCR "},
	{ASTCR,		"ASTCR "},
	{WCRH,		"WCRH  "},
	{WCRL,		"WCRL  "},
	{BRCR,		"BRCR  "},
	{CRSR,		"CRSR  "},
	{ADRCR,		"ADRCR "},
	{BCR,		"BCR   "},
	{DRCRA,		"DRCRA "},
	{DRCRB,		"DRCRB "},
	{RTMCSR,	"RTMCSR"},
	{RTCOR,		"RTCOR "},
	
	{NULL,		""},		/* End Of Array */
};
#endif	/* _DRAM_DEBUG_ */

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/

/*****************************************************************************
	DISCRIPTION	: DRAMの初期化
	ARGUMENT	: -
	RETURN		: 0		= 正常終了
				  0以外	= 異常終了
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
volatile int dramInit(void)
{
#ifndef _20MHZ_CPU_
	*P1DDR  = 0xff;
	*P2DDR  = 0xff;
	*P8DDR  = 0x1C;
	
	*ABWCR  = 0xff;
	*RTCOR  = 0x0A;
	*RTMCSR = 0x18;

	*DRCRB  = 0x90;
	*DRCRA  = 0x30;
#else	/* _25MHZ_CPU_ */
	*P1DDR  = 0xff;
	*P2DDR  = 0x07;
	*P8DDR  = 0x06;
	
	*ABWCR  = 0x00;
	*RTCOR  = 0x0A;
	*RTMCSR = 0x30;

	*DRCRB  = 0x90;
	*DRCRA  = 0x3C;
#endif	/* _25MHZ_CPU_ */

#ifdef _DRAM_DEBUG_
	{
		int i;
		for(i = 0; dramRegs[i].ucReg != NULL; i++){
			puts(dramRegs[i].cRegName);
			puts(": ");
			printhex(*(dramRegs[i].ucReg), 2, 1);
			puts("\n");
		}
	}
#endif	/* _DRAM_DEBUG_ */

	return 0;
}

/*****************************************************************************
	DISCRIPTION	: DRAMのチェック関数01
	ARGUMENT	: -
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
void dramCheck01(void)
{
	volatile uint16* pusBuf = (uint16*)DRAM_START;
	uint16 usBuf = 0;
	(*pusBuf) = 0;
	uint32 i;
	for(i = 0; i != 1; i += 0x1111){
		if(i == 0xFFFF){
			i = 0;
			pusBuf++;
		}
		*pusBuf = i;
		
		printhex((uint32)pusBuf, 8, 0);
		puts(": ");
		printhex((*(uint16*)pusBuf), 4, 0);
		puts("  ");
		
		usBuf = *pusBuf;
		printhex((uint32)&usBuf, 8, 0);
		puts(": ");
		printhex(usBuf, 4, 0);
		puts("\n");
	}
}
/*****************************************************************************
	DISCRIPTION	: DRAMのチェック関数02
	ARGUMENT	: -
	RETURN		: -
	NOTE		: -
	UPDATED		: 2014-06-11
*****************************************************************************/
void dramCheck02(void)
{
	volatile uint08* pucBuf = (uint08*)DRAM_START;
	uint08 ucBuf = 0;
	(*pucBuf) = 0;
	uint32 i;
	for(i = 0; 1; i++){
		if(i == 0xFF){
			i = 0;
			pucBuf++;
		}
		*pucBuf = i;
		
		printhex((uint32)pucBuf, 8, 0);
		puts(": ");
		printhex((*(uint08*)pucBuf), 2, 0);
		puts("  ");
		
		ucBuf = *pucBuf;
		printhex((uint32)&ucBuf, 8, 0);
		puts(": ");
		printhex(ucBuf, 2, 0);
		puts("\n");
	}
}

/***** End Of File *****/

