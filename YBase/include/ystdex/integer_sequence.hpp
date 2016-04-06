﻿/*
	© 2012-2013, 2015-2016 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file integer_sequence.hpp
\ingroup YStandardEx
\brief C++ 变长参数相关操作。
\version r422
\author FrankHB <frankhb1989@gmail.com>
\since build 589
\par 创建时间:
	2013-03-30 00:55:06 +0800
\par 修改时间:
	2016-04-05 12:05 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YStandardEx::IntegerSequence
*/


#ifndef YB_INC_ystdex_sequence_hpp_
#define YB_INC_ystdex_sequence_hpp_ 1

#include "variadic.hpp" // for "variadic.hpp", vseq::defer_i, _t, common_type_t;

namespace ystdex
{

inline namespace cpp2014
{

/*!
\ingroup meta_types
\see ISO C++14 20.5 [intseq] 。
\since build 589
*/
//@{
#if __cpp_lib_integer_sequence >= 201304 || __cplusplus > 201103L
using std::integer_sequence;
using std::index_sequence;
#else
template<typename _tInt, _tInt... _vSeq>
struct integer_sequence
{
	using value_type = _tInt;

	static yconstfn size_t
	size() ynoexcept
	{
		return sizeof...(_vSeq);
	}
};

template<size_t... _vSeq>
using index_sequence = integer_sequence<size_t, _vSeq...>;
#endif
//@}

} // inline namespace cpp2014;


//! \since build 589
//@{
namespace vseq
{

//! \since build 650
template<typename _tInt, template<_tInt...> class _gOp, _tInt... _vSeq>
struct defer_i<_tInt, _gOp, integer_sequence<_tInt, _vSeq...>,
	void_t<_gOp<_vSeq...>>>
{
	using type = _gOp<_vSeq...>;
};


template<typename _tInt, _tInt... _vSeq>
struct clear<integer_sequence<_tInt, _vSeq...>>
{
	using type = integer_sequence<_tInt>;
};


template<typename _tInt, _tInt... _vSeq1, _tInt... _vSeq2>
struct concat<integer_sequence<_tInt, _vSeq1...>,
	integer_sequence<_tInt, _vSeq2...>>
{
	using type = integer_sequence<_tInt, _vSeq1..., _vSeq2...>;
};


template<class _tCtor, typename _tInt, _tInt... _vSeq>
struct fmap<_tCtor, integer_sequence<_tInt, _vSeq...>>
{
	using type = apply_t<_tCtor, integral_constant<_tInt, _vSeq>...>;
};


template<typename _tInt, _tInt... _vSeq>
struct seq_size<integer_sequence<_tInt, _vSeq...>>
	: integral_constant<size_t, sizeof...(_vSeq)>
{};


template<typename _tInt, _tInt _vHead, _tInt... _vSeq>
struct front<integer_sequence<_tInt, _vHead, _vSeq...>>
	: integral_constant<_tInt, _vHead>
{};



template<typename _tInt, _tInt _vHead, _tInt... _vTail>
struct pop_front<integer_sequence<_tInt, _vHead, _vTail...>>
{
	using type = integer_sequence<_tInt, _vTail...>;
};


template<typename _tInt, _tInt... _vSeq, _tInt _vItem>
struct push_back<integer_sequence<_tInt, _vSeq...>,
	integral_constant<_tInt, _vItem>>
{
	using type = integer_sequence<_tInt, _vSeq..., _vItem>;
};


template<typename _tInt, _tInt... _vSeq, _tInt _vItem>
struct push_front<integer_sequence<_tInt, _vSeq...>,
	integral_constant<_tInt, _vItem>>
{
	using type = integer_sequence<_tInt, _vItem, _vSeq...>;
};


template<typename _tInt, _tInt... _vSeq, size_t... _vIdxSeq>
struct project<integer_sequence<_tInt, _vSeq...>,
	index_sequence<_vIdxSeq...>>
{
	using type = integer_sequence<_tInt,
		at<integer_sequence<_tInt, _vSeq...>, _vIdxSeq>::value...>;
};


template<typename _tInt, class _fBinary, class _tState>
struct fold<_fBinary, _tState, integer_sequence<_tInt>>
{
	using type = _tState;

	static yconstexpr const auto value = _tState::value;
};

template<typename _tInt, class _fBinary, class _tState, _tInt _vHead>
struct fold<_fBinary, _tState, integer_sequence<_tInt, _vHead>>
{
	static yconstexpr const auto value = _fBinary()(_tState::value, _vHead);

	using type = integer_sequence<_tInt, value>;
};

template<typename _tInt, class _fBinary, class _tState, _tInt... _vSeq>
struct fold<_fBinary, _tState, integer_sequence<_tInt, _vSeq...>>
{
private:
	using parts = split_n<sizeof...(_vSeq) / 2,
		integer_sequence<_tInt, _vSeq...>>;
	using head = _t<parts>;
	using tail = typename parts::tail;

public:
	static yconstexpr const auto value = fold<_fBinary, std::integral_constant<
		size_t, fold<_fBinary, _tState, head>::value>, tail>::value;

