#pragma once

#include "Core/Class/AutoVerify.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Io/OutputStream.h"
#include "Core/Meta/MethodSignature.h"

namespace traktor
{

/*! \ingroup Core */
/*! \{ */

template < bool Const, typename ClassType, typename ReturnType, typename ... ArgumentTypes >
class AutoMethod final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(AutoMethod);

public:
	typedef typename MethodSignature< Const, ClassType, ReturnType, ArgumentTypes ... >::method_t method_t;

	method_t m_method;

	explicit AutoMethod(method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual void signature(OutputStream& os) const override final 
	{
		os << CastAny< ReturnType >::typeName();
		int __dummy__[(sizeof ... (ArgumentTypes)) + 1] = { (os << L"," << CastAny< ArgumentTypes >::typeName(), 0) ... };
        (void)__dummy__;
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(sizeof ... (ArgumentTypes));
		T_VERIFY_ARGUMENT_TYPES;
		return invokeI(self, argv, std::make_index_sequence< sizeof...(ArgumentTypes) >());
	}

private:
	template < std::size_t ... Is >
	inline Any invokeI(ITypedObject* self, const Any* argv, std::index_sequence< Is ... >) const
	{
		return CastAny< ReturnType >::set((T_VERIFY_CAST_SELF(ClassType, self)->*m_method)(
			CastAny< ArgumentTypes >::get(argv[Is]) ...
		));
	}
};

template < bool Const, typename ClassType, typename ... ArgumentTypes >
class AutoMethod < Const, ClassType, void, ArgumentTypes ... > final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(AutoMethod);

public:
	typedef typename MethodSignature< Const, ClassType, void, ArgumentTypes ... >::method_t method_t;

	method_t m_method;

	explicit AutoMethod(method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual void signature(OutputStream& os) const override final 
	{
		os << L"void";
		int __dummy__[(sizeof ... (ArgumentTypes)) + 1] = { (os << L"," << CastAny< ArgumentTypes >::typeName(), 0) ... };
        (void)__dummy__;
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(sizeof ... (ArgumentTypes));
		T_VERIFY_ARGUMENT_TYPES;
		invokeI(self, argv, std::make_index_sequence< sizeof...(ArgumentTypes) >());
		return Any();
	}

private:
	template < std::size_t... Is >
	inline void invokeI(ITypedObject* self, const Any* argv, std::index_sequence< Is... >) const
	{
		(T_VERIFY_CAST_SELF(ClassType, self)->*m_method)(
			CastAny< ArgumentTypes >::get(argv[Is]) ...
		);
	}
};

/*! \} */

}
