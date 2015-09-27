/*****************************************************************************
	オーダー	: H8/3069F レジスタ定義
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 4.7.0
	ファイル名	: reg3069.h
	接頭辞		: 
	作成日時	: 2015-05-07
*****************************************************************************/
/*
	改訂履歴
*/

#ifndef _KZ_REG3069_H_
#define _KZ_REG3069_H_

/*****************************************************************************
	インクルード
*****************************************************************************/
#include	"defines.h"

/*****************************************************************************
	定義
*****************************************************************************/
#define	MDCR		(*(volatile uint08*)0xFEE011)
#define	SYSCR		(*(volatile uint08*)0xFEE012)
#define	BRCR		(*(volatile uint08*)0xFEE013)
#define	ISCR		(*(volatile uint08*)0xFEE014)
#define	IER			(*(volatile uint08*)0xFEE015)
#define	ISR			(*(volatile uint08*)0xFEE016)
#define	IPRA		(*(volatile uint08*)0xFEE018)
#define	IPRB		(*(volatile uint08*)0xFEE019)
#define	CSCR		(*(volatile uint08*)0xFEE01F)
#define	ABWCR		(*(volatile uint08*)0xFEE020)
#define	ASTCR		(*(volatile uint08*)0xFEE021)
#define	WCRH		(*(volatile uint08*)0xFEE022)
#define	WCRL		(*(volatile uint08*)0xFEE023)
#define	BCR			(*(volatile uint08*)0xFEE024)
#define	RAMCR		(*(volatile uint08*)0xFEE077)
#define	FLMCR		(*(volatile uint08*)0xFEE030)
#define	FLMCR1		(*(volatile uint08*)0xFEE030)
#define	FLMCR2		(*(volatile uint08*)0xFEE031)
#define	EBR			(*(volatile uint08*)0xFEE032)
#define	EBR1		(*(volatile uint08*)0xFEE032)
#define	EBR2		(*(volatile uint08*)0xFEE033)
#define	FLMSR		(*(volatile uint08*)0xFEE07D)
#define	TSTR		(*(volatile uint08*)0xFFFF60)
#define	TSNC		(*(volatile uint08*)0xFFFF61)
#define	TMDR		(*(volatile uint08*)0xFFFF62)
#define	TOLR		(*(volatile uint08*)0xFFFF63)
#define	TISRA		(*(volatile uint08*)0xFFFF64)
#define	TISRB		(*(volatile uint08*)0xFFFF65)
#define	TISRC		(*(volatile uint08*)0xFFFF66)
#define	TCR0		(*(volatile uint08*)0xFFFF68)
#define	TIOR0		(*(volatile uint08*)0xFFFF69)

#define	TCNT0		(*(volatile uint16*)0xFFFF6A)
#define	GRA0		(*(volatile uint16*)0xFFFF6C)
#define	GRB0		(*(volatile uint16*)0xFFFF6E)

#define	TCR1		(*(volatile uint08*)0xFFFF70)
#define	TIOR1		(*(volatile uint08*)0xFFFF71)

#define	TCNT1		(*(volatile uint16*)0xFFFF72)
#define	GRA1		(*(volatile uint16*)0xFFFF74)
#define	GRB1		(*(volatile uint16*)0xFFFF76)

#define	TCR2		(*(volatile uint08*)0xFFFF78)
#define	TIOR2		(*(volatile uint08*)0xFFFF79)

#define	TCNT2		(*(volatile uint16*)0xFFFF7A)
#define	GRA2		(*(volatile uint16*)0xFFFF7C)
#define	GRB2		(*(volatile uint16*)0xFFFF7E)
#define	TCSR		(*(volatile uint16*)0xFFFF8C)
#define	TCNT		(*(volatile uint16*)0xFFFF8D)

#define	T8TCR0		(*(volatile uint08*)0xFFFF80)
#define	T8TCR1		(*(volatile uint08*)0xFFFF81)

