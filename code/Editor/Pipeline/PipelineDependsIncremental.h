#pragma once

#include "Core/Timer/Timer.h"
#include "Editor/IPipelineDepends.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IPipelineDb;
class PipelineDependencySet;
class IPipelineInstanceCache;
class PipelineFactory;

/*! Incremental pipeline dependency walker.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineDependsIncremental : public IPipelineDepends
{
	T_RTTI_CLASS;

public:
	explicit PipelineDependsIncremental(
		PipelineFactory* pipelineFactory,
		db::Database* sourceDatabase,
		db::Database* outputDatabase,
		PipelineDependencySet* dependencySet,
		IPipelineDb* pipelineDb,
		IPipelineInstanceCache* instanceCache,
		const PipelineDependencySet* excludeDependencySet = nullptr,
		uint32_t recursionDepth = ~0U
	);

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
	Ref< PipelineFactory > m_pipelineFactory;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< PipelineDependencySet > m_dependencySet;
	Ref< IPipelineDb > m_pipelineDb;
	Ref< IPipelineInstanceCache > m_instanceCache;
	Ref< const PipelineDependencySet > m_excludeDependencySet;
	uint32_t m_maxRecursionDepth;
	uint32_t m_currentRecursionDepth;
	Ref< PipelineDependency > m_currentDependency;
	bool m_result;

	void addUniqueDependency(
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	);

	void updateDependencyHashes(
		PipelineDependency* dependency,
		const IPipeline* pipeline,
		const db::Instance* sourceInstance
	);
};

}
