#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

/*! \brief
 * \ingroup Core
 */
class T_DLLCLASS IRuntimeObject : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< const IRuntimeClass > getRuntimeClass() const = 0;
};

}

