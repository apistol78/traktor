#pragma once

#include "Core/Class/AutoVerify.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Meta/MethodSignature.h"

namespace traktor
{

/*! \ingroup Core */
/*! \{ */

/*! \name Static method invocations */
/*! \{ */

template <
	typename ClassType,
	typename ReturnType
>
struct StaticMethod_0 final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_0);

	typedef typename MethodSignature< false, ClassType, ReturnType >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_0(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< ReturnType >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		ReturnType returnValue = (*m_method)();
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType
>
struct StaticMethod_0 < ClassType, void > final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_0);

	typedef typename MethodSignature< false, ClassType, void >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_0(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return L"void";
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		(*m_method)();
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type
>
struct StaticMethod_1 final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_1);

	typedef typename MethodSignature< false, ClassType, ReturnType, Argument1Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_1(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< ReturnType >::typeName() + L"," + CastAny< Argument1Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(1)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)

		ReturnType returnValue = (*m_method)(
			CastAny< Argument1Type >::get(argv[0])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type
>
struct StaticMethod_1 < ClassType, void, Argument1Type > final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_1);

	typedef typename MethodSignature< false, ClassType, void, Argument1Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_1(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return L"void," + CastAny< Argument1Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(1)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)

		(*m_method)(
			CastAny< Argument1Type >::get(argv[0])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type
>
struct StaticMethod_2 final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_2);

	typedef typename MethodSignature< false, ClassType, ReturnType, Argument1Type, Argument2Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_2(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< ReturnType >::typeName() + L"," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(2)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)

		ReturnType returnValue = (*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type
>
struct StaticMethod_2 < ClassType, void, Argument1Type, Argument2Type > final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_2);

	typedef typename MethodSignature< false, ClassType, void, Argument1Type, Argument2Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_2(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return L"void," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(2)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)

		(*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct StaticMethod_3 final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_3);

	typedef typename MethodSignature< false, ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_3(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< ReturnType >::typeName() + L"," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(3)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)

		ReturnType returnValue = (*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct StaticMethod_3 < ClassType, void, Argument1Type, Argument2Type, Argument3Type > final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_3);

	typedef typename MethodSignature< false, ClassType, void, Argument1Type, Argument2Type, Argument3Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_3(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return L"void," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(3)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)

		(*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2])
		);
		return Any();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type
>
struct StaticMethod_4 final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_4);

	typedef typename MethodSignature< false, ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_4(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< ReturnType >::typeName() + L"," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName() + L"," + CastAny< Argument4Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(4)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)

		ReturnType returnValue = (*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type
>
struct StaticMethod_4 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type > final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_4);

	typedef typename MethodSignature< false, ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_4(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return L"void," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName() + L"," + CastAny< Argument4Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(4)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)

		(*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3])
		);
		return Any();
	}
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
struct StaticMethod_5 final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_5);

	typedef typename MethodSignature< false, ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_5(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< ReturnType >::typeName() + L"," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName() + L"," + CastAny< Argument4Type >::typeName() + L"," + CastAny< Argument5Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(5)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)

		ReturnType returnValue = (*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type
>
struct StaticMethod_5 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type > final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_5);

	typedef typename MethodSignature< false, ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_5(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return L"void," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName() + L"," + CastAny< Argument4Type >::typeName() + L"," + CastAny< Argument5Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(5)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)

		(*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4])
		);
		return Any();
	}
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
struct StaticMethod_6 final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_6);

	typedef typename MethodSignature< false, ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_6(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< ReturnType >::typeName() + L"," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName() + L"," + CastAny< Argument4Type >::typeName() + L"," + CastAny< Argument5Type >::typeName() + L"," + CastAny< Argument6Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(6)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)
		T_VERIFY_ARGUMENT_TYPE(5, Argument6Type)

		ReturnType returnValue = (*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type
>
struct StaticMethod_6 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type > final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_6);

	typedef typename MethodSignature< false, ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_6(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return L"void," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName() + L"," + CastAny< Argument4Type >::typeName() + L"," + CastAny< Argument5Type >::typeName() + L"," + CastAny< Argument6Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(6)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)
		T_VERIFY_ARGUMENT_TYPE(5, Argument6Type)

		(*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5])
		);
		return Any();
	}
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
struct StaticMethod_7 final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_7);

	typedef typename MethodSignature< false, ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_7(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< ReturnType >::typeName() + L"," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName() + L"," + CastAny< Argument4Type >::typeName() + L"," + CastAny< Argument5Type >::typeName() + L"," + CastAny< Argument6Type >::typeName() + L"," + CastAny< Argument7Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(7)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)
		T_VERIFY_ARGUMENT_TYPE(5, Argument6Type)
		T_VERIFY_ARGUMENT_TYPE(6, Argument7Type)

		ReturnType returnValue = (*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5]),
			CastAny< Argument7Type >::get(argv[6])
		);
		return CastAny< ReturnType >::set(returnValue);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type,
	typename Argument7Type
>
struct StaticMethod_7 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type > final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(StaticMethod_7);

	typedef typename MethodSignature< false, ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type, Argument7Type >::static_method_t static_method_t;

	static_method_t m_method;

	explicit StaticMethod_7(static_method_t method)
	:	m_method(method)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return L"void," + CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName() + L"," + CastAny< Argument4Type >::typeName() + L"," + CastAny< Argument5Type >::typeName() + L"," + CastAny< Argument6Type >::typeName() + L"," + CastAny< Argument7Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(7)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)
		T_VERIFY_ARGUMENT_TYPE(4, Argument5Type)
		T_VERIFY_ARGUMENT_TYPE(5, Argument6Type)
		T_VERIFY_ARGUMENT_TYPE(6, Argument7Type)

		(*m_method)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5]),
			CastAny< Argument7Type >::get(argv[6])
		);
		return Any();
	}
};

/*! \} */

/*! \} */

}
