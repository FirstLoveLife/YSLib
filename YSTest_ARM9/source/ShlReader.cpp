﻿/*
	Copyright by FrankHB 2011 - 2013.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ShlReader.cpp
\ingroup YReader
\brief Shell 阅读器框架。
\version r4224
\author FrankHB <frankhb1989@gmail.com>
\since build 263
\par 创建时间:
	2011-11-24 17:13:41 +0800
\par 修改时间:
	2013-04-12 12:20 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YReader::ShlReader
*/


#include "ShlReader.h"
#include "ShlExplorer.h"

YSL_BEGIN_NAMESPACE(YReader)

namespace
{
//	ResourceMap GlobalResourceMap;

using namespace Text;

/*
\brief 文本阅读器菜单项。
\since build 303
*/
enum MNU_READER : Menu::IndexType
{
	MR_Return = 0,
	MR_Setting,
	MR_FileInfo,
	//! \since build 391
	MR_Bookmark,
	MR_LineUp,
	MR_LineDown,
	MR_ScreenUp,
	MR_ScreenDown
};

} // unnamed namespace;


ReaderBox::ReaderBox(const Rect& r)
	: Control(r),
	btnMenu(Rect(4, 12, 16, 16)), btnSetting(Rect(24, 12, 16, 16)),
	btnInfo(Rect(44, 12, 16, 16)), btnBookmark(Rect(64, 12, 16, 16)),
	btnReturn(Rect(84, 12, 16, 16)),
	btnPrev(Rect(104, 12, 16, 16)), btnNext(Rect(124, 12, 16, 16)),
	pbReader(Rect(4, 0, 248, 8)), lblProgress(Rect(216, 12, 40, 16))
{
	Background = nullptr,
	SetRenderer(make_unique<BufferedRenderer>()),
	unseq_apply(ContainerSetter(*this), btnMenu, btnSetting, btnInfo,
		btnBookmark, btnReturn, btnPrev, btnNext, pbReader, lblProgress);
	SetBufferRendererAndText(btnMenu, u"M"),
	SetBufferRendererAndText(btnSetting, u"S"),
	SetBufferRendererAndText(btnInfo, u"I"),
	SetBufferRendererAndText(btnBookmark, u"B"),
	SetBufferRendererAndText(btnReturn, u"R"),
	SetBufferRendererAndText(btnPrev, u"←"),
	SetBufferRendererAndText(btnNext, u"→");
	pbReader.ForeColor = Color(192, 192, 64),
	lblProgress.SetRenderer(make_unique<BufferedRenderer>()),
	lblProgress.Background = nullptr,
	lblProgress.Font.SetSize(12),
	InitializeProgress();
}

void
ReaderBox::InitializeProgress()
{
	yunseq(lblProgress.Text = u"--%", lblProgress.ForeColor = ColorSpace::Blue);
}

void
ReaderBox::UpdateData(DualScreenReader& reader)
{
	if(YB_LIKELY(reader.IsBufferReady()))
	{
		const auto ts(reader.GetTextSize());

		if(YB_LIKELY(ts != 0))
		{
			const auto tp(reader.GetTopPosition());
			char str[5];

			std::sprintf(str, "%2u%%", tp * 100 / ts);
			yunseq(lblProgress.Text = str,
				lblProgress.ForeColor = reader.GetBottomPosition() == ts
				? ColorSpace::Green : ColorSpace::Fuchsia);
			pbReader.SetMaxValue(ts),
			pbReader.SetValue(tp);
		}
		else
			InitializeProgress();
	}
	else
		InitializeProgress();
	Invalidate(pbReader),
	Invalidate(lblProgress);
}


TextInfoBox::TextInfoBox()
	: DialogBox(Rect(32, 32, 200, 108)),
	lblEncoding(Rect(4, 20, 192, 18)),
	lblSize(Rect(4, 40, 192, 18)),
	lblTop(Rect(4, 60, 192, 18)),
	lblBottom(Rect(4, 80, 192, 18))
{
	unseq_apply(ContainerSetter(*this), lblEncoding, lblSize);
	FetchEvent<TouchMove>(*this) += OnTouchMove_Dragging;
}

void
TextInfoBox::Refresh(PaintEventArgs&& e)
{
	DialogBox::Refresh(std::move(e));

	unseq_apply(ChildPainter(e), lblEncoding, lblSize, lblTop, lblBottom);
	e.ClipArea = Rect(e.Location, GetSizeOf(*this));
}

