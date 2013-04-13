﻿/*
	Copyright by FrankHB 2012 - 2013.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file DSMain.cpp
\ingroup Helper
\brief DS 平台框架。
\version r3042
\author FrankHB <frankhb1989@gmail.com>
\since build 296
\par 创建时间:
	2012-03-25 12:48:49 +0800
\par 修改时间:
	2013-04-13 13:45 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	Helper::DSMain
*/


#include "DSScreen.h"
#include "Host.h"
#include "Helper/Initialization.h"
#include "YSLib/Adaptor/Font.h"
#include "DSWindow.h"
#if YCL_MULTITHREAD == 1
#	include <thread> // for std::this_thread::*;
#endif
#ifdef YCL_DS
#	include "YSLib/Service/yblit.h" // for Drawing::FillPixel;
#endif
#include "YCLib/Debug.h"

YSL_BEGIN

using namespace Drawing;

namespace
{

#if YCL_MINGW32
yconstexpr double g_max_free_fps(1000);
std::chrono::nanoseconds host_sleep(u64(1000000000 / g_max_free_fps));
#endif


//注册的应用程序指针。
DSApplication* pApp;


#if YCL_MINGW32
//! \since build 381
::HWND
InitializeMainWindow(const wchar_t* wnd_title, u16 wnd_w, u16 wnd_h,
	::DWORD wstyle = WS_TILED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
{
	::RECT rect{0, 0, wnd_w, wnd_h};

	::AdjustWindowRect(&rect, wstyle, FALSE);
	return ::CreateWindowW(Host::WindowClassName, wnd_title, wstyle,
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left,
		rect.bottom - rect.top, HWND_DESKTOP, nullptr,
		::GetModuleHandleW(nullptr), nullptr);
}
#endif

} // unnamed namespace;


DSApplication::DSApplication()
	: GUIApplication(),
	scrs()
#if YCL_MINGW32
	, p_wnd_thrd()
#endif
{
	using Devices::DSScreen;

	YAssert(!YSLib::pApp, "Duplicate instance found.");

	//注册全局应用程序实例。
	YSLib::pApp = this;
	//初始化系统设备。
#if YCL_DS
	InitVideo();
#endif
	Devices::InitDSScreen(scrs[0], scrs[1]);
#if YCL_DS
	scrs[0]->Update(ColorSpace::Blue),
	scrs[1]->Update(ColorSpace::Green);
#elif YCL_MINGW32

	using namespace Host;

	YAssert(IsScreenReady(), "Screen is not ready.");

	p_wnd_thrd.reset(new WindowThread([this]{
		return unique_ptr<Window>(new DSWindow(InitializeMainWindow(
			L"YSTest", 256, 384), *scrs[0], *scrs[1], GetHost()));
	}));
	// FIXME: Reduce possible data race.
	while(!p_wnd_thrd->GetWindowPtr())
		// TODO: Resolve magic sleep duration.
		std::this_thread::sleep_for(host_sleep);

	const auto h_wnd(p_wnd_thrd->GetWindowPtr()->GetNativeHandle());

	yunseq(scrs[0]->WindowHandle = h_wnd, scrs[1]->WindowHandle = h_wnd);
#endif
}

DSApplication::~DSApplication()
{
#if YCL_MINGW32
	p_wnd_thrd.reset();
	YCL_DEBUG_PUTS("Host thread dropped.");
#endif
	//等待并确保所有 Shell 被释放。
//	hShell = nullptr;
	//释放全局非静态资源。
	//当主 Shell 句柄为静态存储期对象时需要通过 reset 释放。
	//释放设备。
	reset(scrs[0]),
	reset(scrs[1]);
}

Devices::DSScreen&
DSApplication::GetDSScreenUp() const ynothrow
{
	YAssert(bool(scrs[0]), "Null pointer found.");

	return *scrs[0];
}
Devices::DSScreen&
DSApplication::GetDSScreenDown() const ynothrow
{
	YAssert(bool(scrs[1]), "Null pointer found.");

	return *scrs[1];
}
Devices::Screen&
DSApplication::GetScreenUp() const ynothrow
{
	return GetDSScreenUp();
}
Devices::Screen&
DSApplication::GetScreenDown() const ynothrow
{
	return GetDSScreenDown();
}


bool
#if YCL_DS
InitConsole(Devices::Screen& scr, Drawing::PixelType fc, Drawing::PixelType bc)
{
	using namespace platform;

	if(&FetchGlobalInstance<DSApplication>().GetScreenUp() == &scr)
		YConsoleInit(true, fc, bc);
	else if(&FetchGlobalInstance<DSApplication>().GetScreenDown() == &scr)
		YConsoleInit(false, fc, bc);
	else
		return false;
#elif YCL_MINGW32
InitConsole(Devices::Screen&, Drawing::PixelType, Drawing::PixelType)
{
#else
#	error Unsupported platform found!
#endif
	return true;
}

void
ShowFatalError(const char* s)
{
	using namespace platform;

	YDebugSetStatus();
	YDebugBegin();
	std::printf("Fatal Error:\n%s\n", s);
	terminate();
}

#if YCL_MINGW32
YSL_BEGIN_NAMESPACE(MinGW32)

void
TestFramework(size_t idx)
{
	YCL_DEBUG_PUTS(("Test beginned, idx = " + to_string(idx) + " .").c_str());
	YCL_DEBUG_PUTS("Test ended.");
}

YSL_END_NAMESPACE(MinGW32)
#endif

YSL_END

