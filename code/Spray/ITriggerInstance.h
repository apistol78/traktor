#ifndef traktor_spray_ITriggerInstance_H
#define traktor_spray_ITriggerInstance_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

struct Context;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS ITriggerInstance : public Object
{
	T_RTTI_CLASS;

public:
	virtual void perform(Context& context) = 0;
};

	}
}

#endif	// traktor_spray_ITriggerInstance_H
