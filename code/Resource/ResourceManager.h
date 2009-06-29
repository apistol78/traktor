#ifndef traktor_resource_ResourceManager_H
#define traktor_resource_ResourceManager_H

#include "Core/Heap/Ref.h"
#include "Core/Thread/Semaphore.h"
#include "Resource/IResourceManager.h"

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

/*! \brief Resource manager.
 * \ingroup Resource
 */
class T_DLLCLASS ResourceManager : public IResourceManager
{
	T_RTTI_CLASS(ResourceManager)

public:
	ResourceManager();

	virtual void addFactory(IResourceFactory* factory);

	virtual void removeFactory(IResourceFactory* factory);

	virtual void removeAllFactories();
	
	virtual void setCache(IResourceCache* cache);

	virtual IResourceCache* getCache() const;

	virtual IResourceHandle* bind(const Type& type, const Guid& guid);

private:
	RefList< IResourceFactory > m_factories;
	Ref< IResourceCache > m_cache;
	Semaphore m_lock;
};
	
	}
}

#endif	// traktor_resource_ResourceManager_H
