#ifndef traktor_spray_ITrigger_H
#define traktor_spray_ITrigger_H

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

class ITriggerInstance;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS ITrigger : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< ITriggerInstance > createInstance() const = 0;	
};

	}
}

#endif	// traktor_spray_ITrigger_H