void
TextInfoBox::UpdateData(DualScreenReader& reader)
{
	yunseq(lblEncoding.Text = "Encoding: " + to_string(reader.GetEncoding())
		+ ';',
		lblSize.Text = "Size: " + to_string(reader.GetTextSize()) + " B;",
		lblTop.Text = "Top: " + to_string(reader.GetTopPosition()) + " B;",
		lblBottom.Text = "Bottom: " + to_string(reader.GetBottomPosition())
		+ " B;");
	Invalidate(lblEncoding),
	Invalidate(lblSize);
}


FileInfoPanel::FileInfoPanel()
	: Panel(Rect({}, MainScreenWidth, MainScreenHeight)),
	lblPath(Rect(8, 20, 240, 16)),
	lblSize(Rect(8, 40, 240, 16)),
	lblAccessTime(Rect(8, 60, 240, 16)),
	lblModifiedTime(Rect(8, 80, 240, 16)),
	lblOperations(Rect(8, 120, 240, 16))
{
	Background = SolidBrush(ColorSpace::Silver);
#if YCL_DS
	lblOperations.Text = "<↑↓> 滚动一行 <LR> 滚动一屏 <B>退出";
#else
	lblOperations.Text = "<↑↓> 滚动一行 <LR> 滚动一屏 <Esc>退出";
#endif
	AddWidgets(*this, lblPath, lblSize, lblAccessTime, lblModifiedTime,
		lblOperations);
}


ShlReader::ShlReader(const IO::Path& pth,
	const shared_ptr<Desktop>& h_dsk_up, const shared_ptr<Desktop>& h_dsk_dn)
	: ShlDS(h_dsk_up, h_dsk_dn),
	CurrentPath(pth), fBackgroundTask(), bExit()
{}

void
ShlReader::Exit()
{
	if(bExit)
		return;
	bExit = true;
	fBackgroundTask = nullptr;
	// TODO: Use template %SetShellToNew.
//	SetShellToNew<ShlExplorer>();
	const auto h_up(GetDesktopUpHandle());
	const auto h_dn(GetDesktopDownHandle());

	PostMessage<SM_TASK>(0xF8, [=]{
		ResetDSDesktops(*h_up, *h_dn);
		NowShellTo(ystdex::make_shared<ShlExplorer>(CurrentPath / u"..",
			h_up, h_dn));
	});
}

ReaderSetting
ShlReader::LoadGlobalConfiguration()
{
	try
	{
		// TODO: Move node instead of copying.
		FetchGlobalInstance().Root /= LoadConfiguration().GetNode("YReader");
		return ReaderSetting(FetchGlobalInstance().Root.GetNode("YReader")
			.GetNode("ReaderSetting"));
	}
	catch(std::exception& e) // TODO: Logging.
	{}
	return ReaderSetting();
}

void
ShlReader::OnInput()
{
	PostMessage<SM_PAINT>(0xE0, nullptr);
	if(fBackgroundTask)
		PostMessage<SM_TASK>(0x20, fBackgroundTask);
}

void
ShlReader::SaveGlobalConfiguration(const ReaderSetting& rs)
{
	try
	{
		FetchGlobalInstance().Root["YReader"] /= ValueNode(rs);
		SaveConfiguration(FetchGlobalInstance().Root);
	}
	catch(std::exception& e) // TODO: Logging.
	{}
}


ShlTextReader::BaseSession::BaseSession(ShlTextReader& shl)
	: GShellSession<ShlTextReader>(shl)
{
	shl.StopAutoScroll(),
	Hide(shl.boxReader),
	Hide(shl.boxTextInfo);
}
ShlTextReader::BaseSession::~BaseSession()
{
	auto& shl(GetShell());

	shl.reader.SetVisible(true),
	shl.boxReader.UpdateData(shl.reader),
	shl.boxTextInfo.UpdateData(shl.reader),
	Show(shl.boxReader);
}


