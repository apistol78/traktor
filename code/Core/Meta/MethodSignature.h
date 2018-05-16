#pragma once
#ifndef traktor_MethodSignature_H
#define traktor_MethodSignature_H

#include "Core/Config.h"

namespace traktor
{

#if defined(T_CXX11)

template<
	bool Const,
	typename ClassType,
	typename ReturnType,
	typename... ArgumentTypes
>
struct MethodSignature
{
	typedef ReturnType (ClassType::*method_t)(ArgumentTypes...);
	typedef ReturnType (*static_method_t)(ArgumentTypes...);
};

template<
	typename ClassType,
	typename ReturnType,
	typename... ArgumentTypes
>
struct MethodSignature< true, ClassType, ReturnType, ArgumentTypes... >
{
	typedef ReturnType (ClassType::*method_t)(ArgumentTypes...) const;
};

#else

template <
	typename ClassType,
	typename ReturnType,
	bool Const
>
struct MethodSignature_0
{
	T_NO_COPY_CLASS(MethodSignature_0);
	typedef ReturnType (ClassType::*method_t)();
	typedef ReturnType (*static_method_t)();
};

template <
	typename ClassType,
	typename ReturnType
>
struct MethodSignature_0 < ClassType, ReturnType, true >
{
	T_NO_COPY_CLASS(MethodSignature_0);
	typedef ReturnType (ClassType::*method_t)() const;
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	bool Const
>
struct MethodSignature_1
{
	T_NO_COPY_CLASS(MethodSignature_1);
	typedef ReturnType (ClassType::*method_t)(Argument1Type);
	typedef ReturnType (*static_method_t)(Argument1Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type
>
struct MethodSignature_1 < ClassType, ReturnType, Argument1Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_1);
	typedef ReturnType (ClassType::*method_t)(Argument1Type) const;
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	bool Const
>
struct MethodSignature_2
{
	T_NO_COPY_CLASS(MethodSignature_2);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type
>
struct MethodSignature_2 < ClassType, ReturnType, Argument1Type, Argument2Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_2);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type) const;
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	bool Const
>
struct MethodSignature_3
{
	T_NO_COPY_CLASS(MethodSignature_3);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct MethodSignature_3 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_3);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type) const;
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	bool Const
>
struct MethodSignature_4
{
	T_NO_COPY_CLASS(MethodSignature_4);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type
>
struct MethodSignature_4 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_4);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type) const;
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	bool Const
>
struct MethodSignature_5
{
	T_NO_COPY_CLASS(MethodSignature_5);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type
>
struct MethodSignature_5 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_5);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type) const;
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	bool Const
>
struct MethodSignature_6
{
	T_NO_COPY_CLASS(MethodSignature_6);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type
>
struct MethodSignature_6 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_6);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type) const;
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	typename Argument7Type,
	bool Const
>
struct MethodSignature_7
{
	T_NO_COPY_CLASS(MethodSignature_7);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	typename Argument7Type
>
struct MethodSignature_7 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_7);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type) const;
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	typename Argument7Type,
	typename Argument8Type,
	bool Const
>
struct MethodSignature_8
{
	T_NO_COPY_CLASS(MethodSignature_8);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type);
	typedef ReturnType (*static_method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type);
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	typename Argument7Type,
	typename Argument8Type
>
struct MethodSignature_8 < ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type, true >
{
	T_NO_COPY_CLASS(MethodSignature_8);
	typedef ReturnType (ClassType::*method_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type, Argument8Type) const;
};

#endif

}

#endif	// traktor_MethodSignature_H

