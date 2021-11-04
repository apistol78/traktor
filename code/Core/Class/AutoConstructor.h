#pragma once

#include "Core/Class/AutoVerify.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeDispatch.h"

namespace traktor
{

/*! \ingroup Core */
/*! \{ */

template < typename ClassType, typename ... ArgumentTypes >
class AutoConstructor : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(AutoConstructor);

public:
	AutoConstructor() = default;

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual void signature(OutputStream& os) const override final {}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(sizeof ... (ArgumentTypes));
		return invokeI(argv, std::make_index_sequence< sizeof...(ArgumentTypes) >());
	}

private:
	template < std::size_t... Is >
	inline Any invokeI(const Any* argv, std::index_sequence< Is... >) const
	{
		return Any::fromObject(new ClassType(
			CastAny< ArgumentTypes >::get(argv[Is]) ...
		));
	}
};

template < typename ClassType, typename ... ArgumentTypes >
class AutoConstructorFactory : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(AutoConstructorFactory);

public:
	typedef Ref< ClassType > (*factory_t)(ArgumentTypes ...);

	factory_t m_factory;

	explicit AutoConstructorFactory(factory_t factory)
	:	m_factory(factory)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual void signature(OutputStream& os) const override final {}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(sizeof ... (ArgumentTypes));
		return invokeI(argv, std::make_index_sequence< sizeof...(ArgumentTypes) >());
	}

private:
	template < std::size_t... Is >
	inline Any invokeI(const Any* argv, std::index_sequence< Is... >) const
	{
		return Any::fromObject((*m_factory)(
			CastAny< ArgumentTypes >::get(argv[Is]) ...
		));
	}
};

/*! \} */

}