#define	SMR0		(*(volatile uint08*)0xFFFFB0)
#define	BRR0		(*(volatile uint08*)0xFFFFB1)
#define	SCR0		(*(volatile uint08*)0xFFFFB2)
#define	TDR0		(*(volatile uint08*)0xFFFFB3)
#define	SSR0		(*(volatile uint08*)0xFFFFB4)
#define	RDR0		(*(volatile uint08*)0xFFFFB5)
#define	SMR1		(*(volatile uint08*)0xFFFFB8)
#define	BRR1		(*(volatile uint08*)0xFFFFB9)
#define	SCR1		(*(volatile uint08*)0xFFFFBA)
#define	TDR1		(*(volatile uint08*)0xFFFFBB)
#define	SSR1		(*(volatile uint08*)0xFFFFBC)
#define	RDR1		(*(volatile uint08*)0xFFFFBD)
#define	SMR1		(*(volatile uint08*)0xFFFFB8)
#define	SMR2		(*(volatile uint08*)0xFFFFC0)
#define	BRR2		(*(volatile uint08*)0xFFFFC1)
#define	SCR2		(*(volatile uint08*)0xFFFFC2)
#define	TDR2		(*(volatile uint08*)0xFFFFC3)
#define	SSR2		(*(volatile uint08*)0xFFFFC4)
#define	RDR2		(*(volatile uint08*)0xFFFFC5)

#define	P1DDR		(*(volatile uint08*)0xFEE000)
#define	P2DDR		(*(volatile uint08*)0xFEE001)
#define	P3DDR		(*(volatile uint08*)0xFEE002)
#define	P4DDR		(*(volatile uint08*)0xFEE003)
#define	P5DDR		(*(volatile uint08*)0xFEE004)
#define	P6DDR		(*(volatile uint08*)0xFEE005)
#define	P7DDR		(*(volatile uint08*)0xFEE006)
#define	P8DDR		(*(volatile uint08*)0xFEE007)
#define	P9DDR		(*(volatile uint08*)0xFEE008)
#define	PADDR		(*(volatile uint08*)0xFEE009)
#define	PBDDR		(*(volatile uint08*)0xFEE00A)
#define	P2PCR		(*(volatile uint08*)0xFEE03C)
#define	P4PCR		(*(volatile uint08*)0xFEE03E)
#define	P5PCR		(*(volatile uint08*)0xFEE03F)
#define	P1DR		(*(volatile uint08*)0xFFFFD0)
#define	P2DR		(*(volatile uint08*)0xFFFFD1)
#define	P3DR		(*(volatile uint08*)0xFFFFD2)
#define	P4DR		(*(volatile uint08*)0xFFFFD3)
#define	P5DR		(*(volatile uint08*)0xFFFFD4)
#define	P6DR		(*(volatile uint08*)0xFFFFD5)
#define	P7DR		(*(volatile uint08*)0xFFFFD6)
#define	P8DR		(*(volatile uint08*)0xFFFFD7)
#define	P9DR		(*(volatile uint08*)0xFFFFD8)
#define	PADR		(*(volatile uint08*)0xFFFFD9)
#define	PBDR		(*(volatile uint08*)0xFFFFDA)

#define	ADDRA		(*(volatile uint16*)0xFFFFE0)
#define	ADDRB		(*(volatile uint16*)0xFFFFE2)
#define	ADDRC		(*(volatile uint16*)0xFFFFE4)
#define	ADDRD		(*(volatile uint16*)0xFFFFE6)

#define	ADDRAH		(*(volatile uint08*)0xFFFFE0)
#define	ADDRAL		(*(volatile uint08*)0xFFFFE1)
#define	ADDRBH		(*(volatile uint08*)0xFFFFE2)
#define	ADDRBL		(*(volatile uint08*)0xFFFFE3)
#define	ADDRCH		(*(volatile uint08*)0xFFFFE4)
#define	ADDRCL		(*(volatile uint08*)0xFFFFE5)
#define	ADDRDH		(*(volatile uint08*)0xFFFFE6)
#define	ADDRDL		(*(volatile uint08*)0xFFFFE7)
#define	ADCSR		(*(volatile uint08*)0xFFFFE8)
#define	ADCR		(*(volatile uint08*)0xFFFFE9)

#define	RTCOR		(*(volatile uint08*)0xFEE02A)
#define	RTMCSR		(*(volatile uint08*)0xFEE028)
#define	DRCRA		(*(volatile uint08*)0xFEE026)
#define	DRCRB		(*(volatile uint08*)0xFEE027)

