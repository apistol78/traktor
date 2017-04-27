/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_PipelineDependsIncremental_H
#define traktor_editor_PipelineDependsIncremental_H

#include <map>
#include "Core/Timer/Timer.h"
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
	namespace editor
	{

class IPipelineDb;
class IPipelineDependencySet;
class IPipelineInstanceCache;
class PipelineFactory;

/*! \brief Incremental pipeline dependency walker.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineDependsIncremental : public IPipelineDepends
{
	T_RTTI_CLASS;

public:
	PipelineDependsIncremental(
		PipelineFactory* pipelineFactory,
		db::Database* sourceDatabase,
		db::Database* outputDatabase,
		IPipelineDependencySet* dependencySet,
		IPipelineDb* pipelineDb,
		IPipelineInstanceCache* instanceCache,
		uint32_t recursionDepth = ~0U
	);

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
	Ref< PipelineFactory > m_pipelineFactory;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< IPipelineDependencySet > m_dependencySet;
	Ref< IPipelineDb > m_pipelineDb;
	Ref< IPipelineInstanceCache > m_instanceCache;
	uint32_t m_maxRecursionDepth;
	uint32_t m_currentRecursionDepth;
	Ref< PipelineDependency > m_currentDependency;

#if defined(_DEBUG)
	Timer m_timer;
	std::vector< double > m_buildDepTimeStack;
	std::map< const TypeInfo*, std::pair< int32_t, double > > m_buildDepTimes;
#endif

	void addUniqueDependency(
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	);

	void updateDependencyHashes(
		PipelineDependency* dependency,
		const db::Instance* sourceInstance
	);

	void updateDependencyHashesJob(
		PipelineDependency* dependency,
		const db::Instance* sourceInstance
	);
};

	}
}

#endif	// traktor_editor_PipelineDependsIncremental_H