ShlTextReader::SettingSession::SettingSession(ShlTextReader& shl)
	: BaseSession(shl)
{
	auto& dsk_up(shl.GetDesktopUp());
	auto& dsk_dn(shl.GetDesktopDown());
	auto& reader(shl.reader);
	auto& CurrentSetting(shl.CurrentSetting);
	auto& pnlSetting(shl.pnlSetting);

	shl.reader.SetVisible(false),
	yunseq(CurrentSetting.UpColor = dsk_up.Background
		.target<SolidBrush>()->Color, CurrentSetting.DownColor
		= dsk_dn.Background.target<SolidBrush>()->Color,
		CurrentSetting.FontColor = reader.GetColor(),
		CurrentSetting.Font = reader.GetFont());
	AddWidgets(dsk_up, pnlSetting.lblAreaUp, pnlSetting.lblAreaDown);
	{
		using ystdex::get_key;

		size_t i(std::find(Encodings | get_key,
			(Encodings + arrlen(Encodings)) | get_key,
			reader.GetEncoding()) - Encodings);

		if(i == arrlen(Encodings))
			i = 0;
		yunseq(pnlSetting.lblAreaDown.Text = FetchEncodingString(i),
			pnlSetting.ddlEncoding.Text = Encodings[i].second);
	}
	Show(pnlSetting << CurrentSetting);
}
ShlTextReader::SettingSession::~SettingSession()
{
	auto& shl(GetShell());

	RemoveWidgets(shl.GetDesktopUp(),
		shl.pnlSetting.lblAreaUp, shl.pnlSetting.lblAreaDown);
}


ShlTextReader::BookmarkSession::BookmarkSession(ShlTextReader& shl)
	: BaseSession(shl)
{
	Show(shl.pnlBookmark);
}


