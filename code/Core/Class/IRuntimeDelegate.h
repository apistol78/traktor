#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Any;

/*! \brief
 * \ingroup Core
 */
class T_DLLCLASS IRuntimeDelegate : public Object
{
	T_RTTI_CLASS;

public:
	virtual Any call(int32_t argc, const Any* argv) = 0;
};

}
