#!/usr/bin/sh
# (C) 2014 FrankHB.
# Example script for build YSLib using SHBuild.

set -e

SHBOPT="-xid,alternative -xid,data -xid,include"

echo Options: ${SHBOPT}`printf " \"%s\"" "$@"` .

COMMON_CXXFLAGS=" \
	-O3 -pipe -s -std=c++11 \
	-DNDEBUG \
	-Wall \
	-Wcast-align \
	-Wextra \
	-Winit-self \
	-Winvalid-pch \
	-Wmain \
	-Wmissing-declarations \
	-Wmissing-include-dirs \
	-Wnon-virtual-dtor \
	-Wredundant-decls \
	-Wunreachable-code \
	-Wzero-as-null-pointer-constant \
	-fdata-sections \
	-fexpensive-optimizations \
	-ffat-lto-objects \
	-ffunction-sections \
	-flto=jobserver \
	-fomit-frame-pointer \
	-mthreads \
	-pedantic-errors \
	"

# TODO: Run link commands.
./shbuild ${SHBOPT} "$@" ../../YBase \
	${COMMON_CXXFLAGS} \
	-I../../YBase/include
./shbuild ${SHBOPT} "$@" ../../YFramework \
	${COMMON_CXXFLAGS} \
	-DFREEIMAGE_LIB \
	-I../../YFramework/include -I../../YFramework/Android/include \
	-I../../YFramework/DS/include -I../../YFramework/MinGW32/include \
	-I../../3rdparty/include -I../../YBase/include

echo Done.

