﻿/*
	Copyright (C) by Franksoft 2009 - 2011.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ywidget.cpp
\ingroup Shell
\brief 平台无关的图形用户界面部件实现。
\version 0.4852;
\author FrankHB<frankhb1989@gmail.com>
\par 创建时间:
	2009-11-16 20:06:58 + 08:00;
\par 修改时间:
	2011-01-04 23:55 + 08:00;
\par 字符集:
	UTF-8;
\par 模块名称:
	YSLib::Shell::YWidget;
*/


#include "ydesktop.h"

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Components)

#ifdef YCL_USE_YASSERT

void
yassert(bool exp, const char* msg, int line, const char* file,
	const char* comp, const char* func)
{
	if(!exp)
	{
		YConsole dbg;

		iprintf(
			"At line %i in file %s: \n"
			"An error occured in precedure %s of \n"
			"Components::%s:\n"
			"%s", line, file, func, comp, msg);
		dbg.Pause();
	}
}

#endif

YSL_BEGIN_NAMESPACE(Widgets)

using Controls::YVisualControl;

bool
Contains(const IWidget& w, SPOS x, SPOS y)
{
	return Rect(w.GetLocation(), w.GetSize()).Contains(x, y);
}

bool
ContainsVisible(const IWidget& w, SPOS x, SPOS y)
{
	return w.IsVisible() && Contains(w, x, y);
}


Rect
GetBoundsOf(IWidget& w)
{
	return Rect(w.GetLocation(), w.GetSize());
}

void
SetBoundsOf(IWidget& w, const Rect& r)
{
	w.SetLocation(r.GetPoint());
	w.SetSize(r.GetSize());
}


IUIBox*
FetchDirectContainerPtr(IWidget& w)
{
	IUIBox* const pCon(dynamic_cast<IUIBox*>(&w));

	return pCon ? pCon : w.GetContainerPtr();
}


namespace
{
	void
	GetContainersListFrom(IWidget& w, map<IUIBox*, Point>& m)
	{
		Point pt;
		IUIBox* pCon(dynamic_cast<IUIBox*>(&w));

		if(!pCon)
		{
			pCon = w.GetContainerPtr();
			pt = w.GetLocation();
		}
		while(pCon != NULL)
		{
			m.insert(std::make_pair(pCon, pt));
			pt += pCon->GetLocation();
			pCon = pCon->GetContainerPtr();
		}
	}
}

Point
LocateOffset(const IWidget* pCon, Point p, const IWindow* pWnd)
{
	while(pCon && dynamic_cast<const IWindow*>(pCon) != pWnd)
	{
		p += pCon->GetLocation();
		pCon = dynamic_cast<const IWidget*>(pCon->GetContainerPtr());
	}
	return p;
}

Point
LocateForWidget(IWidget& a, IWidget& b)
{
	map<IUIBox*, Point> m;

	GetContainersListFrom(b, m);

	Point pt;
	IUIBox* pCon(dynamic_cast<IUIBox*>(&a));

	if(!pCon)
	{
		pCon = a.GetContainerPtr();
		pt = a.GetLocation();
	}

	while(pCon != NULL)
	{
		map<IUIBox*, Point>::const_iterator i(m.find(pCon));

		if(i != m.end())
			return pt - i->second;
		pt += pCon->GetLocation();
		pCon = pCon->GetContainerPtr();
	}
	return Point::FullScreen;
}

Point
LocateForWindow(IWidget& w)
{
	if(dynamic_cast<IWindow*>(&w))
		return Point::Zero;

	const IWindow* pWnd(FetchDirectWindowPtr(w));

	return pWnd ? LocateOffset(&w, Point::Zero, pWnd) : Point::FullScreen;
}

Point
LocateForDesktop(IWidget& w)
{
	if(dynamic_cast<YDesktop*>(&w))
		return Point::Zero;

	YDesktop* pDsk(FetchDirectDesktopPtr(w));

	return pDsk ? LocateOffset(&w, Point::Zero, pDsk) : Point::FullScreen;
}