	using type = index_sequence<value>;
};


template<typename _tInt, _tInt... _vSeq1, _tInt... _vSeq2>
struct vec_add<integer_sequence<_tInt, _vSeq1...>,
	integer_sequence<_tInt, _vSeq2...>>
{
	using type = integer_sequence<_tInt, (_vSeq1 + _vSeq2)...>;
};


template<typename _tInt, _tInt... _vSeq1, _tInt... _vSeq2>
struct vec_subtract<integer_sequence<_tInt, _vSeq1...>,
	integer_sequence<_tInt, _vSeq2...>>
{
	using type = integer_sequence<_tInt, (_vSeq1 - _vSeq2)...>;
};

} // namespace vseq;
//@}


/*!
\brief 可变参数整数列表延迟求值。
\sa vseq::defer_i
\since build 650
*/
template<typename _tInt, template<_tInt...> class _gOp, _tInt... _vSeq>
struct vdefer_i
	: vseq::defer_i<_tInt, _gOp, integer_sequence<_tInt, _vSeq...>>
{};

/*!
\sa vseq::fold
\since build 671
*/
//@{
//! \brief 整数类型值二元操作合并应用。
template<typename _tInt, class _fBinary, _tInt _vState, _tInt... _vSeq>
using vfold_i = vseq::fold_t<_fBinary, std::integral_constant<_tInt, _vState>,
	integer_sequence<_tInt, _vSeq...>>;

//! \brief size_t 类型值二元操作合并应用。
template<class _fBinary, size_t _vState, size_t... _vSeq>
using vfold_s = vfold_i<size_t, _fBinary, _vState, _vSeq...>;
//@}


/*!
\ingroup metafunctions
\since build 590
*/
//@{
//! \brief 取整数序列的自然数后继。
//@{
template<typename _tInt, _tInt, class>
struct make_successor;

template<typename _tInt, _tInt _vBase, class _tSeq>
using make_successor_t = _t<make_successor<_tInt, _vBase, _tSeq>>;

template<typename _tInt, _tInt _vBase, _tInt... _vSeq>
struct make_successor<_tInt, _vBase, integer_sequence<_tInt, _vSeq...>>
{
private:
	using common_t = common_type_t<size_t, _tInt>;

public:
	// XXX: Conversion to '_tInt' might be implementation-defined.
	using type
		= integer_sequence<_tInt, _vSeq..., static_cast<_tInt>(static_cast<
		common_t>(_vBase) + static_cast<common_t>(sizeof...(_vSeq)))>;
};
//@}


//! \brief 取皮亚诺公理决定的整数序列。
//@{
template<typename _tInt, _tInt, size_t>
struct make_peano_sequence;

//! \since build 590
template<typename _tInt, _tInt _vBase, size_t _vN>
using make_peano_sequence_t
	= _t<make_peano_sequence<_tInt, _vBase, _vN>>;

template<typename _tInt, _tInt _vBase, size_t _vN>
struct make_peano_sequence
{
	using type = make_successor_t<_tInt, _vBase,
		make_peano_sequence_t<_tInt, _vBase, _vN - 1>>;
};

template<typename _tInt, _tInt _vBase>
struct make_peano_sequence<_tInt, _vBase, 0>
{
	using type = integer_sequence<_tInt>;
};
//@}


//! \brief 取自然数序列。
//@{
template<typename _tInt, size_t _vN>
using make_natural_sequence = make_peano_sequence<_tInt, 0, _vN>;

template<typename _tInt, size_t _vN>
using make_natural_sequence_t
	= _t<make_natural_sequence<_tInt, _vN>>;
//@}

/*!
\brief 判断是否具有相同的指定值。
\since build 651
*/
template<typename _tInt, _tInt _vDefault, _tInt... _values>
using same_value_as = is_same<integer_sequence<_tInt, _vDefault,
	_values...>, integer_sequence<_tInt, _values..., _vDefault>>;
//@}


inline namespace cpp2014
{

/*!
\ingroup metafunctions
\see ISO C++14 20.5 [intseq] 。
\since build 589
*/
//@{
#if __cpp_lib_integer_sequence >= 201304 || __cplusplus > 201103L
using std::make_integer_sequence;
using std::make_index_sequence;

using std::index_sequence_for;
#else
template<typename _tInt, size_t _vN>
using make_integer_sequence = make_natural_sequence_t<_tInt, _vN>;

template<size_t _vN>
using make_index_sequence = make_integer_sequence<size_t, _vN>;


template<typename... _types>
using index_sequence_for = make_index_sequence<sizeof...(_types)>;
#endif
//@}

} // inline namespace cpp2014;

} // namespace ystdex;

#endif

