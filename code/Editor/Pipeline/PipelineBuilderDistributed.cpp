/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Editor/IPipeline.h"
#include "Editor/IPipelineDb.h"
#include "Editor/IPipelineDependencySet.h"
#include "Editor/PipelineDependency.h"
#include "Editor/Pipeline/PipelineAgent.h"
#include "Editor/Pipeline/PipelineAgentsManager.h"
#include "Editor/Pipeline/PipelineBuilderDistributed.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

void calculateGlobalHash(
	const IPipelineDependencySet* dependencySet,
	const PipelineDependency* dependency,
	uint32_t& outPipelineHash,
	uint32_t& outSourceAssetHash,
	uint32_t& outSourceDataHash,
	uint32_t& outFilesHash
)
{
	outPipelineHash += dependency->pipelineHash;
	outSourceAssetHash += dependency->sourceAssetHash;
	outSourceDataHash += dependency->sourceDataHash;
	outFilesHash += dependency->filesHash;

	for (SmallSet< uint32_t >::const_iterator i = dependency->children.begin(); i != dependency->children.end(); ++i)
	{
		const PipelineDependency* childDependency = dependencySet->get(*i);
		T_ASSERT (childDependency);

		if (childDependency == dependency)
			continue;

		if ((childDependency->flags & PdfUse) != 0)
			calculateGlobalHash(
				dependencySet,
				childDependency,
				outPipelineHash,
				outSourceAssetHash,
				outSourceDataHash,
				outFilesHash
			);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineBuilderDistributed", PipelineBuilderDistributed, IPipelineBuilder)

PipelineBuilderDistributed::PipelineBuilderDistributed(
	PipelineAgentsManager* agentsManager,
	PipelineFactory* pipelineFactory,
	IPipelineDb* pipelineDb,
	IListener* listener
)
:	m_agentsManager(agentsManager)
,	m_pipelineFactory(pipelineFactory)
,	m_pipelineDb(pipelineDb)
,	m_listener(listener)
,	m_progress(0)
,	m_progressEnd(0)
,	m_succeeded(0)
,	m_failed(0)
{
}

bool PipelineBuilderDistributed::build(const IPipelineDependencySet* dependencySet, bool rebuild)
{
	std::vector< uint32_t > reasons;

	// Determine build reasons.
	uint32_t dependencyCount = dependencySet->size();
	reasons.resize(dependencyCount, 0);

	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const PipelineDependency* dependency = dependencySet->get(i);
		T_ASSERT (dependency);

		if ((dependency->flags & PdfFailed) != 0)
			continue;

		// Have source asset been modified?
		if (!rebuild)
		{
			uint32_t pipelineHash = 0;
			uint32_t sourceAssetHash = 0;
			uint32_t sourceDataHash = 0;
			uint32_t filesHash = 0;

			calculateGlobalHash(
				dependencySet,
				dependency,
				pipelineHash,
				sourceAssetHash,
				sourceDataHash,
				filesHash
			);

			// Get hash entry from database.
			PipelineDependencyHash previousDependencyHash;
			if (!m_pipelineDb->getDependency(dependency->outputGuid, previousDependencyHash))
			{
				log::info << L"Asset \"" << dependency->outputPath << L"\" modified; not hashed." << Endl;
				reasons[i] |= PbrSourceModified;
			}
			else if (
				previousDependencyHash.pipelineHash != pipelineHash ||
				previousDependencyHash.sourceAssetHash != sourceAssetHash ||
				previousDependencyHash.sourceDataHash != sourceDataHash ||
				previousDependencyHash.filesHash != filesHash
			)
			{
				log::info << L"Asset \"" << dependency->outputPath << L"\" modified; source has been modified (or new pipeline version)." << Endl;
				reasons[i] |= PbrSourceModified;
			}
		}
		else
			reasons[i] |= PbrForced;
	}

	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const PipelineDependency* dependency = dependencySet->get(i);
		T_ASSERT (dependency);

		if ((dependency->flags & PdfFailed) != 0)
			continue;

		SmallSet< uint32_t > visited;
		visited.insert(i);

		AlignedVector< uint32_t > children;
		children.insert(children.end(), dependency->children.begin(), dependency->children.end());

		while (!children.empty())
		{
			if (visited.find(children.back()) != visited.end())
			{
				children.pop_back();
				continue;
			}

			const PipelineDependency* childDependency = dependencySet->get(children.back());
			T_ASSERT (childDependency);

			if ((childDependency->flags & PdfUse) == 0)
			{
				children.pop_back();
				continue;
			}

			if ((reasons[children.back()] & PbrSourceModified) != 0)
				reasons[i] |= PbrDependencyModified;

			visited.insert(children.back());

			children.pop_back();
			children.insert(children.end(), childDependency->children.begin(), childDependency->children.end());
		}
	}

	m_progress = 0;
	m_progressEnd = dependencyCount;
	m_succeeded = 0;
	m_failed = 0;

	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		if (reasons[i] != 0)
		{
			const PipelineDependency* dependency = dependencySet->get(i);
			T_ASSERT (dependency);

			if ((dependency->flags & PdfFailed) != 0)
				continue;

			if (performBuild(dependencySet, dependency))
				++m_succeeded;
			else
				++m_failed;
		}
		else
			++m_succeeded;
	}

	log::info << L"Waiting for agents to complete build..." << Endl;
	m_agentsManager->waitUntilAllIdle();

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

Ref< ISerializable > PipelineBuilderDistributed::getBuildProduct(const ISerializable* sourceAsset)
{
	T_FATAL_ERROR;
	return 0;
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

bool PipelineBuilderDistributed::performBuild(const IPipelineDependencySet* dependencySet, const PipelineDependency* dependency)
{
	PipelineDependencyHash currentDependencyHash;
	bool result = true;

	// Create hash entry.
	calculateGlobalHash(
		dependencySet,
		dependency,
		currentDependencyHash.pipelineHash,
		currentDependencyHash.sourceAssetHash,
		currentDependencyHash.sourceDataHash,
		currentDependencyHash.filesHash
	);

	// Skip no-build asset; just update hash.
	if ((dependency->flags & PdfBuild) == 0)
	{
		m_pipelineDb->setDependency(dependency->outputGuid, currentDependencyHash);
		return true;
	}

	Ref< PipelineAgent > agent = m_agentsManager->getIdleAgent();
	if (!agent)
		return false;

	log::info << L"Building asset \"" << dependency->outputPath << L"..." << Endl;

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*dependency->pipelineType);
	T_ASSERT (pipeline);

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
			currentDependencyHash,
			agentIndex
		),
		makeFunctor(
			this,
			&PipelineBuilderDistributed::agentBuildFailed,
			dependency,
			agentIndex
		)
	);

	return result;
}

void PipelineBuilderDistributed::agentBuildSucceeded(const PipelineDependency* dependency, PipelineDependencyHash hash, int32_t agentIndex)
{
	m_pipelineDb->setDependency(dependency->outputGuid, hash);

	if (m_listener)
		m_listener->endBuild(
			agentIndex,
			m_progress,
			m_progressEnd,
			dependency,
			BrSucceeded
		);

	Atomic::increment(m_progress);
}

void PipelineBuilderDistributed::agentBuildFailed(const PipelineDependency* dependency, int32_t agentIndex)
{
	if (m_listener)
		m_listener->endBuild(
			agentIndex,
			m_progress,
			m_progressEnd,
			dependency,
			BrFailed
		);

	Atomic::increment(m_progress);
}

	}
}