#define	TCR0_8		(*(volatile uint08*)0xFFFF80)
#define	TCR1_8		(*(volatile uint08*)0xFFFF81)
#define	TCSR0_8		(*(volatile uint08*)0xFFFF82)
#define	TCSR1_8		(*(volatile uint08*)0xFFFF83)
#define	TCORA0_8	(*(volatile uint08*)0xFFFF84)
#define	TCORA1_8	(*(volatile uint08*)0xFFFF85)
#define	TCORB0_8	(*(volatile uint08*)0xFFFF86)
#define	TCORB1_8	(*(volatile uint08*)0xFFFF87)
#define	TCNT0_8		(*(volatile uint08*)0xFFFF88)
#define	TCNT1_8		(*(volatile uint08*)0xFFFF89)
#define	TCR2_8		(*(volatile uint08*)0xFFFF90)
#define	TCR3_8		(*(volatile uint08*)0xFFFF91)
#define	TCSR2_8		(*(volatile uint08*)0xFFFF92)
#define	TCSR3_8		(*(volatile uint08*)0xFFFF93)
#define	TCORA2_8	(*(volatile uint08*)0xFFFF94)
#define	TCORA3_8	(*(volatile uint08*)0xFFFF95)
#define	TCORB2_8	(*(volatile uint08*)0xFFFF96)
#define	TCORB3_8	(*(volatile uint08*)0xFFFF97)
#define	TCNT2_8		(*(volatile uint08*)0xFFFF98)
#define	TCNT3_8		(*(volatile uint08*)0xFFFF99)

#define	MAR0AR		(*(volatile uint08*)0xFFFF20)
#define	MAR0AE		(*(volatile uint08*)0xFFFF21)
#define	MAR0AH		(*(volatile uint08*)0xFFFF22)
#define	MAR0AL		(*(volatile uint08*)0xFFFF23)
#define	IOAR0A		(*(volatile uint08*)0xFFFF26)
#define	ETCR0AH		(*(volatile uint08*)0xFFFF24)
#define	ETCR0AL		(*(volatile uint08*)0xFFFF25)
#define	DTCR0A		(*(volatile uint08*)0xFFFF27)
#define	MAR0BR		(*(volatile uint08*)0xFFFF28)
#define	MAR0BE		(*(volatile uint08*)0xFFFF29)
#define	MAR0BH		(*(volatile uint08*)0xFFFF2A)
#define	MAR0BL		(*(volatile uint08*)0xFFFF2B)
#define	IOAR0B		(*(volatile uint08*)0xFFFF2E)
#define	ETCR0BH		(*(volatile uint08*)0xFFFF2C)
#define	ETCR0BL		(*(volatile uint08*)0xFFFF2D)
#define	DTCR0B		(*(volatile uint08*)0xFFFF2F)
#define	MAR1AR		(*(volatile uint08*)0xFFFF30)
#define	MAR1AE		(*(volatile uint08*)0xFFFF31)
#define	MAR1AH		(*(volatile uint08*)0xFFFF32)
#define	MAR1AL		(*(volatile uint08*)0xFFFF33)
#define	IOAR1A		(*(volatile uint08*)0xFFFF36)
#define	ETCR1AH		(*(volatile uint08*)0xFFFF34)
#define	ETCR1AL		(*(volatile uint08*)0xFFFF35)
#define	DTCR1A		(*(volatile uint08*)0xFFFF37)
#define	MAR1BR		(*(volatile uint08*)0xFFFF38)
#define	MAR1BE		(*(volatile uint08*)0xFFFF39)
#define	MAR1BH		(*(volatile uint08*)0xFFFF3A)
#define	MAR1BL		(*(volatile uint08*)0xFFFF3B)
#define	IOAR1B		(*(volatile uint08*)0xFFFF3E)
#define	ETCR1BH		(*(volatile uint08*)0xFFFF3C)
#define	ETCR1BL		(*(volatile uint08*)0xFFFF3D)
#define	DTCR1B		(*(volatile uint08*)0xFFFF3F)

#define	DADR0		(*(volatile uint08*)0xFFFF9C)
#define	DADR1		(*(volatile uint08*)0xFFFF9D)
#define	DACR		(*(volatile uint08*)0xFFFF9E)
#define	DASTCR		(*(volatile uint08*)0xFEE01A)


/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/

/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/

/*****************************************************************************
	外部変数 
*****************************************************************************/

#endif /* _KZ_REG3069_H_ */

/***** End Of File *****/

