#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Save.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/PipelineDependency.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDependsIncremental", PipelineDependsIncremental, IPipelineDepends)

PipelineDependsIncremental::PipelineDependsIncremental(
	PipelineFactory* pipelineFactory,
	db::Database* sourceDatabase,
	uint32_t recursionDepth
)
:	m_pipelineFactory(pipelineFactory)
,	m_sourceDatabase(sourceDatabase)
,	m_maxRecursionDepth(recursionDepth)
,	m_currentRecursionDepth(0)
{
}

void PipelineDependsIncremental::addDependency(const ISerializable* sourceAsset)
{
	T_ASSERT(m_currentDependency);

	if (!sourceAsset)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	Ref< IPipeline > pipeline;
	uint32_t pipelineHash;

	if (m_pipelineFactory->findPipeline(type_of(sourceAsset), pipeline, pipelineHash))
	{
		pipeline->buildDependencies(this, 0, sourceAsset, L"", Guid());
		if (m_currentDependency)
			m_currentDependency->pipelineHash += pipelineHash;
	}
	else
		log::error << L"Unable to add dependency to source asset (" << type_name(sourceAsset) << L"); no pipeline found" << Endl;
}

void PipelineDependsIncremental::addDependency(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, uint32_t flags)
{
	if (!sourceAsset)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Don't add dependency multiple times.
	if (PipelineDependency* dependency = findDependency(outputGuid))
	{
		dependency->flags |= flags;

		// Still need to add to current dependency so one asset can be dependent upon from several others.
		if (m_currentDependency)
			m_currentDependency->children.push_back(dependency);
		return;
	}

	addUniqueDependency(
		0,
		sourceAsset,
		outputPath,
		outputGuid,
		flags
	);
}

void PipelineDependsIncremental::addDependency(db::Instance* sourceAssetInstance, uint32_t flags)
{
	if (!sourceAssetInstance)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Don't add dependency multiple times.
	if (PipelineDependency* dependency = findDependency(sourceAssetInstance->getGuid()))
	{
		dependency->flags |= flags;

		// Still need to add to current dependency so one asset can be dependent upon from several others.
		if (m_currentDependency)
			m_currentDependency->children.push_back(dependency);
		return;
	}

	// Checkout source asset instance.
	Ref< ISerializable > sourceAsset = sourceAssetInstance->getObject();
	if (!sourceAsset)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetInstance->getName() << L"\"; failed to checkout instance" << Endl;
		return;
	}

	addUniqueDependency(
		sourceAssetInstance,
		sourceAsset,
		sourceAssetInstance->getPath(),
		sourceAssetInstance->getGuid(),
		flags
	);
}

void PipelineDependsIncremental::addDependency(const Guid& sourceAssetGuid, uint32_t flags)
{
	if (sourceAssetGuid.isNull() || !sourceAssetGuid.isValid())
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Don't add dependency multiple times.
	if (PipelineDependency* dependency = findDependency(sourceAssetGuid))
	{
		dependency->flags |= flags;

		// Still need to add to current dependency so one asset can be dependent upon from several others.
		if (m_currentDependency)
			m_currentDependency->children.push_back(dependency);
		return;
	}

	// Get source asset instance from database.
	Ref< db::Instance > sourceAssetInstance = m_sourceDatabase->getInstance(sourceAssetGuid);
	if (!sourceAssetInstance)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetGuid.format() << L"\"; no such instance" << Endl;
		return;
	}

	// Checkout source asset instance.
	Ref< ISerializable > sourceAsset = sourceAssetInstance->getObject();
	if (!sourceAsset)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetInstance->getName() << L"\"; failed to checkout instance" << Endl;
		return;
	}

	addUniqueDependency(
		sourceAssetInstance,
		sourceAsset,
		sourceAssetInstance->getPath(),
		sourceAssetInstance->getGuid(),
		flags
	);
}

void PipelineDependsIncremental::addDependency(
	const Path& basePath,
	const std::wstring& fileName
)
{
	Path filePath = FileSystem::getInstance().getAbsolutePath(basePath, fileName);
	if (m_currentDependency)
		m_currentDependency->files.insert(filePath);
}

