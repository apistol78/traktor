#ifndef traktor_editor_PipelineBuilder_H
#define traktor_editor_PipelineBuilder_H

#include <map>
#include "Editor/IPipelineBuilder.h"
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
	Ref< PipelineHash > m_hash;
	IListener* m_listener;
	std::map< Guid, Ref< Serializable > > m_readCache;
	int32_t m_succeeded;
	int32_t m_failed;

	/*! \brief Check if dependency needs to be built. */
	bool needBuild(PipelineDependency* dependency) const;
};

	}
}

#endif	// traktor_editor_PipelineBuilder_H
