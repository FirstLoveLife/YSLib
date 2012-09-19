﻿/*
	Copyright (C) by Franksoft 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file Configuration.cpp
\ingroup NPL
\brief 配置设置。
\version r509
\author FrankHB<frankhb1989@gmail.com>
\since build 334
\par 创建时间:
	2012-08-27 15:15:06 +0800
\par 修改时间:
	2012-09-19 21:12 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	NPL::Configuration
*/


#include "NPL/Configuration.h"
#include "NPL/SContext.h"

using namespace YSLib;

YSL_BEGIN_NAMESPACE(NPL)

ValueNode
TransformConfiguration(const ValueNode& node)
{
	const auto s(node.GetSize());

	if(s == 0)
		return ValueNode("", node ? Access<string>(node) : string());

	auto i(node.GetBegin());

	if(s == 1)
		return TransformConfiguration(*i);

	const auto& new_name(Access<string>(*i++));

	if(s == 2)
	{
		const auto& n(TransformConfiguration(*i));

		if(n.GetName().empty())
			return ValueNode(new_name, n.GetValueRRef());
		return ValueNode(new_name, n);
	}

	auto p_node_cont(make_unique<ValueNode::Container>());

	std::for_each(i, node.GetEnd(), [&](const ValueNode& n){
		p_node_cont->insert(TransformConfiguration(n));
	});
	return ValueNode(new_name, p_node_cont.release(), PointerTag());
}


namespace
{

//! \since build 334
inline File&
WritePrefix(File& f, size_t n = 1, char c = '\t')
{
	while(n--)
		f << c;
	return f;
}

#if 0
//! \since build 334
File&
WriteNode(File& f, const ValueNode& node, size_t depth)
{
	if(node.GetSize() != 0)
	{
		++depth;
		for(const auto& n : node)
		{
			WritePrefix(f, depth);
			f << '(' << '\n';
			try
			{
				WriteNode(f, n, depth + 1);
			}
			catch(std::out_of_range&)
			{}
			WritePrefix(f, depth);
			f << ')' << '\n';
		}
		--depth;
	}
	f << '\n';
	return f;
}
#endif

//! \since build 335
string
EscapeNodeString(const string& str)
{
	const char c(CheckLiteral(str));
	auto content(MakeEscape(c == char() ? str : ystdex::get_mid(str)));

	return c == char() ? std::move(content) : c + content + c;
}

//! \since build 334
File&
WriteNodeC(File& f, const ValueNode& node, size_t depth)
{
	WritePrefix(f, depth);
	f << EscapeNodeString(node.GetName());
	if(node.GetSize() != 0)
	{
		f << '\n';
		for(const auto& n : node)
		{
			WritePrefix(f, depth);
			f << '(' << '\n';
			try
			{
				WriteNodeC(f, n, depth + 1);
			}
			catch(std::out_of_range&)
			{}
			WritePrefix(f, depth);
			f << ')' << '\n';
		}
	}
	else if(node)
		f << ' ' << EscapeNodeString(Access<string>(node));
	f << '\n';
	return f;
}

} // unnamed namespace;

File&
operator<<(File& f, const Configuration& conf)
{
	return WriteNodeC(f, conf.GetRoot(), 0);
}

TextFile&
operator>>(TextFile& tf, Configuration& conf)
{
	try
	{
		tf.Rewind();
		conf.root = TransformConfiguration(SContext::Analyze(Session(tf)));
	}
	catch(ystdex::bad_any_cast&)
	{
		throw LoggedEvent("Bad configuration found.", 0x80);
	}
	return tf;
}


ConfigurationFile::ConfigurationFile(const string& filename)
	: TextFile(filename.c_str(), std::ios_base::in | std::ios_base::out
	| std::ios_base::trunc), conf()
{
	*this >> conf;
}

void
ConfigurationFile::Update()
{
	Truncate(0),
	Rewind();
	*this << conf;
}

YSL_END_NAMESPACE(NPL)
