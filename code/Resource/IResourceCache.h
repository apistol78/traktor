#ifndef traktor_resource_IResourceCache_H
#define traktor_resource_IResourceCache_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! \brief Resource cache interface.
 *
 * A resource cache is a container object which is used by the ResourceManager
 * to store loaded resources.
 */ 
class T_DLLCLASS IResourceCache : public Object
{
	T_RTTI_CLASS(IResourceCache)

public:
	virtual void put(const Guid& guid, Object* resource) = 0;

	virtual bool get(const Guid& guid, Ref< Object >& outResource) const = 0;

	virtual void flush(const Guid& guid) = 0;

	virtual void flush() = 0;
};

	}
}

#endif	// traktor_resource_IResourceCache_H
