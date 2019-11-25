#pragma once

#include "Core/Thread/ReaderWriterLock.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"
#include "Editor/IPipelineDepends.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
	#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Job;

	namespace editor
	{

class IPipelineDb;
class IPipelineDependencySet;
class IPipelineInstanceCache;
class PipelineFactory;

/*! \brief Parallel pipeline dependency walker.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineDependsParallel : public IPipelineDepends
{
	T_RTTI_CLASS;

public:
	PipelineDependsParallel(
		PipelineFactory* pipelineFactory,
		db::Database* sourceDatabase,
		IPipelineDependencySet* dependencySet,
		IPipelineDb* pipelineDb,
		IPipelineInstanceCache* instanceCache
	);

	virtual ~PipelineDependsParallel();

	virtual void addDependency(
		const ISerializable* sourceAsset
	) override final;

	virtual void addDependency(
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	) override final;

	virtual void addDependency(
		db::Instance* sourceAssetInstance,
		uint32_t flags
	) override final;

	virtual void addDependency(
		const Guid& sourceAssetGuid,
		uint32_t flags
	) override final;

	virtual void addDependency(
		const Path& basePath,
		const std::wstring& fileName
	) override final;

	virtual void addDependency(
		const TypeInfo& sourceAssetType
	) override final;

	virtual bool waitUntilFinished() override final;

	virtual Ref< db::Database > getSourceDatabase() const override final;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) override final;

	virtual Ref< File > getFile(const Path& basePath, const std::wstring& fileName) override final;

	virtual Ref< IStream > openFile(const Path& basePath, const std::wstring& fileName) override final;

	virtual Ref< IStream > createTemporaryFile(const std::wstring& fileName) override final;

	virtual Ref< IStream > openTemporaryFile(const std::wstring& fileName) override final;

private:
	RefArray< Job > m_jobs;
	Ref< PipelineFactory > m_pipelineFactory;
	Ref< db::Database > m_sourceDatabase;
	Ref< IPipelineDependencySet > m_dependencySet;
	Ref< IPipelineDb > m_pipelineDb;
	Ref< IPipelineInstanceCache > m_instanceCache;
	ThreadLocal m_currentDependency;
	ReaderWriterLock m_readCacheLock;
	Semaphore m_jobsLock;
	Semaphore m_dependencySetLock;

	Ref< PipelineDependency > findOrCreateDependency(
		const Guid& guid,
		PipelineDependency* parentDependency,
		uint32_t flags,
		bool& outExists
	);

	void addUniqueDependency(
		PipelineDependency* parentDependency,
		PipelineDependency* currentDependency,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	);

	void updateDependencyHashes(
		PipelineDependency* dependency,
		const db::Instance* sourceInstance
	) const;

	void jobAddDependency(Ref< PipelineDependency > parentDependency, Ref< const ISerializable > sourceAsset, std::wstring outputPath, Guid outputGuid, uint32_t flags);

	void jobAddDependency(Ref< PipelineDependency > parentDependency, Ref< db::Instance > sourceAssetInstance, uint32_t flags);

	void jobAddDependency(Ref< PipelineDependency > parentDependency, Guid sourceAssetGuid, uint32_t flags);
};

	}
}

