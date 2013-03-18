#ifndef traktor_editor_PipelineDependsIncremental_H
#define traktor_editor_PipelineDependsIncremental_H

#include <map>
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

class PipelineDependencyCache;
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
		PipelineDependencyCache* dependencyCache,
		db::Database* sourceDatabase,
		uint32_t recursionDepth = ~0UL
	);

	virtual void addDependency(
		const ISerializable* sourceAsset
	);

	virtual void addDependency(
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	);

	virtual void addDependency(
		db::Instance* sourceAssetInstance,
		uint32_t flags
	);

	virtual void addDependency(
		const Guid& sourceAssetGuid,
		uint32_t flags
	);

	virtual void addDependency(
		const Path& basePath,
		const std::wstring& fileName
	);

	virtual void addDependency(
		const TypeInfo& sourceAssetType
	);

	virtual bool waitUntilFinished();

	virtual void getDependencies(RefArray< PipelineDependency >& outDependencies) const;

	virtual Ref< db::Database > getSourceDatabase() const;

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid);

private:
	Ref< PipelineFactory > m_pipelineFactory;
	Ref< PipelineDependencyCache > m_dependencyCache;
	Ref< db::Database > m_sourceDatabase;
	uint32_t m_maxRecursionDepth;
	uint32_t m_currentRecursionDepth;
	RefArray< PipelineDependency > m_dependencies;
	Ref< PipelineDependency > m_currentDependency;
	std::map< Guid, Ref< ISerializable > > m_readCache;
	std::map< Guid, PipelineDependency* > m_dependencyMap;

	PipelineDependency* findDependency(const Guid& guid) const;

	void addUniqueDependency(
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		uint32_t flags
	);

	void addCachedDependency(PipelineDependency* dependency);

	void updateDependencyHashes(
		PipelineDependency* dependency,
		const db::Instance* sourceInstance
	) const;
};

	}
}

#endif	// traktor_editor_PipelineDependsIncremental_H
