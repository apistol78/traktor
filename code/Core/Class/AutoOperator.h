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

/*! \name Operator */
/*! \{ */

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type
>
struct Operator : public IRuntimeDispatch
{
	T_NO_COPY_CLASS(Operator);

	typedef typename MethodSignature< true, ClassType, ReturnType, Argument1Type >::method_t method_t;

	method_t m_method;

	Operator(method_t method)
	:	m_method(method)
	{
	}

	virtual void signature(OutputStream& ss) const T_OVERRIDE T_FINAL
	{
	}

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL
	{
		T_VERIFY_ARGUMENT_COUNT(1)

		if (CastAny< Argument1Type >::accept(argv[0]))
		{
			ClassType* target = mandatory_non_null_type_cast< ClassType* >(self);
			return CastAny< ReturnType >::set((target->*m_method)(
				CastAny< Argument1Type >::get(argv[0])
			));
		}
		else
			return Any();
	}
};

/*! \} */

/*! \} */

}
