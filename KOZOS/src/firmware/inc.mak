#####	Makefileのインクルードファイル
#####	各種Makefileの共通部分の定義など


#PREFIX = /usr/local
#BINDIR	= $(PREFIX)/bin

###########	ツール名称とかの先頭につけるやつ
ARCH	= h8300-elf
ADDNAME	= $(ARCH)-

HEADPATH = $(shell cygpath -m $(realpath ..))

###########	利用するツール類の指定
AR		= $(ADDNAME)ar
AS		= $(ADDNAME)as
CC		= $(ADDNAME)gcc
LD		= $(ADDNAME)ld
NM		= $(ADDNAME)nm
OBJCOPY	= $(ADDNAME)objcopy
OBJDUMP	= $(ADDNAME)objdump
RANLIB	= $(ADDNAME)ranlib
STRIP	= $(ADDNAME)strip

##########	コンパイルオプション(Compile Flags)
# すべての警告を表示する
CFLAGS	= -Wall
CFLAGS	+= -mh#				H8/300用のコードを出力する
CFLAGS	+= -nostdinc#		システムのヘッダファイルを利用しない(stdioなど?)
CFLAGS	+= -nostdlib#		システムのライブラリを利用しない(stdlibなど?)
CFLAGS	+= -fno-builtin #	コンパイラのビルトイン関数を利用しない
CFLAGS	+= -I.#				ヘッダファイルの検索先を指定する
CFLAGS	+= -I$(HEADPATH)/include
CFLAGS	+= -I$(HEADPATH)/daemon
#CFLAGS	+= -Os#				サイズ節約の最適化を行う
CFLAGS	+= -D_H8_3069F_ARCH_
#CFLAGS += -g#				デバッガを利用する
#CFLAGS += -mint32#			intを32ビットにすると掛け算/割り算ができなくなる。
# 以下のような指定もできるが、行コメントを末尾に追加できない
# CFLAGS	= -Wall \
# 	-mh \
# 	-nostdinc \
# 	-nostdlib \
# 	-fno-builtin \
# 	-I. \
# 	-Os \
## 	-g \
## 	-mint32 \
## 	-DKOZOS

##########	リンクオプション(Link Flags)
LFLAGS	= -static#			すべて静的リンクする
LFLAGS	+= -T linker.ld#	リンカ・スクリプトを指定する
LFLAGS	+= -L.#				ライブラリの検索先を指定する
LFLAGS	+= -L../common


##### 以下の様な方法でコンパイルするファイルを指定することも出来る
#ヘッダファイルは、例えばLinuxのシステムのヘッダファイル等と名前が
#かぶる場合に、下記のヘッダファイル指定を行い、ファイル名解決をする。
#HDRS = initProc.h \
#       ../inc/rabbit.h \
#       ../inc/shmem.h \
#       ../inc/msgFunc.h \
#       ../inc/devFunc.h
#
#SRCS = initProc.c \
#       ../common/shmem.c \
#       ../common/msgFunc.c \
#       ../common/devFunc.c
#
#OBJS = $(addsuffix .o,$(basename $(SRCS)))


