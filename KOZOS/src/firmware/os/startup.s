# スタートアップルーチン
# main関数を呼ぶところまで。

.h8300h
.section	.text

# スタートアップ関数
.global		_start
.type		_start,@function
_start:
	mov.l	#_bootstack,sp
# jsrは関数呼び出し
	jsr		@_main

1:
	bra 1b

# ディスパッチ関数
.global		_dispatch
.type		_dispatch,@function
_dispatch:
# er0が引数で「@」がついてアドレスを意味する。
# er0のアドレスが指す先をer7へ格納する。
	mov.l	@er0,er7
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
# rteは割り込み復帰命令で、プログラムカウンタと、CCRの値をスタックから復旧する。
# CCRはコンディションコードレジスタ。
# モードレジスタに相当するもの。
	rte

