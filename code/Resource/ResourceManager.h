#pragma once

#include <utility>
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
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
	namespace db
	{

class Database;
class Instance;

	}

	namespace resource
	{

class ExclusiveResourceHandle;
class ResidentResourceHandle;
class ResourceHandle;

/*! Resource manager.
 * \ingroup Resource
 */
class T_DLLCLASS ResourceManager : public IResourceManager
{
	T_RTTI_CLASS;

public:
	explicit ResourceManager(db::Database* database, bool verbose);

	virtual ~ResourceManager();

	virtual void destroy() override final;

	virtual void addFactory(const IResourceFactory* factory) override final;

	virtual void removeFactory(const IResourceFactory* factory) override final;

	virtual void removeAllFactories() override final;

	virtual bool load(const ResourceBundle* bundle) override final;

	virtual Ref< ResourceHandle > bind(const TypeInfo& productType, const Guid& guid) override final;

	virtual bool reload(const Guid& guid, bool flushedOnly) override final;

	virtual void reload(const TypeInfo& productType, bool flushedOnly) override final;

	virtual void unload(const TypeInfo& productType) override final;

	virtual void unloadUnusedResident() override final;

	virtual void getStatistics(ResourceManagerStatistics& outStatistics) const override final;

private:
	Ref< db::Database > m_database;
	AlignedVector< std::pair< const TypeInfo*, Ref< const IResourceFactory > > > m_resourceFactories;
	SmallMap< Guid, Ref< ResidentResourceHandle > > m_residentHandles;
	SmallMap< Guid, RefArray< ExclusiveResourceHandle > > m_exclusiveHandles;
	mutable Semaphore m_lock;
	bool m_verbose;

	const IResourceFactory* findFactory(const TypeInfo& resourceType) const;

	void load(const db::Instance* instance, const IResourceFactory* factory, const TypeInfo& productType, ResourceHandle* handle);
};

	}
}

