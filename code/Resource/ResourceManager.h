#ifndef traktor_resource_ResourceManager_H
#define traktor_resource_ResourceManager_H

#include <list>
#include "Core/Heap/Ref.h"
#include "Core/Singleton/Singleton.h"
#include "Core/Guid.h"
#include "Core/Thread/Semaphore.h"

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
	
class ResourceLoader;
class IResourceCache;

/*! \brief Resource manager singleton.
 *
 * The resource manager is the hub in the resource system
 * which each proxy uses to validate themselves.
 */
class T_DLLCLASS ResourceManager : public Singleton
{
	T_RTTI_CLASS(ResourceManager)

public:
	static ResourceManager& getInstance();
	
	void addLoader(ResourceLoader* loader);

	void removeLoader(ResourceLoader* loader);

	void removeAllLoaders();
	
	void setCache(IResourceCache* cache);

	IResourceCache* getCache() const;

	void setResource(const Type& type, const Guid& guid, Object* resource);

	Object* getResource(const Type& type, const Guid& guid);

	/*! \brief Begin accept requested resources.
	 *
	 * Requested resources are put into a list
	 * of required resources which are loaded
	 * at endPrepareResources.
	 */
	void beginPrepareResources();

	/*! \brief Request resource for later use.
	 *
	 * Notify resource manager that a resource might
	 * need to be available in the near future.
	 *
	 * \param type Type of resource.
	 * \param guid Resource guid.
	 */
	void requestResource(const Type& type, const Guid& guid);

	/*! \brief Load all requested resources.
	 *
	 * Load all requested resource; as new resources
	 * might add requests for other resources this
	 * method loops until all of the prepare resource
	 * has been loaded.
	 *
	 * \param cancel Cancel preparing resources.
	 */
	void endPrepareResources(bool cancel = false);

protected:
	virtual void destroy();

private:
	RefList< ResourceLoader > m_loaders;
	Ref< IResourceCache > m_cache;
	Semaphore m_lock;
	bool m_acceptResourceRequests;
	std::list< std::pair< const Type*, Guid > > m_requestedResources;

	ResourceManager();
};
	
	}
}

#endif	// traktor_resource_ResourceManager_H