Point
LocateForParentContainer(const IWidget& w)
{
	return w.GetContainerPtr()
		? LocateContainerOffset(*w.GetContainerPtr(),
		w.GetLocation()) : Point::FullScreen;
}

Point
LocateForParentWindow(const IWidget& w)
{
	const IWindow* pWnd(FetchWindowPtr(w));

	return pWnd ? LocateWindowOffset(*pWnd, w.GetLocation())
		: Point::FullScreen;
}


void
MoveToLeft(IWidget& w)
{
	YAssert(w.GetContainerPtr(),
		"In function \"void\n"
		"ATrack::MoveToLeft(IWidget& w)\": \n"
		"Container pointer of the widget is null.");

	w.SetLocation(Point(0, w.GetLocation().Y));
}

void MoveToRight(IWidget& w)
{
	YAssert(w.GetContainerPtr(),
		"In function \"void\n"
		"ATrack::MoveToLeft(IWidget& w)\": \n"
		"Container pointer of the widget is null.");

	w.SetLocation(Point(w.GetContainerPtr()->GetSize().Width
		- w.GetSize().Width, w.GetLocation().Y));
}

void MoveToTop(IWidget& w)
{
	YAssert(w.GetContainerPtr(),
		"In function \"void\n"
		"ATrack::MoveToLeft(IWidget& w)\": \n"
		"Container pointer of the widget is null.");

	w.SetLocation(Point(w.GetLocation().X, 0));
}

void MoveToBottom(IWidget& w)
{
	YAssert(w.GetContainerPtr(),
		"In function \"void\n"
		"ATrack::MoveToLeft(IWidget& w)\": \n"
		"Container pointer of the widget is null.");

	w.SetLocation(Point(w.GetLocation().X,
		w.GetContainerPtr()->GetSize().Height - w.GetSize().Height));
}


void
Fill(IWidget& w, Color c)
{
	IWindow* pWnd(FetchDirectWindowPtr(w));

	if(pWnd)
	{
		const Graphics& g(pWnd->GetContext());

		FillRect(g, LocateOffset(&w, Point::Zero, pWnd), w.GetSize(), c);
	}
}


Visual::Visual(const Rect& r, Color b, Color f)
	: visible(true), transparent(false), background_redrawed(false),
	location(r.GetPoint()), size(r.Width, r.Height),
	BackColor(b), ForeColor(f)
{}

void
Visual::SetSize(const Size& s)
{
	if(size != s)
	{
		size = s;
		background_redrawed = false;
	}
}


Widget::Widget(const Rect& r, IUIBox* pCon, Color b, Color f)
	: Visual(r, b, f),
	pContainer(pCon)
{}

void
Widget::DrawBackground()
{
	YWidgetAssert(this, Widgets::Widget, DrawBackground);

	if(!IsTransparent())
		Fill(*ystdex::polymorphic_crosscast<IWidget*>(this), BackColor);
}

void
Widget::DrawForeground()
{
	YWidgetAssert(this, Widgets::Widget, DrawForeground);

	if(!IsTransparent())
		SetBgRedrawed(false);
}

void
Widget::Refresh()
{
	IWindow* pWnd(FetchWindowPtr(
		*ystdex::polymorphic_crosscast<IWidget*>(this)));

	while(pWnd && !pWnd->IsRefreshRequired())
	{
		pWnd->SetRefresh(true);
		pWnd = FetchWindowPtr(*pWnd);
	}
}


YWidget::YWidget(const Rect& r, IUIBox* pCon)
	: YComponent(),
	Widget(r, pCon)
{
	IUIContainer* const p(dynamic_cast<IUIContainer*>(GetContainerPtr()));

	if(p)
		*p += static_cast<IWidget&>(*this);
}
YWidget::~YWidget() ythrow()
{
	IUIContainer* const p(dynamic_cast<IUIContainer*>(GetContainerPtr()));

	if(p)
		*p -= static_cast<IWidget&>(*this);
}


