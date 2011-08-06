﻿/*
	Copyright (C) by Franksoft 2010 - 2011.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ydesktop.cpp
\ingroup UI
\brief 平台无关的桌面抽象层。
\version 0.2336;
\author FrankHB<frankhb1989@gmail.com>
\par 创建时间:
	2010-05-02 12:00:08 +0800;
\par 修改时间:
	2011-08-02 02:44 +0800;
\par 字符集:
	UTF-8;
\par 模块名称:
	YSLib::UI::Desktop;
*/


#include "ydesktop.h"

YSL_BEGIN

using namespace Drawing;

YSL_BEGIN_NAMESPACE(Components)

using namespace Controls;

Desktop::Desktop(Devices::Screen& s, Color c, const shared_ptr<Image>& hImg)
	: Frame(Rect::FullScreen, hImg),
	Screen(s)
{
	BackColor = c;
}

IControl*
Desktop::GetTopVisibleDesktopObjectPtr(const Point& pt) const
{
	for(auto i(sWidgets.crbegin()); i != sWidgets.crend(); ++i)
	{
		try
		{
			if(i->second->IsVisible() && Widgets::Contains(*i->second, pt))
				return &dynamic_cast<IControl&>(*i->second);
		}
		catch(std::bad_cast&)
		{}
	}
	return nullptr;
}

bool
Desktop::MoveToTop(IControl& ctl)
{
	auto i(std::find_if(sWidgets.begin(), sWidgets.end(),
		[&](const WidgetMap::value_type& val){
		return val.second == &ctl;
	}));

	if(i != sWidgets.end())
	{
		const Widgets::ZOrderType z(i->first);

		sWidgets.erase(i);
		sWidgets.insert(make_pair(z, static_cast<IWidget*>(&ctl)));
		SetInvalidation();
		return true;
	}
	return false;
}

/*void
Desktop::RemoveTopDesktopObject()
{
	if(!sDOs.empty())
	{
		sDOs.back()->ReleaseFocus();
		sDOs.pop_back();
		bRefresh = true;
	}
}*/

void
Desktop::ClearContents()
{
	ClearFocusingPtr();
	sWidgets.clear();
	sFocusContainers.clear();
	SetInvalidation();
}

void
Desktop::Update()
{
	if(!GetRenderer().RequiresRefresh())
		Screen.Update(FetchContext(*this).GetBufferPtr());
}

YSL_END_NAMESPACE(Components)

YSL_END
