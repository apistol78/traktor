#ifndef traktor_spray_ITrigger_H
#define traktor_spray_ITrigger_H

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

class ITriggerInstance;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS ITrigger : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool bind(resource::IResourceManager* resourceManager) = 0;

	virtual Ref< ITriggerInstance > createInstance() const = 0;	
};

	}
}

#endif	// traktor_spray_ITrigger_H
