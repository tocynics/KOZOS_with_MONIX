# makefileの共通定義をインクルード
include ../inc.mak

CFLAGS	+= -DKZLOAD#		条件コンパイルのデファインでしょう
#CFLAGS	+= -D_20MHZ_CPU_#	H8/3069F LANボードで使用

##########	生成する実行形式のファイル名
TARGET	= kzload

##########	他のディレクトリ
KZ_OS_COMMON_DIR	= ../common
KZ_OS_DAEMON_DIR	= ../daemon

##########	コンパイルするソースコード郡
SRCS	=  vector.c
SRCS	+= startup.s
SRCS	+= $(KZ_OS_COMMON_DIR)/intr.S
SRCS	+= main.c
SRCS	+= $(KZ_OS_COMMON_DIR)/xmodem.c
SRCS	+= $(KZ_OS_COMMON_DIR)/elf.c
SRCS	+= $(KZ_OS_COMMON_DIR)/lib.c
SRCS	+= $(KZ_OS_COMMON_DIR)/serial.c
SRCS	+= $(KZ_OS_COMMON_DIR)/interrupt.c
SRCS	+= $(KZ_OS_COMMON_DIR)/dram.c

OBJS = $(addsuffix .o, $(basename $(realpath $(SRCS))))

########## ターゲット部
# makeコマンドでターゲットを指定しなかった場合
# 一番上に記述されているターゲットが実行される
# ":"のあとには、ターゲットを指定するが、
# タスクターゲットとファイルターゲットがある
# all:は、タスクターゲットを指定している。
# $(TARGET)はファイルターゲットを指定している。
# ファイルターゲットを指定した場合、ファイルのタイムスタンプを
# 確認してから実行する。
# よって、ファイルに変更がなかった場合、そのターゲットは実行しない。
.PHONY:	all
all				: $(TARGET)

# 実行形式ファイルの生成ルール
.PHONY:	$(TARGET)
$(TARGET)		: $(OBJS)
	$(CC) $(notdir $(OBJS)) -o $(TARGET) $(CFLAGS) $(LFLAGS)
	cp $(TARGET) $(TARGET).elf
	$(STRIP) $(TARGET)

# *.cファイルのコンパイルルール
.c.o			: $<
	$(CC) -c $(CFLAGS) $<

# *.s(アセンブラファイル)のアセンブルルール
.s.o			: $<
	$(CC) -c $(CFLAGS) $<

# *.S(アセンブラファイル:大文字の.S)のアセンブルルール
.S.o			: $<
	$(CC) -c $(CFLAGS) $<

# モトローラSレコード・フォーマットへの変換ルール
.PHONY:	$(TARGET).mot
$(TARGET).mot	: $(TARGET)
	$(OBJCOPY) -O srec $(TARGET) $(TARGET).mot

# イメージファイル作成ルール
.PHONY:	image
image			: $(TARGET).mot

# フォルダの掃除
.PHONY:	clean
clean			:
	rm -f $(OBJS) $(TARGET) $(TARGET).elf $(TARGET).mot *.o *.elf *.exe *.mot


