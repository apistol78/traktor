#ifndef traktor_editor_PipelineBuilder_H
#define traktor_editor_PipelineBuilder_H

#include <map>
#include "Editor/IPipelineBuilder.h"
#include "Core/Heap/Ref.h"
#include "Core/Thread/Event.h"
#include "Core/Io/Path.h"

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

	namespace db
	{

class Group;

	}

	namespace editor
	{

class IPipelineCache;
class PipelineDependency;
class PipelineHash;

/*! \brief Pipeline manager.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineBuilder : public IPipelineBuilder
{
	T_RTTI_CLASS(PipelineBuilder)

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

	PipelineBuilder(
		db::Database* sourceDatabase,
		db::Database* outputDatabase,
		IPipelineCache* cache,
		PipelineHash* hash,
		IListener* listener = 0
	);

	virtual bool build(const RefArray< PipelineDependency >& dependencies, bool rebuild);

	virtual db::Database* getSourceDatabase() const;

	virtual db::Database* getOutputDatabase() const;

	virtual db::Instance* createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid);

	virtual const Serializable* getObjectReadOnly(const Guid& instanceGuid);

private:
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< IPipelineCache > m_cache;
	Ref< PipelineHash > m_hash;
	IListener* m_listener;
	std::map< Guid, Ref< Serializable > > m_readCache;
	std::map< Path, uint32_t > m_externalFileHash;
	RefArray< db::Instance > m_builtInstances;
	int32_t m_succeeded;
	int32_t m_failed;

	/*! \brief Check if dependency needs to be built. */
	bool needBuild(PipelineDependency* dependency) const;

	/*! \brief Isolate instance in cache. */
	bool putInstancesInCache(const Guid& guid, uint32_t hash, const RefArray< db::Instance >& instances);

	/*! \brief Get isolated instance from cache. */
	bool getInstancesFromCache(const Guid& guid, uint32_t hash);

	/*! \brief Calculate dependency cache hash. */
	uint32_t dependencyCacheHash(const PipelineDependency* dependency);

	/*! \brief Calculate external file hash. */
	uint32_t externalFileHash(const Path& path);
};

	}
}

#endif	// traktor_editor_PipelineBuilder_H
