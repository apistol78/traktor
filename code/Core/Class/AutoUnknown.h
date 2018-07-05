/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
struct Unknown : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Unknown);

	typedef Any (*unknown_fn_t)(ClassType* self, const std::string& methodName, uint32_t argc, const Any* argv);

	unknown_fn_t m_unknown;

	Unknown(unknown_fn_t unknown)
	:	m_unknown(unknown)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(1)
		
		std::string methodName = argv[0].getString();

		return (*m_unknown)(
			mandatory_non_null_type_cast< ClassType* >(self),
			methodName,
			argc - 1,
			&argv[1]
		);
	}
};

/*! \} */

/*! \} */

}
