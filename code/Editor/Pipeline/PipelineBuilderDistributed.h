#ifndef traktor_editor_PipelineBuilderDistributed_H
#define traktor_editor_PipelineBuilderDistributed_H

#include "Editor/IPipelineBuilder.h"

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

	namespace editor
	{

class IPipelineDb;
class PipelineAgentsManager;
class PipelineDependency;
class PipelineFactory;

/*! \brief Distributed pipeline builder.
 * \ingroup Editor
 */
class T_DLLCLASS PipelineBuilderDistributed : public IPipelineBuilder
{
	T_RTTI_CLASS;

public:
	PipelineBuilderDistributed(
		PipelineAgentsManager* agentsManager,
		PipelineFactory* pipelineFactory,
		IPipelineDb* db,
		IListener* listener
	);

	virtual bool build(const IPipelineDependencySet* dependencySet, bool rebuild);

	virtual Ref< ISerializable > buildOutput(const ISerializable* sourceAsset);

	virtual bool buildOutput(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, const Object* buildParams);

	virtual Ref< db::Database > getSourceDatabase() const;

	virtual Ref< db::Database > getOutputDatabase() const;

	virtual Ref< db::Instance > createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid);

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid);

	virtual Ref< IStream > openFile(const Path& basePath, const std::wstring& fileName);

	virtual Ref< IStream > createTemporaryFile(const std::wstring& fileName);

	virtual Ref< IStream > openTemporaryFile(const std::wstring& fileName);

	virtual Ref< IPipelineReport > createReport(const std::wstring& name, const Guid& guid);

private:
	Ref< PipelineAgentsManager > m_agentsManager;
	Ref< PipelineFactory > m_pipelineFactory;
	Ref< IPipelineDb > m_pipelineDb;
	IListener* m_listener;
	int32_t m_progress;
	int32_t m_progressEnd;
	int32_t m_succeeded;
	int32_t m_failed;

	/*! \brief Perform build. */
	bool performBuild(const IPipelineDependencySet* dependencySet, const PipelineDependency* dependency);

	/*! \brief */
	void agentBuildSucceeded(const PipelineDependency* dependency, uint32_t pipelineVersion, uint32_t hash, int32_t agentIndex);

	void agentBuildFailed(const PipelineDependency* dependency, int32_t agentIndex);
};

	}
}

#endif	// traktor_editor_PipelineBuilderDistributed_H