MUIContainer::MUIContainer()
	: GMFocusResponser<IVisualControl>(),
	sWgtSet(), sFOCSet()
{}

IVisualControl*
MUIContainer::GetFocusingPtr() const
{
	return GMFocusResponser<IVisualControl>::GetFocusingPtr();
}
IWidget*
MUIContainer::GetTopWidgetPtr(const Point& pt)
{
	for(WidgetSet::const_iterator i(sWgtSet.begin()); i != sWgtSet.end(); ++i)
		if(Contains(**i, pt))
			return *i;
	return NULL;
}
IVisualControl*
MUIContainer::GetTopVisualControlPtr(const Point& pt)
{
	for(FOs::const_iterator i(sFOs.begin()); i != sFOs.end(); ++i)
	{
		if(*i && Contains(**i, pt))
			return *i;
	}
	return NULL;
}

bool
MUIContainer::ResponseFocusRequest(AFocusRequester& w)
{
	return w.RequestFocus<GMFocusResponser, IVisualControl>(*this);
}

bool
MUIContainer::ResponseFocusRelease(AFocusRequester& w)
{
	return w.ReleaseFocus<GMFocusResponser, IVisualControl>(*this);
}


YUIContainer::YUIContainer(const Rect& r, IUIBox* pCon)
	: YComponent(),
	Widget(r, pCon), MUIContainer()
{
	IUIContainer* p(dynamic_cast<IUIContainer*>(GetContainerPtr()));

	if(p)
	{
		*p += static_cast<IWidget&>(*this);
		*p += static_cast<GMFocusResponser<IVisualControl>&>(*this);
	}
}
YUIContainer::~YUIContainer() ythrow()
{
	IUIContainer* p(dynamic_cast<IUIContainer*>(GetContainerPtr()));

	if(p)
	{
		*p -= static_cast<IWidget&>(*this);
		*p -= static_cast<GMFocusResponser<IVisualControl>&>(*this);
	}
}


MLabel::MLabel(const Drawing::Font& f, GHWeak<Drawing::TextRegion> pTr_)
	: wpTextRegion(pTr_ ? pTr_ : GetGlobalResource<Drawing::TextRegion>()),
	Font(f), Margin(wpTextRegion->Margin), AutoSize(true), AutoEllipsis(false),
	Text()
{}

void
MLabel::PaintText(Widget& w, const Point& pt)
{
	IWindow* pWnd(FetchDirectWindowPtr(
		ystdex::polymorphic_crosscast<IWidget&>(w)));

	if(pWnd && wpTextRegion)
	{
		wpTextRegion->Font = Font;
		wpTextRegion->Font.Update();
		wpTextRegion->ResetPen();
		wpTextRegion->Color = w.ForeColor;
		wpTextRegion->SetSize(w.GetWidth(), w.GetHeight());
		SetMarginsTo(*wpTextRegion, 2, 2, 2, 2);
		wpTextRegion->PutLine(Text);

		const Graphics& g(pWnd->GetContext());

		wpTextRegion->BlitTo(g.GetBufferPtr(), g.GetSize(), Point::Zero,
			pt, w.GetSize());
		wpTextRegion->SetSize(0, 0);
	}
}


YLabel::YLabel(const Rect& r, IUIBox* pCon, const Drawing::Font& f,
	GHWeak<Drawing::TextRegion> pTr_)
	: YWidget(r, pCon), MLabel(f, pCon
	? pTr_ : GetGlobalResource<Drawing::TextRegion>())
{}

void
YLabel::DrawForeground()
{
	YWidgetAssert(this, Widgets::YLabel, DrawForeground);

	ParentType::DrawForeground();
	PaintText(*this, LocateForWindow(*this));
}

YSL_END_NAMESPACE(Widgets)

YSL_END_NAMESPACE(Components)

YSL_END
