#ifndef traktor_editor_PipelineManager_H
#define traktor_editor_PipelineManager_H

#include <map>
#include "Editor/IPipelineManager.h"
#include "Core/Heap/Ref.h"
#include "Core/Thread/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

	namespace editor
	{

class PipelineDependency;
class PipelineHash;

/*! \brief Pipeline manager.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineManager : public IPipelineManager
{
	T_RTTI_CLASS(PipelineManager)

public:
	struct IListener
	{
		virtual ~IListener() {}

		/*! \brief Called when an asset is about to be built.
		 *
		 * \param assetName Human readable name of asset.
		 * \param index Index of asset.
		 * \param count Number of assets to build.
		 */
		virtual void begunBuildingAsset(
			const std::wstring& assetName,
			uint32_t index,
			uint32_t count
		) const = 0;
	};

	PipelineManager(
		db::Database* sourceDatabase,
		db::Database* outputDatabase,
		IPipelineCache* cache,
		const RefArray< IPipeline >& pipelines,
		PipelineHash* hash,
		IListener* listener = 0,
		uint32_t recursionDepth = ~0UL
	);

	virtual IPipeline* findPipeline(const Type& sourceType) const;

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

	virtual bool build(bool rebuild);

	virtual db::Database* getSourceDatabase() const;

	virtual db::Database* getOutputDatabase() const;

	virtual IPipelineCache* getCache() const;

	virtual db::Instance* createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid);

	virtual const Serializable* getObjectReadOnly(const Guid& instanceGuid);

private:
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< IPipelineCache > m_cache;
	Ref< PipelineHash > m_hash;
	IListener* m_listener;
	uint32_t m_maxRecursionDepth;
	uint32_t m_currentRecursionDepth;
	RefArray< IPipeline > m_pipelines;
	RefArray< PipelineDependency > m_dependencies;
	Ref< PipelineDependency > m_currentDependency;
	std::map< Guid, Ref< Serializable > > m_readCache;
	int32_t m_succeeded;
	int32_t m_failed;

	/*! \brief Find already added dependency.
	 *
	 * \param guid Output guid.
	 * \return Pointer to added dependency, null if dependency not added.
	 */
	PipelineDependency* findDependency(const Guid& guid) const;

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

	/*! \brief Check if dependency needs to be built. */
	bool needBuild(PipelineDependency* dependency) const;
};

	}
}

#endif	// traktor_editor_PipelineManager_H
