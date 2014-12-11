#!/usr/bin/bash
# (C) 2014 FrankHB.
# Common source script: bootstrap configuration.

echo Bootstrap beginned.

SHBuild_ToolDir=$(cd `dirname "$0"`; pwd)
. ${SHBuild_ToolDir}/SHBuild-common.sh
: ${SHBuild_BaseDir:="${SHBuild_ToolDir}/../SHBuild"}

SHBuild_EchoVar_N "SHBuild.BaseDir"
SHBuild_EchoVar_N "SHBuild.ToolDir"

echo Configuring ...

SHBuild_BuildDir="${SHBuild_BaseDir}/.shbuild"

: ${AR:="gcc-ar"}
. ${SHBuild_ToolDir}/SHBuild-common-toolchain.sh

. ${SHBuild_ToolDir}/SHBuild-common-options.sh

INCLUDES=" \
	-I../../YFramework/include -I../../YFramework/Android/include \
	-I../../YFramework/DS/include -I../../YFramework/MinGW32/include \
	-I../../3rdparty/include -I../../YBase/include \
	"

# Coordinated with build 559.
LIBS=" \
	../../YBase/source/ystdex/cassert.cpp \
	../../YBase/source/ystdex/cstdio.cpp \
	../../YBase/source/ystdex/base.cpp \
	../../YBase/source/ystdex/any.cpp \
	../../YBase/source/ystdex/concurrency.cpp \
	../../YFramework/source/CHRLib/chrmap.cpp \
	../../YFramework/source/CHRLib/chrproc.cpp \
	../../YFramework/source/CHRLib/MapEx.cpp \
	../../YFramework/source/YCLib/Debug.cpp \
	../../YFramework/source/YCLib/FileSystem.cpp \
	../../YFramework/source/YCLib/Host.cpp \
	../../YFramework/source/YCLib/ycommon.cpp \
	../../YFramework/source/YSLib/Core/YCoreUtilities.cpp \
	../../YFramework/source/YSLib/Core/yexcept.cpp \
	../../YFramework/source/YSLib/Core/ValueNode.cpp \
	../../YFramework/source/YSLib/Service/FileSystem.cpp \
	../../YFramework/source/YSLib/Service/File.cpp \
	../../YFramework/source/YSLib/Service/TextFile.cpp \
	../../YFramework/source/NPL/Lexical.cpp \
	../../YFramework/source/NPL/SContext.cpp
	"
if uname | grep MINGW > /dev/null || uname | grep MSYS > /dev/null; then
	LIBS="${LIBS} \
		../../YFramework/MinGW32/source/YCLib/MinGW32.cpp \
		../../YFramework/MinGW32/source/YCLib/Consoles.cpp \
		"
fi

# Note '-fwhole-program' should not be used because there
#	does exsit multiple translation units when linking with YSLib source,
#	otherwise there would be unresolved reference to names with external
#	linkage which had been optimized away.
export CXXFLAGS
export LDFLAGS

SHBuild_EchoVar_N "SHBuild.BuildDir"
SHBuild_EchoVar_N "CXX"
SHBuild_EchoVar_N "CXXFLAGS"
SHBuild_EchoVar_N "AR"
SHBuild_EchoVar_N "LD"
SHBuild_EchoVar_N "LDFLAGS"
SHBuild_EchoVar_N "INCLUDES"
SHBuild_EchoVar_N "LIBS"

echo Configuring done.

