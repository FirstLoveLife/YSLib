﻿/*
	© 2014-2015 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file type_pun.hpp
\ingroup YStandardEx
\brief 共享存储和直接转换。
\version r314
\author FrankHB <frankhb1989@gmail.com>
\since build 629
\par 创建时间:
	2015-09-04 12:16:27 +0800
\par 修改时间:
	2015-11-06 12:37 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YStandardEx::TypePun
*/


#ifndef YB_INC_ystdex_type_pun_hpp_
#define YB_INC_ystdex_type_pun_hpp_ 1

#include "type_traits.hpp" // for bool_constant, yalignof, and_, is_trivial,
//	enable_if_t, is_object_pointer, remove_pointer_t, aligned_storage_t,
//	is_reference, remove_reference_t, exclude_self_ctor_t, decay_t;
#include <new> // for placement ::operator new from standard library;

namespace ystdex
{

/*!	\defgroup aligned_type_traits Aligned Type Traits
\ingroup binary_type_traits
\brief 对齐类型特征。
\note 第一参数类型比第二参数类型对齐要求更严格或相同。
\since build 630
*/
//@{
/*!
\brief 判断是否对齐兼容。
\since build 610
*/
template<typename _type, typename _type2>
struct is_aligned_compatible : bool_constant<!(yalignof(_type)
	< yalignof(_type2)) && yalignof(_type) % yalignof(_type2) == 0>
{};


/*!
\brief 判断是否可原地对齐存储。
\since build 610
*/
template<typename _type, typename _tDst>
struct is_aligned_placeable : bool_constant<sizeof(_type)
	== sizeof(_tDst) && is_aligned_compatible<_type, _tDst>::value>
{};


/*!
\brief 判断是否可对齐存储。
\since build 503
*/
template<typename _type, typename _tDst>
struct is_aligned_storable : bool_constant<!(sizeof(_type) < sizeof(_tDst))
	&& is_aligned_compatible<_type, _tDst>::value>
{};


/*!
\brief 判断是否可替换对齐存储。
\since build 610
*/
template<typename _type, typename _tDst>
struct is_aligned_replaceable : and_<is_aligned_storable<_type, _tDst>,
	is_aligned_storable<_tDst, _type>>
{};


/*!
\brief 判断是否可平凡替换存储。
\todo 使用 \c is_trivially_copyable 代替 is_trivial 。
*/
template<typename _type, typename _tDst>
struct is_trivially_replaceable : and_<is_trivial<_type>, is_trivial<_tDst>,
	is_aligned_replaceable<_type, _tDst>>
{};
//@}


//! \since build 630
//@{
/*!
\ingroup metafunctions
\brief 选择和特定类型可替换字符类类型的特定重载以避免冲突。
*/
template<typename _tSrc, typename _tDst, typename _type = void>
using enable_if_replaceable_t
	= enable_if_t<is_trivially_replaceable<_tSrc, _tDst>::value, _type>;


/*!
\ingroup transformation_traits
\brief 用于显式转换的替代对齐存储 POD 类型。
*/
template<typename _type, size_t _vAlign = yalignof(_type)>
using pun_storage_t = aligned_storage_t<sizeof(_type), _vAlign>;


/*!
\ingroup cast
\pre 目标类型是对象指针或引用。
\note 使用 \c reinterpret_cast 且保证目标类型的对齐要求不比源类型更严格以保证可逆。
*/
//@{
/*!
\brief 保证对齐兼容的显式转换。
\note 用于替代针对满足 is_aligned_compatible 要求对象的 \c reinterpret_cast 。
*/
//@{
template<typename _pDst, typename _tSrc>
inline yimpl(ystdex::enable_if_t)<and_<is_object_pointer<_pDst>,
	is_aligned_compatible<_tSrc, remove_pointer_t<_pDst>>>::value, _pDst>
aligned_cast(_tSrc* v) ynothrow
{
	return reinterpret_cast<_pDst>(v);
}
template<typename _rDst, typename _tSrc, yimpl(typename = ystdex::enable_if_t<
	and_<is_reference<_rDst>, is_aligned_compatible<remove_reference_t<_tSrc>,
	remove_reference_t<_rDst>>>::value>)>
inline auto
aligned_cast(_tSrc&& v) ynothrow
	-> decltype(yforward(reinterpret_cast<_rDst>(v)))
{
	return yforward(reinterpret_cast<_rDst>(v));
}
//@}


/*!
\brief 保证对齐存储的显式转换。
\note 用于替代针对满足 is_aligned_storable 要求对象的 \c reinterpret_cast 。
*/
//@{
template<typename _pDst, typename _tSrc>
inline yimpl(ystdex::enable_if_t)<and_<is_object_pointer<_pDst>,
	is_aligned_storable<_tSrc, remove_pointer_t<_pDst>>>::value, _pDst>
aligned_store_cast(_tSrc* v) ynothrow
{
	return reinterpret_cast<_pDst>(v);
}
template<typename _rDst, typename _tSrc, yimpl(typename = ystdex::enable_if_t<
	and_<is_reference<_rDst>, is_aligned_storable<remove_reference_t<_tSrc>,
	remove_reference_t<_rDst>>>::value>)>
inline auto
aligned_store_cast(_tSrc&& v) ynothrow
	-> decltype(yforward(reinterpret_cast<_rDst>(v)))
{
	return yforward(reinterpret_cast<_rDst>(v));
}
//@}


/*!
\brief 保证对齐替换存储的显式转换。
\note 用于替代针对满足 is_aligned_replaceable 要求对象的 \c reinterpret_cast 。
*/
//@{
template<typename _pDst, typename _tSrc>
inline yimpl(ystdex::enable_if_t)<and_<is_object_pointer<_pDst>,
	is_aligned_replaceable<_tSrc, remove_pointer_t<_pDst>>>::value, _pDst>
aligned_replace_cast(_tSrc* v) ynothrow
{
	return reinterpret_cast<_pDst>(v);
}
template<typename _rDst, typename _tSrc, yimpl(typename = ystdex::enable_if_t<
	and_<is_reference<_rDst>, is_aligned_replaceable<remove_reference_t<_tSrc>,
	remove_reference_t<_rDst>>>::value>)>
inline auto
aligned_replace_cast(_tSrc&& v) ynothrow
	-> decltype(yforward(reinterpret_cast<_rDst>(v)))
{
	return yforward(reinterpret_cast<_rDst>(v));
}
//@}


/*!
\brief 保证平凡替换存储的显式转换。
\note 用于替代针对满足 is_trivially_replaceable 要求对象的 \c reinterpret_cast 。
\since build 629
*/
//@{
template<typename _pDst, typename _tSrc>
inline yimpl(ystdex::enable_if_t)<and_<is_object_pointer<_pDst>,
	is_trivially_replaceable<_tSrc, remove_pointer_t<_pDst>>>::value, _pDst>
replace_cast(_tSrc* v) ynothrow
{
	return reinterpret_cast<_pDst>(v);
}
template<typename _rDst, typename _tSrc, yimpl(typename = ystdex::enable_if_t<
	and_<is_reference<_rDst>, is_trivially_replaceable<remove_reference_t<_tSrc>,
	remove_reference_t<_rDst>>>::value>)>
inline auto
replace_cast(_tSrc&& v) ynothrow
	-> decltype(yforward(reinterpret_cast<_rDst>(v)))
{
	return yforward(reinterpret_cast<_rDst>(v));
}
//@}
//@}
//@}


/*!
\brief 任意 POD 类型存储。
\note POD 的含义参考 ISO C++11 。
\since build 351
*/
template<typename _tPOD = aligned_storage_t<sizeof(void*)>>
union pod_storage
{
	static_assert(is_pod<_tPOD>(), "Non-POD underlying type found.");

