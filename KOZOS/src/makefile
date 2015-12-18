#	makefile 

LD_PATH = "firmware/loader"
OS_PATH = "firmware/os"
MONIX_PATH = "monix"

.PHONY:	all
all				:
	@echo	"----- You have to choice which targets. -----"
	@echo	">> ld (KOZ OS loader)"
	@echo	">> ld_image"
	@echo	">> ld_clean"
	@echo	">> ld_build"
	@echo	">> os (KOZ OS)"
	@echo	">> os_image"
	@echo	">> os_clean"
	@echo	">> os_build"
	@echo	">> monix (toppers jsp, HOS, and KOZOS loader)"
	@echo	">> os_image"
	@echo	">> os_clean"
	@echo	">> os_build"
	@echo	">> cleanup"
	@echo	">> set_project"

.PHONY: ld
ld				:
	@echo 	"----- I make up Koz Loader. -----"
	cd $(LD_PATH); make

.PHONY:	ld_image
ld_image		:
	@echo	"----- I make up Motorola S Recode format file. -----"
	cd $(LD_PATH); make image

.PHONY:	ld_clean
ld_clean		:
	@echo	"----- I clean this directory. -----"
	cd $(LD_PATH); make clean

.PHONY:	ld_build
ld_build		:
	cd $(LD_PATH); make clean
	cd $(LD_PATH); make
	cd $(LD_PATH); make image

#.PHONY:	ld_write
#ld_write		:
#	@echo "----- Write module to aki... -----"
#	cd $(LD_PATH); h8write.exe -3069 -f25 kzload.mot COM3

.PHONY:	os
os				:
	@echo "----- I make up koz os. -----"
	cd $(OS_PATH); make

.PHONY:	os_image
os_image		:
	@echo "----- I make up koz os. -----"
	cd $(OS_PATH); make image;

.PHONY:	os_clean
os_clean		:
	@echo "----- I clean koz os. -----"
	cd $(OS_PATH); make clean

.PHONY:	os_build
os_build		:
	@echo "----- I remake up koz os. -----"
	cd $(OS_PATH); make clean
	cd $(OS_PATH); make

.PHONY:	monix
monix			:
	@echo "----- I make up monix. -----"
	cd $(MONIX_PATH); make

.PHONY:	monix_image
monix_image		:
	@echo "----- I make up monix. -----"
	cd $(MONIX_PATH); make image;

.PHONY:	monix_clean
monix_clean		:
	@echo "----- I clean monix. -----"
	cd $(MONIX_PATH); make clean

.PHONY:	monix_build
monix_build		:
	@echo "----- I remake up monix. -----"
	cd $(MONIX_PATH); make clean
	cd $(MONIX_PATH); make

.PHONEY: cleanup
cleanup			:
	@echo "----- all module and exe and other clean. -----"
	cd $(LD_PATH); make clean
	cd $(OS_PATH); make clean
	cd $(MONIX_PATH); make clean
	rm tags *.vimpj

# "include ./Makefile.in"
# 上記のように記述し、別途makefileを用意すれば、
# makefileを分割して記述できる。
#
# .PHONY run_exe
# run_exe : ...
# などとすれば、make経由で作成したプログラムを実行できる。
#