ShlTextReader::ShlTextReader(const IO::Path& pth,
	const shared_ptr<Desktop>& h_dsk_up, const shared_ptr<Desktop>& h_dsk_dn)
	: ShlReader(pth, h_dsk_up, h_dsk_dn),
	LastRead(ystdex::parameterize_static_object<ReadingList>()),
	CurrentSetting(LoadGlobalConfiguration()), bookmarks(), tmrScroll(
	CurrentSetting.GetTimerSetting()), tmrInput(), reader(),
	boxReader(Rect(0, 160, 256, 32)), boxTextInfo(), pnlSetting(),
	pTextFile(), mhMain(GetDesktopDown()), pnlBookmark(bookmarks, *this),
	session_ptr()
{
	using ystdex::get_key;

	const auto exit_session([this](TouchEventArgs&&){
		session_ptr.reset();
	});

	SetVisibleOf(boxReader, false),
	SetVisibleOf(boxTextInfo, false),
	SetVisibleOf(pnlSetting, false),
	SetVisibleOf(pnlBookmark, false);
	yunseq(
		reader.ViewChanged = [this]{
			if(IsVisible(boxReader))
				boxReader.UpdateData(reader);
			if(IsVisible(boxTextInfo))
				boxTextInfo.UpdateData(reader);
		},
		mhMain.Roots[&boxReader.btnMenu] = 1U,
		FetchEvent<Click>(boxReader.btnMenu) += [this](TouchEventArgs&&){
			if(mhMain.IsShowing(1U))
				mhMain.Hide(1U);
			else
			{
				const auto& pt(LocateForWidget(GetDesktopDown(),
					boxReader.btnMenu));

				ShowMenu(1U, Point(pt.X, pt.Y - mhMain[1U].GetHeight()));
			}
		},
		FetchEvent<Click>(boxReader.btnSetting) += [this](TouchEventArgs&&){
			Execute(MR_Setting);
		},
		FetchEvent<Click>(boxReader.btnInfo) += [this](TouchEventArgs&&){
			Execute(MR_FileInfo);
		},
		FetchEvent<Click>(boxReader.btnBookmark) += [this](TouchEventArgs&&){
			Execute(MR_Bookmark);
		},
		FetchEvent<Click>(boxReader.btnReturn) += [this](TouchEventArgs&&){
			Execute(MR_Return);
		},
		FetchEvent<Click>(boxReader.btnPrev) += [this](TouchEventArgs&&){
			UpdateReadingList(true);
		},
		FetchEvent<Click>(boxReader.btnNext) += [this](TouchEventArgs&&){
			UpdateReadingList(false);
		},
		FetchEvent<TouchDown>(boxReader.pbReader) += [this](TouchEventArgs&& e){
			const auto s(reader.GetTextSize());

			if(YB_LIKELY(s != 0))
				Locate(e.X * s / boxReader.pbReader.GetWidth());
		},
		FetchEvent<Paint>(boxReader.pbReader) += [this](PaintEventArgs&& e){
			auto& pb(boxReader.pbReader);
			const auto mval(pb.GetMaxValue());
			const auto w(pb.GetWidth() - 2);
			auto& pt(e.Location);

			FillRect(e.Target, Point(pt.X + 1 + round(pb.GetValue() * w / mval),
				pt.Y + 1), Size(round((reader.GetBottomPosition()
				- GetReaderPosition()) * w / mval), pb.GetHeight() - 2),
				ColorSpace::Yellow);
		},
		FetchEvent<Click>(pnlSetting.btnClose) += exit_session,
		FetchEvent<Click>(pnlSetting.btnOK) += [&, this](TouchEventArgs&&){
			pnlSetting >> CurrentSetting;
			tmrScroll.SetInterval(CurrentSetting.GetTimerSetting());
			Switch(pnlSetting.current_encoding),
			reader.SetColor(CurrentSetting.FontColor),
			reader.SetFont(CurrentSetting.Font);
			reader.UpdateView();
			yunseq(GetDesktopUp().Background = pnlSetting.lblAreaUp.Background,
				GetDesktopDown().Background = pnlSetting.lblAreaDown.Background
			);
			if(IsVisible(boxReader))
				for(auto pr(boxReader.GetChildren()); pr.first != pr.second;
					++pr.first)
					if(dynamic_cast<BufferedRenderer*>(
						&pr.first->GetRenderer()))
						Invalidate(*pr.first);
		},
		FetchEvent<Click>(pnlSetting.btnOK) += exit_session,
		FetchEvent<Click>(pnlBookmark.btnClose) += exit_session,
		FetchEvent<Click>(pnlBookmark.btnOK) += [this](TouchEventArgs&&){
			if(pnlBookmark.lbPosition.IsSelected()
				&& Locate(bookmarks[pnlBookmark.lbPosition.GetSelectedIndex()]))
				boxReader.UpdateData(reader);
		},
		FetchEvent<Click>(pnlBookmark.btnOK) += exit_session
	);
	{
		Menu& mnu(*(ynew Menu(Rect(), shared_ptr<Menu::ListType>(new
			Menu::ListType{"返回", "设置...", "文件信息...", "书签...",
			"向上一行", "向下一行", "向上一屏", "向下一屏"}), 1u)));

		mnu.GetConfirmed() += [this](IndexEventArgs&& e){
			Execute(e.Value);
		};
		mhMain += mnu;
	}
	ResizeForContent(mhMain[1u]);

	auto& dsk_up(GetDesktopUp());
	auto& dsk_dn(GetDesktopDown());

	reader.SetColor(CurrentSetting.FontColor),
	reader.SetFont(CurrentSetting.Font),
	yunseq(
		dsk_up.Background = SolidBrush(CurrentSetting.UpColor),
		dsk_dn.Background = SolidBrush(CurrentSetting.DownColor),
		FetchEvent<Click>(dsk_dn).Add(*this, &ShlTextReader::OnClick),
		FetchEvent<KeyDown>(dsk_dn).Add(*this, &ShlTextReader::OnKeyDown),
		FetchEvent<KeyHeld>(dsk_dn) += OnEvent_Call<KeyDown>
	);
	reader.Attach(dsk_up, dsk_dn),
	AddWidgets(dsk_dn, boxReader, boxTextInfo, pnlSetting, pnlBookmark);
	LoadFile(pth);
	LastRead.DropSubsequent();
	UpdateButtons();
	//置默认视图。
	// TODO: Associate view setting state for user selection.
	OnClick(TouchEventArgs(dsk_dn, 0));
	RequestFocusCascade(dsk_dn);
}

ShlTextReader::~ShlTextReader()
{
	SaveGlobalConfiguration(CurrentSetting);
	LastRead.Insert(CurrentPath, GetReaderPosition());
}

string
ShlTextReader::GetSlice(Bookmark::PositionType pos, string::size_type len)
{
	return CopySliceFrom(reader.GetTextBufferRef(), pos, len);
}

