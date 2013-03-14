#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Editor/IPipelineDb.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/PipelineAgent.h"
#include "Editor/Pipeline/PipelineAgentsManager.h"
#include "Editor/Pipeline/PipelineBuilderDistributed.h"
#include "Editor/Pipeline/PipelineDependency.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineBuilderDistributed", PipelineBuilderDistributed, IPipelineBuilder)

PipelineBuilderDistributed::PipelineBuilderDistributed(
	PipelineAgentsManager* agentsManager,
	PipelineFactory* pipelineFactory,
	IPipelineDb* db,
	IListener* listener
)
:	m_agentsManager(agentsManager)
,	m_pipelineFactory(pipelineFactory)
,	m_db(db)
,	m_listener(listener)
,	m_progress(0)
,	m_progressEnd(0)
,	m_succeeded(0)
,	m_failed(0)
{
}

bool PipelineBuilderDistributed::build(const RefArray< PipelineDependency >& dependencies, bool rebuild)
{
	m_db->beginTransaction();

	// Update dependency hashes.
	for (RefArray< PipelineDependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		updateLocalHashes(*i);

	// Check which dependencies are dirty; ie. need to be rebuilt.
	for (RefArray< PipelineDependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		updateBuildReason(*i, rebuild);

	m_db->endTransaction();

	m_progress = 0;
	m_progressEnd = dependencies.size();
	m_succeeded = 0;
	m_failed = 0;

	m_db->beginTransaction();

	RefArray< PipelineDependency > workSet = dependencies;
	while (!workSet.empty())
	{
		if (performBuild(workSet.back()))
			++m_succeeded;
		else
			++m_failed;
		workSet.pop_back();
	}

	log::info << L"Waiting for agents to complete build..." << Endl;
	m_agentsManager->waitUntilAllIdle();

	m_db->endTransaction();

	// Log results.
	if (!ThreadManager::getInstance().getCurrentThread()->stopped())
		log::info << L"Build finished; " << m_succeeded << L" succeeded, " << m_failed << L" failed" << Endl;
	else
		log::info << L"Build finished; aborted" << Endl;

	return m_failed == 0;
}

Ref< ISerializable > PipelineBuilderDistributed::buildOutput(const ISerializable* sourceAsset)
{
	T_FATAL_ERROR;
	return 0;
}

bool PipelineBuilderDistributed::buildOutput(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, const Object* buildParams)
{
	T_FATAL_ERROR;
	return false;
}

Ref< db::Database > PipelineBuilderDistributed::getSourceDatabase() const
{
	T_FATAL_ERROR;
	return 0;
}

Ref< db::Database > PipelineBuilderDistributed::getOutputDatabase() const
{
	T_FATAL_ERROR;
	return 0;
}

Ref< db::Instance > PipelineBuilderDistributed::createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid)
{
	T_FATAL_ERROR;
	return 0;
}

Ref< const ISerializable > PipelineBuilderDistributed::getObjectReadOnly(const Guid& instanceGuid)
{
	T_FATAL_ERROR;
	return 0;
}

Ref< IStream > PipelineBuilderDistributed::openFile(const Path& basePath, const std::wstring& fileName)
{
	T_FATAL_ERROR;
	return 0;
}

Ref< IStream > PipelineBuilderDistributed::createTemporaryFile(const std::wstring& fileName)
{
	T_FATAL_ERROR;
	return 0;
}

Ref< IStream > PipelineBuilderDistributed::openTemporaryFile(const std::wstring& fileName)
{
	T_FATAL_ERROR;
	return 0;
}

Ref< IPipelineReport > PipelineBuilderDistributed::createReport(const std::wstring& name, const Guid& guid)
{
	T_FATAL_ERROR;
	return 0;
}

void PipelineBuilderDistributed::updateLocalHashes(PipelineDependency* dependency)
{
	IPipelineDb::FileHash previousFileHash;

	dependency->sourceAssetHash = DeepHash(dependency->sourceAsset).get();
	dependency->dependencyHash = 0;

	const std::set< Path >& files = dependency->files;
	for (std::set< Path >::const_iterator j = files.begin(); j != files.end(); ++j)
	{
		Ref< File > file = FileSystem::getInstance().get(*j);
		if (!file)
		{
			log::error << L"Unable to get hash of file \"" << j->getPathName() << L"\"; no such file" << Endl;
			continue;
		}

		if (m_db->getFile(*j, previousFileHash))
		{
			if (
				previousFileHash.size == file->getSize() &&
				previousFileHash.lastWriteTime == file->getLastWriteTime()
			)
			{
				// Reuse previous file hash from cache.
				dependency->dependencyHash += previousFileHash.hash;
				continue;
			}
		}
		else
		{
			// File doesn't exist in cache; ensure cycle is reset.
			previousFileHash.hash = 0;
		}

		// File has either been modified or is new; calculate hash and update cache.
		previousFileHash.size = file->getSize();
		previousFileHash.lastWriteTime = file->getLastWriteTime();
		previousFileHash.hash++;

		m_db->setFile(*j, previousFileHash);

		dependency->dependencyHash += previousFileHash.hash;
	}

	dependency->dependencyHash += dependency->pipelineHash;
	dependency->dependencyHash += dependency->sourceDataHash;
	dependency->dependencyHash += dependency->sourceAssetHash;
}

void PipelineBuilderDistributed::updateBuildReason(PipelineDependency* dependency, bool rebuild)
{
	dependency->sourceAssetHash = DeepHash(checked_type_cast< const ISerializable* >(dependency->sourceAsset)).get();

	// Have source asset been modified?
	if (!rebuild)
	{
		uint32_t dependencyHash = calculateGlobalHash(dependency);

		// Get hash entry from database.
		IPipelineDb::DependencyHash previousDependencyHash;
		if (!m_db->getDependency(dependency->outputGuid, previousDependencyHash))
		{
			log::info << L"Asset \"" << dependency->outputPath << L"\" modified; not hashed" << Endl;
			dependency->reason |= PbrSourceModified;
		}
		else if (previousDependencyHash.pipelineVersion != type_of(dependency->pipeline).getVersion())
		{
			log::info << L"Asset \"" << dependency->outputPath << L"\" modified; pipeline version differ" << Endl;
			dependency->reason |= PbrSourceModified;
		}
		else if (previousDependencyHash.hash != dependencyHash)
		{
			log::info << L"Asset \"" << dependency->outputPath << L"\" modified; source has been modified" << Endl;
			dependency->reason |= PbrSourceModified;
		}
	}
	else
		dependency->reason |= PbrForced;
}

bool PipelineBuilderDistributed::performBuild(PipelineDependency* dependency)
{
	IPipelineDb::DependencyHash currentDependencyHash;
	bool result = true;

	// Create hash entry.
	currentDependencyHash.pipelineVersion = type_of(dependency->pipeline).getVersion();
	currentDependencyHash.hash = calculateGlobalHash(dependency);

	// Skip no-build asset; just update hash.
	if ((dependency->flags & PdfBuild) == 0)
	{
		if ((dependency->reason & PbrSourceModified) != 0)
			m_db->setDependency(dependency->outputGuid, currentDependencyHash);
		return true;
	}

	// Check if we need to build asset; check the entire dependency chain (will update reason if dependency dirty).
	if (needBuild(dependency))
	{
		Ref< PipelineAgent > agent = m_agentsManager->getIdleAgent();
		if (!agent)
			return false;

		log::info << L"Building asset \"" << dependency->outputPath << L"\" (" << type_name(dependency->pipeline) << L") on agent " << agent->getDescription() << L"..." << Endl;

		int32_t agentIndex = m_agentsManager->getAgentIndex(agent);

		if (m_listener)
			m_listener->beginBuild(
				agentIndex,
				m_progress,
				m_progressEnd,
				dependency
			);

		result = agent->build(
			dependency,
			makeFunctor(
				this,
				&PipelineBuilderDistributed::agentBuildSucceeded,
				dependency,
				currentDependencyHash.pipelineVersion,
				currentDependencyHash.hash,
				agentIndex
			),
			makeFunctor(
				this,
				&PipelineBuilderDistributed::agentBuildFailed,
				dependency,
				agentIndex
			)
		);
	}

	return result;
}

uint32_t PipelineBuilderDistributed::calculateGlobalHash(const PipelineDependency* dependency) const
{
	uint32_t hash = dependency->dependencyHash;
	for (RefArray< PipelineDependency >::const_iterator i = dependency->children.begin(); i != dependency->children.end(); ++i)
	{
		if (((*i)->flags & PdfUse) != 0)
			hash += calculateGlobalHash(*i);
	}
	return hash;
}

bool PipelineBuilderDistributed::needBuild(PipelineDependency* dependency) const
{
	if (dependency->reason != PbrNone)
		return true;

	for (RefArray< PipelineDependency >::const_iterator i = dependency->children.begin(); i != dependency->children.end(); ++i)
	{
		// Skip non-use dependencies; non-used dependencies are not critical for building
		// given dependency.
		if (((*i)->flags & PdfUse) == 0)
			continue;

		if (needBuild(*i))
		{
			dependency->reason |= PbrDependencyModified;
			return true;
		}
	}

	return false;
}

void PipelineBuilderDistributed::agentBuildSucceeded(PipelineDependency* dependency, uint32_t pipelineVersion, uint32_t hash, int32_t agentIndex)
{
	IPipelineDb::DependencyHash dependencyHash;
	dependencyHash.pipelineVersion = pipelineVersion;
	dependencyHash.hash = hash;
	m_db->setDependency(dependency->outputGuid, dependencyHash);

	if (m_listener)
		m_listener->endBuild(
			agentIndex,
			m_progress,
			m_progressEnd,
			dependency,
			BrSucceeded
		);
}

void PipelineBuilderDistributed::agentBuildFailed(PipelineDependency* dependency, int32_t agentIndex)
{
	if (m_listener)
		m_listener->endBuild(
			agentIndex,
			m_progress,
			m_progressEnd,
			dependency,
			BrFailed
		);
}

	}
}
