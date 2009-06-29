#ifndef traktor_resource_ResourceCache_H
#define traktor_resource_ResourceCache_H

#include <map>
#include "Resource/IResourceCache.h"

namespace traktor
{
	namespace resource
	{

/*! \brief Default resource cache.
 * \ingroup Resource
 *
 * Simple resource cache which stores every resource
 * in a plain map.
 */ 
class ResourceCache : public IResourceCache
{
	T_RTTI_CLASS(ResourceCache)

public:
	virtual void put(const Guid& guid, IResourceHandle* handle);

	virtual IResourceHandle* get(const Guid& guid);

	virtual void flush(const Guid& guid);

	virtual void flush();

private:
	std::map< Guid, Ref< IResourceHandle > > m_cache;
};

	}
}

#endif	// traktor_resource_ResourceCache_H