void PipelineDependsIncremental::addDependency(
	const TypeInfo& sourceAssetType
)
{
	Ref< IPipeline > pipeline;
	uint32_t pipelineHash;

	// Find pipeline which consume asset type.
	if (!m_pipelineFactory->findPipeline(sourceAssetType, pipeline, pipelineHash))
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetType.getName() << L"\"; no pipeline found" << Endl;
		return;
	}

	// Merge hash of dependent pipeline with current pipeline hash.
	if (m_currentDependency)
		m_currentDependency->pipelineHash += pipelineHash;
}

bool PipelineDependsIncremental::waitUntilFinished()
{
	return true;
}

void PipelineDependsIncremental::getDependencies(RefArray< PipelineDependency >& outDependencies) const
{
	outDependencies = m_dependencies;
}

Ref< db::Database > PipelineDependsIncremental::getSourceDatabase() const
{
	return m_sourceDatabase;
}

Ref< const ISerializable > PipelineDependsIncremental::getObjectReadOnly(const Guid& instanceGuid)
{
	Ref< ISerializable > object;

	std::map< Guid, Ref< ISerializable > >::iterator i = m_readCache.find(instanceGuid);
	if (i != m_readCache.end())
		object = i->second;
	else
	{
		object = m_sourceDatabase->getObjectReadOnly(instanceGuid);
		m_readCache[instanceGuid] = object;
	}

	return object;
}

Ref< PipelineDependency > PipelineDependsIncremental::findDependency(const Guid& guid) const
{
	std::map< Guid, PipelineDependency* >::const_iterator i = m_dependencyMap.find(guid);
	return i != m_dependencyMap.end() ? i->second : 0;
}

void PipelineDependsIncremental::addUniqueDependency(
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t flags
)
{
	Ref< IPipeline > pipeline;
	uint32_t pipelineHash;

	// Find appropriate pipeline.
	if (!m_pipelineFactory->findPipeline(type_of(sourceAsset), pipeline, pipelineHash))
	{
		log::error << L"Unable to add dependency to \"" << outputPath << L"\"; no pipeline found" << Endl;
		return;
	}

	// Register dependency, add to "parent" dependency as well.
	Ref< PipelineDependency > dependency = new PipelineDependency();
	dependency->pipeline = pipeline;
	dependency->pipelineHash = pipelineHash;
	dependency->sourceAsset = sourceAsset;
	dependency->outputPath = outputPath;
	dependency->outputGuid = outputGuid;
	dependency->flags = flags;
	dependency->reason = PbrNone;

	if (m_currentDependency)
		m_currentDependency->children.push_back(dependency);

	uint32_t dependencyIndex = uint32_t(m_dependencies.size());
	m_dependencies.push_back(dependency);
	m_dependencyMap.insert(std::make_pair(outputGuid, dependency));

	bool result = true;

	// Calculate hash of instance data.
	if (sourceInstance)
	{
		std::vector< std::wstring > dataNames;
		sourceInstance->getDataNames(dataNames);

		for (std::vector< std::wstring >::const_iterator i = dataNames.begin(); i != dataNames.end(); ++i)
		{
			Ref< IStream > dataStream = sourceInstance->readData(*i);
			if (dataStream)
			{
				uint8_t buffer[4096];
				Adler32 a32;
				int32_t r;

				a32.begin();
				while ((r = dataStream->read(buffer, sizeof(buffer))) > 0)
					a32.feed(buffer, r);
				a32.end();

				dependency->sourceDataHash += a32.get();
			}
		}
	}

	// Recurse scan child dependencies.
	if (m_currentRecursionDepth < m_maxRecursionDepth)
	{
		T_ANONYMOUS_VAR(Save< uint32_t >)(m_currentRecursionDepth, m_currentRecursionDepth + 1);
		T_ANONYMOUS_VAR(Save< Ref< PipelineDependency > >)(m_currentDependency, dependency);

		result = pipeline->buildDependencies(
			this,
			sourceInstance,
			sourceAsset,
			outputPath,
			outputGuid
		);
	}

	if (!result)
	{
		// Pipeline build dependencies failed; remove dependency from array.
		T_ASSERT (dependencyIndex < m_dependencies.size());
		m_dependencies.erase(m_dependencies.begin() + dependencyIndex);

		// Remove from current as well.
		if (m_currentDependency)
		{
			RefArray< PipelineDependency >::iterator i = std::find(m_currentDependency->children.begin(), m_currentDependency->children.end(), dependency);
			m_currentDependency->children.erase(i);
		}
	}
}

	}
}
