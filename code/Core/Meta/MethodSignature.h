#pragma once

namespace traktor
{

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

}
