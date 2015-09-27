CC=h8300-elf-gcc
CXX=h8300-elf-gcc
AS=h8300-elf-gcc
LD=h8300-elf-gcc
NM=
AR=
OBJCOPY=h8300-elf-objcopy

OBJNAME=monix

DEFS=-DH8

DEFS+=-DAKI_H8LAN

DEFS+=-DUSE_LIBS
DEFS+=-DUSE_SPRINTF

DEFS+=-DMBUF_NUM=5
DEFS+=-DUSE_DMA

INCS=-I.

ifneq ($(findstring -DAKI_H8LAN, $(DEFS)), )
	LDSCRIPT = ./monix-aki_h8lan.x
endif

CPPFLAGS = -E -mh -x c
CFLAGS=$(DEFS) $(INCS) -mh -mrelax -c -pipe -O2 -Wall
CFLAGS+=-mint32
AFLAGS=$(DEFS) $(INCS) -mh -mint32 -mrelax -c -pipe -O2 -Wall
LDFLAGS=-mh -mint32 -mrelax -nostartfiles -Wl,-Map,$(OBJNAME).map -Wl,-T$(LDSCRIPT)
ifneq ($(findstring -DUSE_LIBS, $(DEFS)), )
	LDFLAGS+=-nodefaultlibs -nostdlib
endif
LIBS=-lgcc -lc

VPATH=.

OBJS=vector.o
OBJS+=crt0.o
OBJS+=interrupt.o
OBJS+=intr.o
OBJS+=bus.o 
OBJS+=monix.o
OBJS+=cmd.o
OBJS+=sci.o
OBJS+=memutil.o
OBJS+=sys.o
OBJS+=ctype.o
ifneq ($(findstring -DUSE_LIBS, $(DEFS)), )
	OBJS+=string.o
	OBJS+=stdio.o
endif
ifneq ($(findstring -DUSE_DMA, $(DEFS)), )
	OBJS+=dma.o
endif

all: $(OBJNAME).mot

$(OBJNAME).mot:	$(OBJNAME).elf
	$(OBJCOPY) -O srec -S $(OBJNAME).elf $@
$(OBJNAME).bin:	$(OBJNAME).elf
	$(OBJCOPY) -O binary $(OBJNAME).elf $@

# Link object files
$(OBJNAME).elf:	$(OBJS) $(LDSCRIPT) $(HEADERS)
	$(LD) $(LDFLAGS) -o $(OBJNAME).elf $(OBJS) $(LIBS)

# Compile .c files
%.o:	%.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

# Assemble .o files
%.o:	%.s $(HEADERS)
	$(AS) $(AFLAGS) $< -o $@

# Assemble .o files which are being preprocessed
%.o:	%.S $(HEADERS)
	$(AS) $(AFLAGS) $< -o $@

clean:
	rm -f *.o *.d $(OBJNAME) *.syms *.bin *.lst *.elf *.mot *.map

-include $(OBJS:.o=.d)
