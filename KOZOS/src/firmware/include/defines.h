/*****************************************************************************
	オーダー	: 定義、型定義、構造体定義 ヘッダファイル
	CPU			: H8 3069F
	コンパイラ	: h8300-elf-gcc version 3.4.6
	ファイル名	: defines.h
	作成日時	: 2013-09-03
*****************************************************************************/
/*
	作成日時
*/

#ifndef _DEFINES_H_
#define _DEFINES_H_

/*****************************************************************************
	インクルード
*****************************************************************************/

/*****************************************************************************
	定義
*****************************************************************************/
#define	_DEBUG_
#ifdef	_DEBUG_
#define	DPUTS	puts
#define	DPHEX	printhex
#else	/* _DEBUG_ */
#define	DPUTS(...)
#define	DPHEX(...)
#endif	/* _DEBUG_ */

#define	NULL				((void*)0)

/* ビット操作関連マクロ x:変数 y:ビット位置0～31 */
#define	SET_BIT(x, y)	((x) | (1 << (y)))
#define	CLR_BIT(x, y)	((x) | ~(1 << (y)))
#define	CHK_BIT(x, y)	(((x) >> (y)) & 1)

#define	K_ERR			(-1)

#define	BIT0	(1 << 0)
#define	BIT1	(1 << 1)
#define	BIT2	(1 << 2)
#define	BIT3	(1 << 3)
#define	BIT4	(1 << 4)
#define	BIT5	(1 << 5)
#define	BIT6	(1 << 6)
#define	BIT7	(1 << 7)


/*****************************************************************************
	型定義、構造体定義
*****************************************************************************/
typedef	unsigned char		uint08;	/* uc */
typedef	unsigned short		uint16;	/* us */
typedef	unsigned long		uint32;	/* ul */
typedef signed char			int08;	/* c */
typedef	signed short		int16;	/* i */
typedef signed long			int32;	/* l */

/* スレッドID */
typedef	unsigned long		KZ_ID_T;
/* スレッドのメイン関数の型 */
typedef	int (KZ_FUNC_T)(int argc, char** argv);
/* 割り込みハンドラの型 */
typedef	void (KZ_HANDLER_T)(void);

typedef	enum _kos_bool {
	FALSE = 0,
	TRUE  = 1
} KOS_BOOL;

typedef enum _kos_sts {
	KOS_ER = -1,
	KOS_OK = 0
} KOS_STS;

typedef enum _koz_massage_box_{
	MSGBOX_ID_CONS_INPUT = 0,
	MSGBOX_ID_CONS_OUTPUT,
	MSGBOX_ID_NUM,
	DUMMY,
} KZ_MSGBOX_ID_T;


/*****************************************************************************
	プロトタイプ宣言
*****************************************************************************/

/*****************************************************************************
	外部変数 
*****************************************************************************/

#endif /* _DEFINES_H_ */


/***** End Of File *****/


