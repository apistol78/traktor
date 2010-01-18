#ifndef traktor_editor_PipelineBuilder_H
#define traktor_editor_PipelineBuilder_H

#include <map>
#include "Editor/IPipelineBuilder.h"
#include "Core/Thread/Event.h"
#include "Core/Io/Path.h"

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
class PipelineDependency;
class PipelineFactory;

/*! \brief Pipeline manager.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineBuilder : public IPipelineBuilder
{
	T_RTTI_CLASS;

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
		PipelineFactory* pipelineFactory,
		db::Database* sourceDatabase,
		db::Database* outputDatabase,
		IPipelineCache* cache,
		IPipelineDb* db,
		IListener* listener = 0
	);

	virtual bool build(const RefArray< PipelineDependency >& dependencies, bool rebuild);

	virtual bool buildOutput(const ISerializable* sourceAsset, const Object* buildParams, const std::wstring& name, const std::wstring& outputPath, const Guid& outputGuid);

	virtual Ref< db::Database > getSourceDatabase() const;

	virtual Ref< db::Database > getOutputDatabase() const;

	virtual Ref< db::Instance > createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid);

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid);

	virtual Ref< IPipelineReport > createReport(const std::wstring& name, const Guid& guid);

private:
	Ref< PipelineFactory > m_pipelineFactory;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< IPipelineCache > m_cache;
	Ref< IPipelineDb > m_db;
	IListener* m_listener;
	std::map< Guid, Ref< ISerializable > > m_readCache;
	std::map< Path, uint32_t > m_externalFileHash;
	RefArray< db::Instance > m_builtInstances;

	/*! \brief Update build reasons. */
	void analyzeBuildReason(PipelineDependency* dependency, bool rebuild);

	/*! \brief Perform build. */
	bool performBuild(PipelineDependency* dependency);

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
