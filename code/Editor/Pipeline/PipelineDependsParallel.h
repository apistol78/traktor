/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_PipelineDependsParallel_H
#define traktor_editor_PipelineDependsParallel_H

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

class JobQueue;

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
		db::Database* outputDatabase,
		IPipelineDependencySet* dependencySet,
		IPipelineDb* pipelineDb,
		IPipelineInstanceCache* instanceCache
	);

	virtual ~PipelineDependsParallel();

	virtual void addDependency(
		const ISerializable* sourceAsset
	) T_OVERRIDE T_FINAL;

	virtual void addDependency(
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	) T_OVERRIDE T_FINAL;

	virtual void addDependency(
		db::Instance* sourceAssetInstance,
		uint32_t flags
	) T_OVERRIDE T_FINAL;

	virtual void addDependency(
		const Guid& sourceAssetGuid,
		uint32_t flags
	) T_OVERRIDE T_FINAL;

	virtual void addDependency(
		const Path& basePath,
		const std::wstring& fileName
	) T_OVERRIDE T_FINAL;

	virtual void addDependency(
		const TypeInfo& sourceAssetType
	) T_OVERRIDE T_FINAL;

	virtual bool waitUntilFinished() T_OVERRIDE T_FINAL;

	virtual Ref< db::Database > getSourceDatabase() const T_OVERRIDE T_FINAL;

	virtual Ref< db::Database > getOutputDatabase() const T_OVERRIDE T_FINAL;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > openFile(const Path& basePath, const std::wstring& fileName) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > createTemporaryFile(const std::wstring& fileName) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > openTemporaryFile(const std::wstring& fileName) T_OVERRIDE T_FINAL;

private:
	Ref< JobQueue > m_jobQueue;
	Ref< PipelineFactory > m_pipelineFactory;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< IPipelineDependencySet > m_dependencySet;
	Ref< IPipelineDb > m_pipelineDb;
	Ref< IPipelineInstanceCache > m_instanceCache;
	ThreadLocal m_currentDependency;
	ReaderWriterLock m_readCacheLock;
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

#endif	// traktor_editor_PipelineDependsParallel_H
