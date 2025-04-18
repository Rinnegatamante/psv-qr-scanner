TARGET		:= qr_scanner
APP_NAME	:= QR Scanner
TITLE		:= QRSCANNER

LIBS = -lcurl -lquirc -lvita2d -lSceKernelDmacMgr_stub -lc -lSceCommonDialog_stub -lSceLibKernel_stub \
	-lSceNet_stub -lSceNetCtl_stub -lpng -lSceDisplay_stub -lSceGxm_stub -lSceAppMgr_stub \
	-Wl,--whole-archive -lSceSysmodule_stub -Wl,--no-whole-archive -lSceCtrl_stub -lm \
	-lSceAppUtil_stub -lScePgf_stub -ljpeg -lSceRtc_stub -lScePower_stub -lcurl -lssl -lcrypto -lz \
	-lSceCamera_stub

COMMON_OBJS = main.o dialogs.o network.o
	
CFILES	:= $(COMMON_OBJS)
CGFILES  := $(foreach dir,$(SHADERS), $(wildcard $(dir)/*.cg))
CGSHADERS  := $(CGFILES:.cg=.h)
OBJS     := $(CFILES:.c=.o)

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -Wl,-q -O3 -g
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	vita-make-fself -c -s $< eboot.bin
	vita-mksfoex -s TITLE_ID=$(TITLE) "$(APP_NAME)" param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin $(TARGET).vpk \
		-a icon0.png=sce_sys/icon0.png

%.velf: %.elf
	cp $< $<.unstripped.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS) $(TARGET).elf.unstripped.elf $(TARGET).vpk eboot.bin param.sfo
