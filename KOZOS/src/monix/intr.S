# 割り込み処理定義
# ファイル名の拡張子が、大文字と小文字の場合があるが、
# 大文字の場合、C言語のプリプロセスに当たる処理が実行される。
# 小文字の場合は実行されない。

# Windows用の頒布版バイナリmakeは不備があり、これが区別されなかった為、
# ソースをダウンロードしてきてコンパイルしなおし、makeが正常に通る
# ようにした。

#include "intr.h"
	.h8300h
	.section	.text

#	ソフトウェアエラー
	.global		_intrSofterr
	.type		_intrSofterr,@function
_intrSofterr:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.l	#_intrstack,sp
	mov.l	er1,@-er7
	mov.w	#SV_SOFTERR,r0
	jsr		@_svInterrupt
	mov.l	@er7+,er1
	mov.l	@er1,er7
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte

#	システムコール
	.global	_intrSyscall
	.type	_intrSyscall,@function
_intrSyscall:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.l	#_intrstack,sp
	mov.l	er1,@-er7
	mov.w	#SV_SYSCALL,r0
	jsr		@_svInterrupt
	mov.l	@er7+,er1
	mov.l	@er1,er7
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte
	
#	シリアル割り込み
	.global	_intrSerial
	.type	_intrSerial,@function
_intrSerial:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.l	#_intrstack,sp
	mov.l	er1,@-er7
	mov.w	#SV_SERIAL,r0
	jsr		@_svInterrupt
	mov.l	@er7+,er1
	mov.l	@er1,er7
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte


