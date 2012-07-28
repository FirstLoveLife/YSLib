﻿/*
	Copyright (C) by Franksoft 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file MemoryMapping.h
\ingroup YCLib
\brief 内存映射文件。
\version r1082;
\author FrankHB<frankhb1989@gmail.com>
\since build 324 。
\par 创建时间:
	2012-07-11 21:48:15 +0800;
\par 修改时间:
	2012-07-17 01:26 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YCLib::MemoryMapping;
*/


#ifndef YCL_INC_MEMORYMAPPING_H_
#define YCL_INC_MEMORYMAPPING_H_

#include "ycommon.h"
#include "FileSystem.h" // for const_path_t;

namespace platform
{

/*!
\brief 只读内存映射文件。
\since build 324 。
*/
class MappedFile
{
private:
	int fd;
	struct ::stat st;
	ystdex::byte* addr;

public:
	//! \throw std::runtime_error 映射失败。
	//@{
	explicit
	MappedFile(const_path_t);
	//! \since build 326 。
	template<class _tString>
	explicit
	MappedFile(const _tString& filename)
		: MappedFile(filename.c_str())
	{}
	//@}
	~MappedFile();

	ystdex::byte*
	GetPtr() const
	{
		return addr;
	}
	size_t
	GetSize() const;
};

} // namespace platform;

#endif