void
ShlTextReader::Execute(IndexEventArgs::ValueType idx)
{
	switch(idx)
	{
	case MR_Return:
		Exit();
		break;
	case MR_Setting:
		session_ptr.reset(new SettingSession(*this));
		break;
	case MR_Bookmark:
		session_ptr.reset(new BookmarkSession(*this));
		break;
	case MR_FileInfo:
		boxTextInfo.UpdateData(reader);
		Show(boxTextInfo);
		break;
	case MR_LineUp:
		reader.Execute(DualScreenReader::LineUpScroll);
		break;
	case MR_LineDown:
		reader.Execute(DualScreenReader::LineDownScroll);
		break;
	case MR_ScreenUp:
		reader.Execute(DualScreenReader::ScreenUpScroll);
		break;
	case MR_ScreenDown:
		reader.Execute(DualScreenReader::ScreenDownScroll);
		break;
	}
}

void
ShlTextReader::LoadFile(const IO::Path& pth)
{
	CurrentPath = pth;
	pTextFile = make_unique<TextFile>(pth);
	reader.LoadText(*pTextFile);
}

bool
ShlTextReader::Locate(Bookmark::PositionType pos)
{
	const auto s(reader.GetTextSize());

	if(YB_LIKELY(s != 0))
	{
		const auto old_pos(GetReaderPosition());

		reader.Locate(pos);
		if(YB_LIKELY(old_pos != GetReaderPosition()))
		{
			LastRead.Insert(CurrentPath, old_pos);
			LastRead.DropSubsequent();
			UpdateButtons();
			return true;
		}
	}
	return false;
}

void
ShlTextReader::Scroll()
{
	if(tmrScroll.IsActive())
		if(YB_UNLIKELY(tmrScroll.Refresh()))
		{
			if(CurrentSetting.SmoothScroll)
				reader.ScrollByPixel(1U);
			else
				reader.Execute(DualScreenReader::LineDownScroll);
		}
}

void
ShlTextReader::ShowMenu(Menu::ID id, const Point& pt)
{
	if(!mhMain.IsShowing(id))
	{
		auto& mnu(mhMain[id]);

		mnu.ClearSelected();
		switch(id)
		{
		case 1u:
			mnu.SetItemEnabled(MR_LineUp, !reader.IsTextTop());
			mnu.SetItemEnabled(MR_LineDown, !reader.IsTextBottom());
			mnu.SetItemEnabled(MR_ScreenUp, !reader.IsTextTop());
			mnu.SetItemEnabled(MR_ScreenDown, !reader.IsTextBottom());
		}
		SetLocationOf(mnu, pt);
		mhMain.Show(id);
	}
}

void
ShlTextReader::StopAutoScroll()
{
	reader.AdjustScrollOffset(),
	fBackgroundTask = nullptr,
	Deactivate(tmrScroll);
}

void
ShlTextReader::Switch(Encoding enc)
{
	if(enc != Encoding() && pTextFile && bool(*pTextFile)
		&& pTextFile->Encoding != enc)
	{
		pTextFile->Encoding = enc;
		reader.LoadText(*pTextFile);
	}
}

void
ShlTextReader::UpdateReadingList(bool is_prev)
{
	LastRead.Insert(CurrentPath, GetReaderPosition());

	const auto& bm(LastRead.Switch(is_prev));

	if(bm.Path != CurrentPath)
		LoadFile(bm.Path);
	if(reader.IsBufferReady())
		reader.Locate(bm.Position);
	UpdateButtons();
}

void
ShlTextReader::UpdateButtons()
{
	const auto pr(LastRead.CheckBoundary());

	yunseq(Enable(boxReader.btnPrev, pr.first),
		Enable(boxReader.btnNext, pr.second));
}

void
#if YCL_MINGW32
ShlTextReader::OnClick(TouchEventArgs&& e)
#else
ShlTextReader::OnClick(TouchEventArgs&&)
#endif
{
#if YCL_MINGW32
	if(e.Keys[VK_RBUTTON])
	{
		ShowMenu(1U, e);
		return;
	}
#endif
	if(tmrScroll.IsActive())
	{
		StopAutoScroll();
		Deactivate(tmrScroll);
		return;
	}
	if(IsVisible(boxReader))
	{
		Close(boxReader);
		reader.Stretch(0);
	}
	else
	{
		Show(boxReader);
		reader.Stretch(boxReader.GetHeight());
	}
}

