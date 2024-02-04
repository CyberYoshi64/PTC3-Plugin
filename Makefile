ifeq ($(strip $(DEVKITPRO)),)
    $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
endif

ENABLE_LINK_TIME_OPTIMIZATIONS := 1
NAME := ptc3plg

BUILD_DIR := build
OUTPUT_DIR := output
SOURCE_DIRS := source
INCLUDE_DIRS := $(SOURCE_DIRS) include

LIBRARY_DIRS = lib/Library $(DEVKITPRO)/libctru
LIBRARIES = ctrpf ctru

VERSION_MAJOR := 0
VERSION_MINOR := 0
VERSION_MICRO := 6

COMMIT_HASH		:=	$(shell git rev-parse --short=7 HEAD)
COMPILE_DATE 	:=  $(shell date -u +"%y%m%d%H%M")

BUILD_FLAGS := 	-Os -march=armv6k -mtune=mpcore -mfloat-abi=hard \
				-Wno-address-of-packed-member \
				-DBUILD_DATE="\"$(COMPILE_DATE)\"" \
				-DCOMMIT_HASH="0x0$(COMMIT_HASH)"

BUILD_FLAGS_CC := 	-mword-relocations -fno-strict-aliasing \
					-fomit-frame-pointer -ffunction-sections

BUILD_FLAGS_CXX := -fno-rtti -fno-exceptions -std=gnu++20

include resources/makerules

re : clean .WAIT all
.PHONY: re