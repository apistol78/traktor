#ifndef traktor_resource_ResourceManager_H
#define traktor_resource_ResourceManager_H

#include <map>
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Resource/IResourceManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceHandle;
class ResourceHandle;

/*! \brief Resource manager.
 * \ingroup Resource
 */
class T_DLLCLASS ResourceManager : public IResourceManager
{
	T_RTTI_CLASS;

public:
	ResourceManager();

	virtual void addFactory(IResourceFactory* factory);

	virtual void removeFactory(IResourceFactory* factory);

	virtual void removeAllFactories();
	
	virtual Ref< IResourceHandle > bind(const TypeInfo& type, const Guid& guid);

	virtual void update(const Guid& guid, bool force);

	virtual void flush(const Guid& guid);

	virtual void flush();

private:
	RefArray< IResourceFactory > m_factories;
	Ref< IResourceHandle > m_nullHandle;
	std::map< Guid, Ref< ResourceHandle > > m_cache;
	Semaphore m_lock;

	Ref< IResourceFactory > findFactory(const TypeInfo& type);

	void load(const Guid& guid, IResourceFactory* factory, ResourceHandle* handle);
};
	
	}
}

#endif	// traktor_resource_ResourceManager_H
