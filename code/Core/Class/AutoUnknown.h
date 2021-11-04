#pragma once

#include "Core/Class/AutoVerify.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Meta/MethodSignature.h"

namespace traktor
{

/*! \ingroup Core */
/*! \{ */

/*! \name Unknown */
/*! \{ */

template <
	typename ClassType
>
struct Unknown final : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Unknown);

	typedef Any (*unknown_fn_t)(ClassType* self, const std::string& methodName, uint32_t argc, const Any* argv);

	unknown_fn_t m_unknown;

	explicit Unknown(unknown_fn_t unknown)
	:	m_unknown(unknown)
	{
	}

#if defined(T_NEED_RUNTIME_SIGNATURE)
	virtual void signature(OutputStream& os) const override final {}
#endif

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final
	{
		T_VERIFY_ARGUMENT_COUNT(1)

		std::string methodName = argv[0].getString();
		return (*m_unknown)(
			T_VERIFY_CAST_SELF(ClassType, self),
			methodName,
			argc - 1,
			&argv[1]
		);
	}
};

/*! \} */

/*! \} */

}
