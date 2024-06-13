/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Thread/ReaderWriterLock.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"
#include "Editor/IPipelineDepends.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Job;

}

namespace traktor::editor
{

class IPipelineDb;
class PipelineDependencySet;
class IPipelineInstanceCache;
class PipelineFactory;

/*! Parallel pipeline dependency walker.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineDependsParallel : public IPipelineDepends
{
	T_RTTI_CLASS;

public:
	explicit PipelineDependsParallel(
		PipelineFactory* pipelineFactory,
		db::Database* sourceDatabase,
		db::Database* outputDatabase,
		PipelineDependencySet* dependencySet,
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

	virtual db::Database* getSourceDatabase() const override final;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) override final;

private:
	RefArray< Job > m_jobs;
	Ref< PipelineFactory > m_pipelineFactory;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< PipelineDependencySet > m_dependencySet;
	Ref< IPipelineDb > m_pipelineDb;
	Ref< IPipelineInstanceCache > m_instanceCache;
	ThreadLocal m_currentDependency;
	ReaderWriterLock m_readCacheLock;
	Semaphore m_jobsLock;
	Semaphore m_dependencySetLock;
	mutable bool m_result;

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
		const IPipeline* pipeline,
		const db::Instance* sourceInstance
	) const;

	void jobAddDependency(Ref< PipelineDependency > parentDependency, Ref< const ISerializable > sourceAsset, std::wstring outputPath, Guid outputGuid, uint32_t flags);

	void jobAddDependency(Ref< PipelineDependency > parentDependency, Ref< db::Instance > sourceAssetInstance, uint32_t flags);

	void jobAddDependency(Ref< PipelineDependency > parentDependency, Guid sourceAssetGuid, uint32_t flags);
};

}
