#pragma once

#include <list>
#include <map>
#include "Core/Io/Path.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/ReaderWriterLock.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/PipelineTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

	namespace db
	{

class Group;

	}

	namespace editor
	{

class IPipelineCache;
class IPipelineDb;
class IPipelineInstanceCache;
class PipelineFactory;
class PipelineProfiler;

/*! Pipeline manager.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineBuilder : public IPipelineBuilder
{
	T_RTTI_CLASS;

public:
	PipelineBuilder(
		PipelineFactory* pipelineFactory,
		db::Database* sourceDatabase,
		db::Database* outputDatabase,
		IPipelineCache* cache,
		IPipelineDb* db,
		IPipelineInstanceCache* instanceCache,
		IListener* listener,
		bool threadedBuildEnable,
		bool verbose
	);

	virtual bool build(const PipelineDependencySet* dependencySet, bool rebuild) override final;

	virtual Ref< ISerializable > buildOutput(const db::Instance* sourceInstance, const ISerializable* sourceAsset, const Object* buildParams) override final;

	virtual bool buildAdHocOutput(const ISerializable* sourceAsset, const Guid& outputGuid, const Object* buildParams) override final;

	virtual bool buildAdHocOutput(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, const Object* buildParams) override final;

	virtual uint32_t calculateInclusiveHash(const ISerializable* sourceAsset) const override final;

	virtual Ref< ISerializable > getBuildProduct(const ISerializable* sourceAsset) override final;

	virtual Ref< db::Instance > createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid) override final;

	virtual db::Database* getOutputDatabase() const override final;

	virtual db::Database* getSourceDatabase() const override final;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) override final;

	virtual DataAccessCache* getDataAccessCache() const override final;

	virtual PipelineProfiler* getProfiler() const override final;

private:
	struct WorkEntry
	{
		Ref< const PipelineDependency > dependency;
		Ref< const Object > buildParams;
		uint32_t reason;
	};

	struct BuiltCacheEntry
	{
		const ISerializable* sourceAsset;
		Ref< ISerializable > product;
	};

	typedef std::list< BuiltCacheEntry > built_cache_list_t;

	Ref< PipelineFactory > m_pipelineFactory;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< IPipelineCache > m_cache;
	Ref< IPipelineDb > m_pipelineDb;
	Ref< IPipelineInstanceCache > m_instanceCache;
	Ref< DataAccessCache > m_dataAccessCache;
	IListener* m_listener;
	bool m_threadedBuildEnable;
	bool m_verbose;
	bool m_rebuild;
	Semaphore m_createOutputLock;
	ReaderWriterLock m_readCacheLock;
	Semaphore m_builtCacheLock;
	Semaphore m_workSetLock;
	Ref< PipelineProfiler > m_profiler;
	AlignedVector< WorkEntry > m_workSet;
	std::map< Guid, Ref< ISerializable > > m_readCache;
	std::map< uint32_t, built_cache_list_t > m_builtCache;
	ThreadLocal m_buildInstances;
	int32_t m_progressEnd;
	std::atomic< int32_t > m_progress;
	std::atomic< int32_t > m_succeeded;
	std::atomic< int32_t > m_succeededBuilt;
	std::atomic< int32_t > m_failed;
	std::atomic< int32_t > m_cacheHit;
	std::atomic< int32_t > m_cacheMiss;
	std::atomic< int32_t > m_cacheVoid;

	/*! Perform build. */
	BuildResult performBuild(const PipelineDependencySet* dependencySet, const PipelineDependency* dependency, const Object* buildParams, uint32_t reason);

	/*! Isolate instance in cache. */
	bool putInstancesInCache(const Guid& guid, const PipelineDependencyHash& hash, const RefArray< db::Instance >& instances);

	/*! Get isolated instance from cache. */
	bool getInstancesFromCache(const PipelineDependency* dependency, const PipelineDependencyHash& hash, RefArray< db::Instance >& outInstances);

	/*! Build thread method. */
	void buildThread(const PipelineDependencySet* dependencySet, Thread* controlThread, int32_t cpuCore);
};

	}
}

