#ifndef traktor_IRuntimeObject_H
#define traktor_IRuntimeObject_H

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

class IRuntimeClass;

/*! \brief Object instance which can provide it's runtime class descriptor.
 * \ingroup Core
 */
class T_DLLCLASS IRuntimeObject : public Object
{
	T_RTTI_CLASS;

public:
	virtual const IRuntimeClass* getRuntimeClass() const = 0;
};

}

#endif	// traktor_IRuntimeObject_H
