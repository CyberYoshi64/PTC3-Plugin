.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ENABLE_LINK_TIME_OPTIMIZATIONS = 1

export COMMIT_HASH		:=	$(shell git rev-parse --short=8 HEAD)
export COMPILE_DATE 	:=  $(shell date -u +"%y%m%d%H%M")

TOPDIR 		?= 	$(CURDIR)
include $(DEVKITARM)/3ds_rules

TARGET		:= 	ptc3plg
PLGINFO 	:= 	$(TARGET).plgInfo

BUILD		:= 	build
INCLUDES	:= 	include lib/Library/include
SOURCES 	:= 	source

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH		:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS		:=	$(ARCH) -Os -mword-relocations \
				-fomit-frame-pointer -ffunction-sections -fno-strict-aliasing \
				-Wno-address-of-packed-member

DEFINES		:= -DBUILD_DATE="\"$(COMPILE_DATE)\"" -DCOMMIT_HASH="\"$(COMMIT_HASH)\""

CFLAGS		+=	$(INCLUDE) -D__3DS__ $(DEFINES)

ASFLAGS		:=	$(ARCH)
LDFLAGS		:= -T $(TOPDIR)/3gx.ld $(ARCH) -Os -Wl,--gc-sections,--strip-discarded,--strip-debug

ifneq ($(ENABLE_LINK_TIME_OPTIMIZATIONS), 0)
	CFLAGS += -flto=auto -ffat-lto-objects
	ASFLAGS += -flto=auto -ffat-lto-objects
endif
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++20

LIBS		:= -lctrpf -lctru
LIBDIRS		:=  $(TOPDIR) $(CTRULIB) $(PORTLIBS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES			:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES			:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export LD 		:= 	$(CXX)
export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I $(CURDIR)/$(dir) ) \
					$(foreach dir,$(LIBDIRS),-I $(dir)/include) \
					-I $(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L $(dir)/lib)

.PHONY: $(BUILD) clean lib all

#---------------------------------------------------------------------------------
all: $(BUILD)

lib:
	@cd lib && make clean && make release && cd ..

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/cookbook.mk

#---------------------------------------------------------------------------------
clean:
	@echo clean ... 
	@rm -rf $(BUILD) $(OUTPUT).3gx $(OUTPUT).elf

re: clean lib .WAIT all

#---------------------------------------------------------------------------------

else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).3gx : $(OFILES)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
.PRECIOUS: %.elf

%.3gx: .WAIT %.elf
#---------------------------------------------------------------------------------
	@echo creating $(notdir $@)
	@3gxtool -d -s $(word 1, $^) $(TOPDIR)/$(PLGINFO) $@

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
