﻿/*
	© 2013-2015 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file Environment.cpp
\ingroup Helper
\brief 环境。
\version r1531
\author FrankHB <frankhb1989@gmail.com>
\since build 379
\par 创建时间:
	2013-02-08 01:27:29 +0800
\par 修改时间:
	2015-01-23 01:17 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	Helper::Environment
*/


#include "Helper/YModules.h"
#include YFM_Helper_Environment
#include YFM_Helper_ShellHelper // for YSL_DEBUG_DECL_TIMER;
#include YFM_YCLib_Input // for platform_ex::FetchCursor;
#include YFM_Helper_Initialization // for InitializeEnvironment;

namespace YSLib
{

using namespace Drawing;

namespace
{

#if YCL_Win32 && 0
yconstexpr double g_max_free_fps(1000);
std::chrono::nanoseconds host_sleep(std::uint64_t(1000000000 / g_max_free_fps));
#endif

} // unnamed namespace;


#if YF_Hosted
using namespace Host;
#endif

namespace
{

#if YCL_Win32
::LRESULT CALLBACK
WndProc(::HWND h_wnd, unsigned msg, ::WPARAM w_param, ::LPARAM l_param)
{
	if(const auto p = reinterpret_cast<Window*>(::GetWindowLongPtrW(h_wnd,
		GWLP_USERDATA)))
	{
		YSL_DEBUG_DECL_TIMER(tmr, std::to_string(msg));
		auto& m(p->MessageMap);
		const auto i(m.find(msg));

		if(i != m.cend())
		{
			i->second(w_param, l_param);
			return 0;
		}
	}
	return ::DefWindowProcW(h_wnd, msg, w_param, l_param);
}
#endif

} // unnamed namespace;


Environment::Environment()
#if YF_Hosted
	: wnd_map(), wmap_mtx()
#	if YF_Multithread == 1
#		if YCL_Win32
	, window_class(WindowClassName, WndProc)
#		endif
#	endif
#endif
{
#if YCL_Win32
	TryExpr(FixConsoleHandler())
	CatchExpr(Win32Exception&,
		YTraceDe(Warning, "Console handler setup failed."))
#endif
	InitializeEnvironment();
	YCL_Trace(Debug, "Environment lifetime began.");
}
Environment::~Environment()
{
	YCL_Trace(Debug, "Environment lifetime ended.");

#	if YF_Hosted && !YCL_Android
	using ystdex::get_value;

	std::for_each(wnd_map.cbegin() | get_value, wnd_map.cend() | get_value,
		[](Window* const& p){
			p->Close();
	});
#	endif
	Uninitialize();
}

#if YF_Hosted
Window*
Environment::GetForegroundWindow() const ynothrow
{
#	ifdef YCL_Win32
	return FindWindow(::GetForegroundWindow());
#	else
	return {};
#	endif
}

void
Environment::AddMappedItem(NativeWindowHandle h, Window* p)
{
	lock_guard<mutex> lck(wmap_mtx);

	wnd_map.emplace(h, p);
}

Window*
Environment::FindWindow(NativeWindowHandle h) const ynothrow
{
	lock_guard<mutex> lck(wmap_mtx);
	const auto i(wnd_map.find(h));

	return i == wnd_map.end() ? nullptr : i->second;
}

#	if YF_Multithread == 1
void
Environment::LeaveWindowThread()
{
	if(--wnd_thrd_count == 0 && ExitOnAllWindowThreadCompleted)
		YSLib::PostQuitMessage(0);
}
#	endif

pair<Host::Window*, Point>
Environment::MapCursor(Host::Window* p_wnd) const
{
#	if YCL_Win32
	::POINT cursor;

	if(YB_UNLIKELY(!::GetCursorPos(&cursor)))
		YF_Raise_Win32Exception("GetCursorPos @ Environment::MapCursor");
	if(!p_wnd)
		if(const auto h = ::ChildWindowFromPointEx(::GetDesktopWindow(),
			cursor, CWP_SKIPINVISIBLE))
			p_wnd = FindWindow(h);
	if(!p_wnd)
		return {p_wnd, Point::Invalid};
	if(YB_UNLIKELY(!::ScreenToClient(p_wnd->GetNativeHandle(), &cursor)))
		YF_Raise_Win32Exception("ScreenToClient @ Environment::MapCursor");
	return {p_wnd, p_wnd->MapPoint({cursor.x, cursor.y})};
#	elif YCL_Android
	// TODO: Support floating point coordinates.
	const auto& cursor(platform_ex::FetchCursor());
	const Point pt(cursor.first, cursor.second);

	return {p_wnd, MapPoint ? MapPoint(pt) : pt};
#	else
	return {p_wnd, Point::Invalid};
#	endif
}

void
Environment::RemoveMappedItem(NativeWindowHandle h) ynothrow
{
	lock_guard<mutex> lck(wmap_mtx);
	const auto i(wnd_map.find(h));

	if(i != wnd_map.end())
		wnd_map.erase(i);
}

void
Environment::UpdateRenderWindows()
{
	lock_guard<mutex> lck(wmap_mtx);

	for(const auto& pr : wnd_map)
		if(pr.second)
			pr.second->Refresh();
}
#endif

} // namespace YSLib;

