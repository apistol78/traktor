#pragma once

#include "Core/Class/AutoVerify.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeDispatch.h"

namespace traktor
{

/*! \ingroup Core */
/*! \{ */

/*! \name Constructors */
/*! \{ */

template<
	typename ClassType
>
struct Constructor_0 : public IRuntimeDispatch
{
#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return L"";
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		return Any::fromObject(new ClassType());
	}
};

template<
	typename ClassType,
	typename Argument1Type
>
struct Constructor_1 : public IRuntimeDispatch
{
#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< Argument1Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(1)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)

		return Any::fromObject(new ClassType(
			CastAny< Argument1Type >::get(argv[0])
		));
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type
>
struct Constructor_2 : public IRuntimeDispatch
{
#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(2)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)

		return Any::fromObject(new ClassType(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
		));
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct Constructor_3 : public IRuntimeDispatch
{
#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(3)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)

		return Any::fromObject(new ClassType(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2])
		));
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type
>
struct Constructor_4 : public IRuntimeDispatch
{
#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return CastAny< Argument1Type >::typeName() + L"," + CastAny< Argument2Type >::typeName() + L"," + CastAny< Argument3Type >::typeName() + L"," + CastAny< Argument4Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(4)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)

		return Any::fromObject(new ClassType(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3])
		));
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type
>
struct Constructor_5 : public IRuntimeDispatch
{
#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return
			CastAny< Argument1Type >::typeName() + L"," +
			CastAny< Argument2Type >::typeName() + L"," +
			CastAny< Argument3Type >::typeName() + L"," +
			CastAny< Argument4Type >::typeName() + L"," +
			CastAny< Argument5Type >::typeName();
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

		return Any::fromObject(new ClassType(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4])
		));
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type
>
struct Constructor_6 : public IRuntimeDispatch
{
#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return
 			CastAny< Argument1Type >::typeName() + L"," +
			CastAny< Argument2Type >::typeName() + L"," +
			CastAny< Argument3Type >::typeName() + L"," +
			CastAny< Argument4Type >::typeName() + L"," +
			CastAny< Argument5Type >::typeName() + L"," +
			CastAny< Argument6Type >::typeName();
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

		return Any::fromObject(new ClassType(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3]),
			CastAny< Argument5Type >::get(argv[4]),
			CastAny< Argument6Type >::get(argv[5])
		));
	}
};

template<
	typename ClassType,
	typename Argument1Type
>
struct FnConstructor_1 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(FnConstructor_1);

	typedef Ref< ClassType > (*fn_t)(Argument1Type);
	fn_t fn;

	explicit FnConstructor_1(fn_t fn_)
	:	fn(fn_)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
 		return CastAny< Argument1Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(1)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)

		return Any::fromObject((*fn)(
			CastAny< Argument1Type >::get(argv[0])
		));
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type
>
struct FnConstructor_2 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(FnConstructor_2);

	typedef Ref< ClassType > (*fn_t)(Argument1Type, Argument2Type);
	fn_t fn;

	explicit FnConstructor_2(fn_t fn_)
	:	fn(fn_)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return
	 		CastAny< Argument1Type >::typeName() + L"," +
			CastAny< Argument2Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(2)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)

		return Any::fromObject((*fn)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1])
		));
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct FnConstructor_3 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(FnConstructor_3);

	typedef Ref< ClassType > (*fn_t)(Argument1Type, Argument2Type, Argument3Type);
	fn_t fn;

	explicit FnConstructor_3(fn_t fn_)
	:	fn(fn_)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return
 			CastAny< Argument1Type >::typeName() + L"," +
			CastAny< Argument2Type >::typeName() + L"," +
			CastAny< Argument3Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(3)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)

		return Any::fromObject((*fn)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2])
		));
	}
};

template<
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type
>
struct FnConstructor_4 : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(FnConstructor_4);

	typedef Ref< ClassType > (*fn_t)(Argument1Type, Argument2Type, Argument3Type, Argument4Type);
	fn_t fn;

	explicit FnConstructor_4(fn_t fn_)
	:	fn(fn_)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual std::wstring signature() const override final
	{
		return
 			CastAny< Argument1Type >::typeName() + L"," +
			CastAny< Argument2Type >::typeName() + L"," +
			CastAny< Argument3Type >::typeName() + L"," +
			CastAny< Argument4Type >::typeName();
	}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(4)
		T_VERIFY_ARGUMENT_TYPE(0, Argument1Type)
		T_VERIFY_ARGUMENT_TYPE(1, Argument2Type)
		T_VERIFY_ARGUMENT_TYPE(2, Argument3Type)
		T_VERIFY_ARGUMENT_TYPE(3, Argument4Type)

		return Any::fromObject((*fn)(
			CastAny< Argument1Type >::get(argv[0]),
			CastAny< Argument2Type >::get(argv[1]),
			CastAny< Argument3Type >::get(argv[2]),
			CastAny< Argument4Type >::get(argv[3])
		));
	}
};

/*! \} */

/*! \} */

}
