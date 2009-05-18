#ifndef traktor_editor_PipelineManager_H
#define traktor_editor_PipelineManager_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Misc/MD5.h"
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
class Serializable;

	namespace db
	{

class Database;
class Instance;

	}

	namespace editor
	{

class PipelineHash;
class Pipeline;

/*! \brief Pipeline manager. */
class T_DLLCLASS PipelineManager : public Object
{
	T_RTTI_CLASS(PipelineManager)

public:
	struct Listener
	{
		virtual ~Listener() {}

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
		const RefArray< Pipeline >& pipelines,
		PipelineHash* hash,
		Listener* listener = 0
	);

	virtual Pipeline* findPipeline(const Type& sourceType) const;

	virtual void addDependency(
		const Object* sourceAsset
	);

	virtual void addDependency(
		const Object* sourceAsset,
		const std::wstring& name,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		bool build = true
	);

	virtual void addDependency(
		db::Instance* sourceAssetInstance,
		bool build = true
	);

	virtual void addDependency(
		const Guid& sourceAssetGuid,
		bool build = true
	);

	virtual bool build(bool rebuild);

	virtual db::Database* getSourceDatabase() const;

	virtual db::Database* getOutputDatabase() const;

	virtual db::Instance* createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid, Object* object);

	virtual const Serializable* getObjectReadOnly(const Guid& instanceGuid);

	template < typename T >
	const T* getObjectReadOnly(const Guid& guid)
	{
		return dynamic_type_cast< const T* >(getObjectReadOnly(guid));
	}

private:
	struct Dependency : public Object
	{
		std::wstring name;
		Ref< Pipeline > pipeline;
		Ref< const Object > sourceAsset;
		std::wstring outputPath;
		Guid outputGuid;
		Ref< const Object > buildParams;
		RefArray< Dependency > dependencies;
		MD5 checksum;
		bool build;
		uint32_t reason;

		bool needBuild();
	};

	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< PipelineHash > m_hash;
	Listener* m_listener;
	RefArray< Pipeline > m_pipelines;
	RefArray< Dependency > m_dependencies;
	Ref< Dependency > m_currentDependency;
	std::map< Guid, Ref< Serializable > > m_readCache;
	Thread* m_buildThreads[4];
	Semaphore m_buildQueueLock;
	RefList< Dependency > m_buildQueue;
	Event m_buildQueueWr;
	Event m_buildQueueRd;
	int32_t m_succeeded;
	int32_t m_failed;

	/*! \brief Find already added dependency.
	 *
	 * \param guid Output guid.
	 * \return Pointer to added dependency, null if dependency not added.
	 */
	Dependency* findDependency(const Guid& guid) const;

	/*! \brief Add dependency.
	 * Add dependency without checking if it's already added.
	 *
	 * \param sourceAsset Pointer to source asset object.
	 * \param name Name of source asset.
	 * \param outputPath Output path of target instance.
	 * \param outputGuid Guid of output instance.
	 * \param build If asset needs to be built.
	 */
	void addUniqueDependency(const Object* sourceAsset, const std::wstring& name, const std::wstring& outputPath, const Guid& outputGuid, bool build);

	/*! \brief Build thread method. */
	void buildThread();
};

	}
}

#endif	// traktor_editor_PipelineManager_H
