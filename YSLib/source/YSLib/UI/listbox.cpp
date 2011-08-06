﻿/*
	Copyright (C) by Franksoft 2011.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file listbox.cpp
\ingroup UI
\brief 样式相关的图形用户界面列表框控件。
\version 0.3656;
\author FrankHB<frankhb1989@gmail.com>
\par 创建时间:
	2011-03-07 20:33:05 +0800;
\par 修改时间:
	2011-07-31 20:40 +0800;
\par 字符集:
	UTF-8;
\par 模块名称:
	YSLib::UI::ListBox;
*/


#include "listbox.h"
#include "ystyle.h"
#include "../Service/ytext.h"

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Components)

YSL_BEGIN_NAMESPACE(Controls)

namespace
{
	const SDst defMinScrollBarWidth(16); //!< 默认最小滚动条宽。
//	const SDst defMinScrollBarHeight(16); //!< 默认最小滚动条高。
}


ListBox::ListBox(const Rect& r, const shared_ptr<ListType>& h)
	: ScrollableContainer(r),
	TextListBox(Rect(Point::Zero, r), h)
{
	TextListBox.GetContainerPtrRef() = this;
	VerticalScrollBar.GetTrack().GetScroll().Add(*this,
		&ListBox::OnScroll_VerticalScrollBar);
	TextListBox.GetViewChanged().Add(*this,
		&ListBox::OnViewChanged_TextListBox);
	//刷新文本状态，防止第一次绘制时无法正确决定是否需要滚动条。
	TextListBox.RefreshTextState();
}

IControl*
ListBox::GetTopControlPtr(const Point& pt)
{
	IControl* pCon(ScrollableContainer::GetTopControlPtr(pt));

	if(pCon == this)
		return &TextListBox;
	return pCon;
}

void
ListBox::DrawControl(const Graphics& g, const Point& pt, const Rect& r)
{
	ScrollableContainer::DrawControl(g, pt, r);
	RenderChild(TextListBox, g, pt, r);
}

void
ListBox::OnScroll_VerticalScrollBar(ScrollEventArgs&& e)
{
	TextListBox.LocateViewPosition(e.Value);
	Invalidate(*this);
}

void
ListBox::OnViewChanged_TextListBox(EventArgs&&)
{
	if(GetWidth() > defMinScrollBarWidth)
	{
		Size view_arena(TextListBox.GetFullViewSize());

		view_arena.Width = GetWidth() - defMinScrollBarWidth;
		TextListBox.SetSize(FixLayout(view_arena));
		VerticalScrollBar.SetSmallDelta(TextListBox.GetItemHeight());
		VerticalScrollBar.SetMaxValue(view_arena.Height);
		VerticalScrollBar.SetLargeDelta(TextListBox.GetHeight());
		VerticalScrollBar.SetValue(TextListBox.GetViewPosition());
	}
}


FileBox::FileBox(const Rect& r)
	: FileList(), ListBox(r, GetListPtr())
{
	GetConfirmed().Add(*this, &FileBox::OnConfirmed);
	ListItems();
	UpdateView();
}

IO::Path
FileBox::GetPath() const
{
	if(IsSelected() && GetSelectedIndex() >= 0)
		return Directory / (GetList()[GetSelectedIndex()]);
	return Directory;
}

void
FileBox::OnConfirmed(IndexEventArgs&& e)
{
	if(Contains(e) && static_cast<bool>(*this /= GetList()[e.Index]))
	{
		ListItems();
		ResetView();
	}
}

YSL_END_NAMESPACE(Controls)

YSL_END_NAMESPACE(Components)

YSL_END
