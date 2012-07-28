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

	virtual void destroy();

	virtual void addFactory(IResourceFactory* factory);

	virtual void removeFactory(IResourceFactory* factory);

	virtual void removeAllFactories();
	
	virtual Ref< IResourceHandle > bind(const TypeInfo& type, const Guid& guid);

	virtual void reload(const Guid& guid);

	virtual void unloadUnusedResident();
	
	virtual void getStatistics(ResourceManagerStatistics& outStatistics) const;

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

	RefArray< IResourceFactory > m_factories;
	std::map< Guid, Ref< ResidentResourceHandle > > m_residentHandles;
	std::map< Guid, RefArray< ExclusiveResourceHandle > > m_exclusiveHandles;
	std::map< const TypeInfo*, TimeCount > m_times;
	std::stack< double > m_timeStack;
	mutable Semaphore m_lock;
	bool m_verbose;

	Ref< IResourceFactory > findFactory(const TypeInfo& type);

	void load(const Guid& guid, IResourceFactory* factory, const TypeInfo& resourceType, IResourceHandle* handle);
};
	
	}
}

#endif	// traktor_resource_ResourceManager_H
