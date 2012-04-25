#ifndef traktor_spray_ITriggerData_H
#define traktor_spray_ITriggerData_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace spray
	{

class ITrigger;

/*! \brief Trigger persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS ITriggerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< ITrigger > createTrigger(resource::IResourceManager* resourceManager) const = 0;	
};

	}
}

#endif	// traktor_spray_ITriggerData_H