	using underlying = _tPOD;

	//! \since build 595
	mutable underlying object;
	//! \since build 595
	mutable byte data[sizeof(underlying)];

	//! \since build 352
	//@{
	pod_storage() = default;
	//! \since build 503
	pod_storage(const pod_storage&) = default;
	//! \since build 454
	template<typename _type,
		yimpl(typename = exclude_self_ctor_t<pod_storage, _type>)>
	inline
	pod_storage(_type&& x)
	{
		new(access()) decay_t<_type>(yforward(x));
	}

	//! \since build 503
	pod_storage&
	operator=(const pod_storage&) = default;
	/*!
	\note 为避免类型错误，需要确定类型时应使用显式使用 access 指定类型赋值。
	\since build 454
	*/
	template<typename _type,
		yimpl(typename = exclude_self_ctor_t<pod_storage, _type>)>
	inline pod_storage&
	operator=(_type&& x)
	{
		assign(yforward(x));
		return *this;
	}
	//@}

	//! \since build 595
	yconstfn YB_PURE void*
	access() const
	{
		return &data[0];
	}
	//! \since build 595
	template<typename _type>
	yconstfn YB_PURE _type&
	access() const
	{
		static_assert(is_aligned_storable<pod_storage, _type>(),
			"Invalid type found.");

		return *static_cast<_type*>(access());
	}

	//! \since build 503
	template<typename _type>
	inline void
	assign(_type&& x)
	{
		access<decay_t<_type>>() = yforward(x);
	}
};

} // namespace ystdex;

#endif

