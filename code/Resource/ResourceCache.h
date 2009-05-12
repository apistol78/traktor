#ifndef traktor_resource_ResourceCache_H
#define traktor_resource_ResourceCache_H

#include <map>
#include "Resource/IResourceCache.h"

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

/*! \brief Default resource cache.
 *
 * Simple resource cache which stores every resource
 * in a plain map.
 */ 
class T_DLLCLASS ResourceCache : public IResourceCache
{
	T_RTTI_CLASS(ResourceCache)

public:
	virtual void put(const Guid& guid, Object* resource);

	virtual bool get(const Guid& guid, Ref< Object >& outResource) const;

	virtual void flush(const Guid& guid);

	virtual void flush();

private:
	std::map< Guid, Ref< Object > > m_cache;
};

	}
}

#endif	// traktor_resource_ResourceCache_H
