#ifndef traktor_resource_ResourceManager_H
#define traktor_resource_ResourceManager_H

#include <map>
#include <stack>
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

class ExclusiveResourceHandle;
class ResidentResourceHandle;
class ResourceHandle;

/*! \brief Resource manager.
 * \ingroup Resource
 */
class T_DLLCLASS ResourceManager : public IResourceManager
{
	T_RTTI_CLASS;

public:
	ResourceManager(bool verbose);

	virtual ~ResourceManager();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void addFactory(const IResourceFactory* factory) T_OVERRIDE T_FINAL;

	virtual void removeFactory(const IResourceFactory* factory) T_OVERRIDE T_FINAL;

	virtual void removeAllFactories() T_OVERRIDE T_FINAL;

	virtual bool load(const ResourceBundle* bundle) T_OVERRIDE T_FINAL;
	
	virtual Ref< ResourceHandle > bind(const TypeInfo& type, const Guid& guid) T_OVERRIDE T_FINAL;

	virtual void reload(const Guid& guid, bool flushedOnly) T_OVERRIDE T_FINAL;

	virtual void reload(const TypeInfo& type, bool flushedOnly) T_OVERRIDE T_FINAL;

	virtual void unload(const TypeInfo& type) T_OVERRIDE T_FINAL;

	virtual void unloadUnusedResident() T_OVERRIDE T_FINAL;
	
	virtual void getStatistics(ResourceManagerStatistics& outStatistics) const T_OVERRIDE T_FINAL;

private:
	struct TimeCount
	{
		uint32_t count;
		double time;

		TimeCount()
		:	count(0)
		,	time(0.0)
		{
		}
	};

	std::map< const TypeInfo*, Ref< const IResourceFactory > > m_resourceToFactory;
	std::map< const TypeInfo*, Ref< const IResourceFactory > > m_productToFactory;
	std::map< Guid, Ref< ResidentResourceHandle > > m_residentHandles;
	std::map< Guid, RefArray< ExclusiveResourceHandle > > m_exclusiveHandles;
	std::map< const TypeInfo*, TimeCount > m_times;
	std::stack< double > m_timeStack;
	mutable Semaphore m_lock;
	bool m_verbose;

	const IResourceFactory* findFactoryFromResourceType(const TypeInfo& type);

	const IResourceFactory* findFactoryFromProductType(const TypeInfo& type);

	void load(const Guid& guid, const IResourceFactory* factory, const TypeInfo& resourceType, ResourceHandle* handle);
};
	
	}
}

#endif	// traktor_resource_ResourceManager_H
