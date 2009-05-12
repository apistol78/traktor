#ifndef traktor_resource_DualResourceCache_H
#define traktor_resource_DualResourceCache_H

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

/*! \brief Dual resource cache.
 *
 * This resource cache is only a facade to two
 * other resource caches; the idea is to have
 * multiple caches, one for shared resources across for example
 * multiple levels and one for per-level resources.
 * If balanced correctly there is no need to
 * reload all resources for each level etc.
 *
 * Primary cache is the cache which will be storing new
 * resources; thus it's the cache that will get flushed aswell.
 */ 
class T_DLLCLASS DualResourceCache : public IResourceCache
{
	T_RTTI_CLASS(DualResourceCache)

public:
	void setPrimaryCache(IResourceCache* primaryCache);

	IResourceCache* getPrimaryCache() const;

	void setSecondaryCache(IResourceCache* secondaryCache);

	IResourceCache* getSecondaryCache() const;

	virtual void put(const Guid& guid, Object* resource);

	virtual bool get(const Guid& guid, Ref< Object >& outResource) const;

	virtual void flush(const Guid& guid);

	virtual void flush();

private:
	Ref< IResourceCache > m_primaryCache;
	Ref< IResourceCache > m_secondaryCache;
};

	}
}

#endif	// traktor_resource_DualResourceCache_H
