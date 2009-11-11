#ifndef traktor_editor_PipelineDependsIncremental_H
#define traktor_editor_PipelineDependsIncremental_H

#include <map>
#include "Editor/IPipelineDepends.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class Settings;

/*! \brief Incremental pipeline dependency walker.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineDependsIncremental : public IPipelineDepends
{
	T_RTTI_CLASS(PipelineDependsIncremental)

public:
	PipelineDependsIncremental(
		Settings* settings,
		db::Database* sourceDatabase,
		uint32_t recursionDepth = ~0UL
	);

	virtual ~PipelineDependsIncremental();

	virtual void addDependency(
		const Serializable* sourceAsset
	);

	virtual void addDependency(
		const Serializable* sourceAsset,
		const std::wstring& name,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		bool build
	);

	virtual void addDependency(
		db::Instance* sourceAssetInstance,
		bool build
	);

	virtual void addDependency(
		const Guid& sourceAssetGuid,
		bool build
	);

	virtual void addDependency(
		const Path& fileName
	);

	virtual void getDependencies(RefArray< PipelineDependency >& outDependencies) const;

	virtual Ref< db::Database > getSourceDatabase() const;

	virtual Ref< const Serializable > getObjectReadOnly(const Guid& instanceGuid);

private:
	Ref< db::Database > m_sourceDatabase;
	std::vector< std::pair< Ref< IPipeline >, uint32_t > > m_pipelines;
	uint32_t m_maxRecursionDepth;
	uint32_t m_currentRecursionDepth;
	RefArray< PipelineDependency > m_dependencies;
	Ref< PipelineDependency > m_currentDependency;
	std::map< Guid, Ref< Serializable > > m_readCache;

	/*! \brief Find pipeline from source type.
	 *
	 * \param sourceType Type of source asset.
	 * \param outPipeline Pipeline.
	 * \param outPipelineHash Hash of pipeline settings.
	 * \return True if pipeline found.
	 */
	bool findPipeline(const Type& sourceType, Ref< IPipeline >& outPipeline, uint32_t& outPipelineHash) const;

	/*! \brief Find already added dependency.
	 *
	 * \param guid Output guid.
	 * \return Pointer to added dependency, null if dependency not added.
	 */
	Ref< PipelineDependency > findDependency(const Guid& guid) const;

	/*! \brief Add dependency.
	 * Add dependency without checking if it's already added.
	 *
	 * \param sourceInstance Source asset database instance; null if not originate from database.
	 * \param sourceAsset Pointer to source asset object.
	 * \param name Name of source asset.
	 * \param outputPath Output path of target instance.
	 * \param outputGuid Guid of output instance.
	 * \param build If asset needs to be built.
	 */
	void addUniqueDependency(
		const db::Instance* sourceInstance,
		const Serializable* sourceAsset,
		const std::wstring& name,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		bool build
	);
};

	}
}

#endif	// traktor_editor_PipelineDependsIncremental_H
