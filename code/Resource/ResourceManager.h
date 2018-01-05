/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_resource_ResourceManager_H
#define traktor_resource_ResourceManager_H

#include <map>
#include <vector>
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

/*! \brief Resource manager.
 * \ingroup Resource
 */
class T_DLLCLASS ResourceManager : public IResourceManager
{
	T_RTTI_CLASS;

public:
	ResourceManager(db::Database* database, bool verbose);

	virtual ~ResourceManager();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void addFactory(const IResourceFactory* factory) T_OVERRIDE T_FINAL;

	virtual void removeFactory(const IResourceFactory* factory) T_OVERRIDE T_FINAL;

	virtual void removeAllFactories() T_OVERRIDE T_FINAL;

	virtual bool load(const ResourceBundle* bundle) T_OVERRIDE T_FINAL;
	
	virtual Ref< ResourceHandle > bind(const TypeInfo& productType, const Guid& guid) T_OVERRIDE T_FINAL;

	virtual void reload(const Guid& guid, bool flushedOnly) T_OVERRIDE T_FINAL;

	virtual void reload(const TypeInfo& productType, bool flushedOnly) T_OVERRIDE T_FINAL;

	virtual void unload(const TypeInfo& productType) T_OVERRIDE T_FINAL;

	virtual void unloadUnusedResident() T_OVERRIDE T_FINAL;
	
	virtual void getStatistics(ResourceManagerStatistics& outStatistics) const T_OVERRIDE T_FINAL;

private:
	Ref< db::Database > m_database;
	std::vector< std::pair< const TypeInfo*, Ref< const IResourceFactory > > > m_resourceFactories;
	std::map< Guid, Ref< ResidentResourceHandle > > m_residentHandles;
	std::map< Guid, RefArray< ExclusiveResourceHandle > > m_exclusiveHandles;
	mutable Semaphore m_lock;
	bool m_verbose;

	const IResourceFactory* findFactory(const TypeInfo& resourceType) const;

	void load(const db::Instance* instance, const IResourceFactory* factory, const TypeInfo& productType, ResourceHandle* handle);
};
	
	}
}

#endif	// traktor_resource_ResourceManager_H