void
ShlTextReader::OnKeyDown(KeyEventArgs&& e)
{
	using namespace Timers;
	using namespace KeyCodes;

	if(e.Strategy != RoutedEventArgs::Tunnel && !mhMain.IsShowing(1u)
		&& RepeatHeld(tmrInput, FetchGUIState().KeyHeldState,
		TimeSpan(240), TimeSpan(60)))
	{
		const auto ntick(HighResolutionClock::now());

		//这里可以考虑提供暂停，不调整视图。
		if(tmrScroll.IsActive())
		{
			StopAutoScroll();
			return;
		}

		const auto& k(e.GetKeys());

		if(k.count() != 1)
			return;
		if(k[YCL_KEY_Start])
		{
			fBackgroundTask = std::bind(&ShlTextReader::Scroll, this);
			tmrScroll.Reset();
			Activate(tmrScroll);
			return;
		}
		if(k[KeyCodes::Enter])
			reader.UpdateView();
		else if(k[Esc])
			Exit();
		else if(k[Up])
			reader.Execute(DualScreenReader::LineUpScroll);
		else if(k[Down])
			reader.Execute(DualScreenReader::LineDownScroll);
		else if(k[YCL_KEY(X)] || k[YCL_KEY(Y)])
		{
			auto size(reader.GetFont().GetSize());

			e.Handled = true;
			if(k[YCL_KEY(X)])
			{
				if(YB_LIKELY(size > Font::MinimalSize))
					--size;
				else
					return;
			}
			else if(YB_LIKELY(size < Font::MaximalSize))
				++size;
			else
				return;
			reader.SetFontSize(size);
			reader.UpdateView();
		}
		else if(k[YCL_KEY(L)])
		{
			if(YB_LIKELY(reader.GetLineGap() != 0))
				reader.SetLineGap(reader.GetLineGap() - 1);
		}
		else if(k[YCL_KEY(R)])
		{
			if(YB_LIKELY(reader.GetLineGap() != 12))
				reader.SetLineGap(reader.GetLineGap() + 1);
		}
		else if(k[Left])
	//	else if(k[PgUp])
			reader.Execute(DualScreenReader::ScreenUpScroll);
		else if(k[Right])
	//	else if(k[PgDn])
			reader.Execute(DualScreenReader::ScreenDownScroll);
		else
			return;
		tmrInput.Delay(HighResolutionClock::now() - ntick);
		e.Handled = true;
	}
}


ShlHexBrowser::ShlHexBrowser(const IO::Path& pth,
	const shared_ptr<Desktop>& h_dsk_up, const shared_ptr<Desktop>& h_dsk_dn)
	: ShlReader(pth, h_dsk_up, h_dsk_dn),
	HexArea(Rect({}, MainScreenWidth, MainScreenHeight)), pnlFileInfo()
{
	HexArea.SetRenderer(make_unique<BufferedRenderer>(true));
	yunseq(
		FetchEvent<KeyDown>(HexArea) += [this](KeyEventArgs&& e){
			if(e.GetKeys() == 1 << KeyCodes::Esc)
				Exit();
		},
		HexArea.ViewChanged += [this](HexViewArea::ViewArgs&&){
			pnlFileInfo.lblSize.Text = u"当前位置： "
				+ String(to_string(HexArea.GetModel().GetPosition())
				+ " / " + to_string(HexArea.GetModel().GetSize()));
			Invalidate(pnlFileInfo.lblSize);
		}
	);

	auto& dsk_up(GetDesktopUp());
	auto& dsk_dn(GetDesktopDown());
	const auto& path_str(pth.GetNativeString());

	pnlFileInfo.lblPath.Text = u"文件路径：" + pth;

	struct ::stat file_stat;

	//在 DeSmuME 上无效； iDSL + DSTT 上访问时间精确不到日，修改时间正常。
	::stat(path_str.c_str(), &file_stat);
	yunseq(pnlFileInfo.lblAccessTime.Text = u"访问时间："
		+ String(TranslateTime(file_stat.st_atime)),
		pnlFileInfo.lblModifiedTime.Text = u"修改时间："
		+ String(TranslateTime(file_stat.st_mtime)));
	dsk_up += pnlFileInfo;
	HexArea.Load(path_str.c_str());
	HexArea.UpdateData(0);
	HexArea.ViewChanged(HexViewArea::ViewArgs(HexArea, true));
	dsk_dn += HexArea;
	RequestFocusCascade(HexArea);
}

YSL_END_NAMESPACE(YReader)

