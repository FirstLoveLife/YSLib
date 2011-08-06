﻿/*
	Copyright (C) by Franksoft 2010 - 2011.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\defgroup UI UI
\ingroup YSLib
\brief YSLib UI 模块。
*/

/*!	\file ycomp.h
\ingroup UI
\brief 平台无关的 Shell 组件。
\version 0.3129;
\author FrankHB<frankhb1989@gmail.com>
\par 创建时间:
	2010-03-19 20:05:08 +0800;
\par 修改时间:
	2011-08-05 04:15 +0800;
\par 字符集:
	UTF-8;
\par 模块名称:
	YSLib::UI::YComponent;
*/


#ifndef YSL_INC_UI_YCOMP_H_
#define YSL_INC_UI_YCOMP_H_

#include "../Core/yshell.h"
#include "../Core/yapp.h"
#include "../Helper/yglobal.h"

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Components)

YSL_BEGIN_NAMESPACE(Controls)
PDeclInterface(IControl)
YSL_END_NAMESPACE(Controls)

YSL_BEGIN_NAMESPACE(Forms)
class AWindow;
class AFrame;
class Frame;
YSL_END_NAMESPACE(Forms)

YSL_BEGIN_NAMESPACE(Widgets)
PDeclInterface(IWidget)
YSL_END_NAMESPACE(Widgets)

YSL_END_NAMESPACE(Components)

using Components::Controls::IControl;
using Components::Widgets::IWidget;

YSL_END

#endif
