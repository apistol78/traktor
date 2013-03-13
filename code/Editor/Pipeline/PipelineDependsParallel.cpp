#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/Save.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/PipelineDependency.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDependsParallel", PipelineDependsParallel, IPipelineDepends)

PipelineDependsParallel::PipelineDependsParallel(
	PipelineFactory* pipelineFactory,
	db::Database* sourceDatabase
)
:	m_pipelineFactory(pipelineFactory)
,	m_sourceDatabase(sourceDatabase)
{
	m_jobQueue = new JobQueue();
	m_jobQueue->create(4);
}

PipelineDependsParallel::~PipelineDependsParallel()
{
	safeDestroy(m_jobQueue);
}

void PipelineDependsParallel::addDependency(const ISerializable* sourceAsset)
{
	if (!sourceAsset)
		return;

	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());
	T_ASSERT (parentDependency);

	m_jobQueue->add(makeFunctor(this, &PipelineDependsParallel::jobAddDependency, parentDependency, Ref< const ISerializable >(sourceAsset)));
}

void PipelineDependsParallel::addDependency(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, uint32_t flags)
{
	if (!sourceAsset)
		return;

	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());

	m_jobQueue->add(makeFunctor(this, &PipelineDependsParallel::jobAddDependency, parentDependency, Ref< const ISerializable >(sourceAsset), outputPath, outputGuid, flags));
}

void PipelineDependsParallel::addDependency(db::Instance* sourceAssetInstance, uint32_t flags)
{
	if (!sourceAssetInstance)
		return;

	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());

	m_jobQueue->add(makeFunctor(this, &PipelineDependsParallel::jobAddDependency, parentDependency, Ref< db::Instance >(sourceAssetInstance), flags));
}

void PipelineDependsParallel::addDependency(const Guid& sourceAssetGuid, uint32_t flags)
{
	if (sourceAssetGuid.isNull() || !sourceAssetGuid.isValid())
		return;

	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());

	m_jobQueue->add(makeFunctor(this, &PipelineDependsParallel::jobAddDependency, parentDependency, sourceAssetGuid, flags));
}

void PipelineDependsParallel::addDependency(
	const Path& basePath,
	const std::wstring& fileName
)
{
	Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());
	if (parentDependency)
	{
		Path filePath = FileSystem::getInstance().getAbsolutePath(basePath, fileName);
		parentDependency->files.insert(filePath);
	}
}

void PipelineDependsParallel::addDependency(
	const TypeInfo& sourceAssetType
)
{
	Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());
	if (parentDependency)
	{
		Ref< IPipeline > pipeline;
		uint32_t pipelineHash;

		// Find pipeline which consume asset type.
		if (!m_pipelineFactory->findPipeline(sourceAssetType, pipeline, pipelineHash))
		{
			log::error << L"Unable to add dependency to \"" << sourceAssetType.getName() << L"\"; no pipeline found" << Endl;
			return;
		}

		// Merge hash of dependent pipeline with parent pipeline hash.
		parentDependency->pipelineHash += pipelineHash;
	}
}

bool PipelineDependsParallel::waitUntilFinished()
{
	return m_jobQueue->wait();
}

void PipelineDependsParallel::getDependencies(RefArray< PipelineDependency >& outDependencies) const
{
	outDependencies = m_dependencies;
}

Ref< db::Database > PipelineDependsParallel::getSourceDatabase() const
{
	return m_sourceDatabase;
}

Ref< const ISerializable > PipelineDependsParallel::getObjectReadOnly(const Guid& instanceGuid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_readCacheLock);
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

Ref< PipelineDependency > PipelineDependsParallel::findOrCreateDependency(
	const Guid& guid,
	PipelineDependency* parentDependency,
	uint32_t flags,
	bool& outExists
)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_dependencyMapLock);

	std::map< Guid, Ref< PipelineDependency > >::const_iterator i = m_dependencyMap.find(guid);
	if (i != m_dependencyMap.end())
	{
		i->second->flags |= flags;
		if (parentDependency)
			parentDependency->children.push_back(i->second);
		outExists = true;
		return i->second;
	}

	Ref< PipelineDependency > dependency = new PipelineDependency();
	dependency->flags = flags;
	if (parentDependency)
		parentDependency->children.push_back(dependency);
	m_dependencyMap[guid] = dependency;
	outExists = false;

	return dependency;
}

void PipelineDependsParallel::addUniqueDependency(
	PipelineDependency* parentDependency,
	PipelineDependency* currentDependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
)
{
	Ref< IPipeline > pipeline;
	uint32_t pipelineHash;
	uint32_t dependencyIndex;

	// Find appropriate pipeline.
	if (!m_pipelineFactory->findPipeline(type_of(sourceAsset), pipeline, pipelineHash))
	{
		log::error << L"Unable to add dependency to \"" << outputPath << L"\"; no pipeline found" << Endl;
		return;
	}

	// Setup dependency.
	currentDependency->pipeline = pipeline;
	currentDependency->pipelineHash = pipelineHash;
	currentDependency->sourceAsset = sourceAsset;
	currentDependency->outputPath = outputPath;
	currentDependency->outputGuid = outputGuid;
	currentDependency->reason = PbrNone;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_dependenciesLock);
		dependencyIndex = uint32_t(m_dependencies.size());
		m_dependencies.push_back(currentDependency);
	}

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

				currentDependency->sourceDataHash += a32.get();
			}
		}
	}

	// Scan child dependencies.
	{
		Ref< PipelineDependency > previousDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());
		m_currentDependency.set(currentDependency);

		result = pipeline->buildDependencies(
			this,
			sourceInstance,
			sourceAsset,
			currentDependency->outputPath,
			currentDependency->outputGuid
		);

		m_currentDependency.set(previousDependency);
	}

	if (!result)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_dependenciesLock);

		// Pipeline build dependencies failed; remove dependency from array.
		T_ASSERT (dependencyIndex < m_dependencies.size());
		m_dependencies.erase(m_dependencies.begin() + dependencyIndex);

		// Remove from parent as well.
		if (parentDependency)
		{
			RefArray< PipelineDependency >::iterator i = std::find(parentDependency->children.begin(), parentDependency->children.end(), currentDependency);
			parentDependency->children.erase(i);
		}
	}
}

void PipelineDependsParallel::jobAddDependency(Ref< PipelineDependency > parentDependency, Ref< const ISerializable > sourceAsset)
{
	Ref< IPipeline > pipeline;
	uint32_t pipelineHash;

	if (m_pipelineFactory->findPipeline(type_of(sourceAsset), pipeline, pipelineHash))
	{
		Ref< PipelineDependency > previousDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());
		m_currentDependency.set(parentDependency);

		pipeline->buildDependencies(this, 0, sourceAsset, L"", Guid());

		// Merge hash of dependent pipeline with parent's pipeline hash.
		parentDependency->pipelineHash += pipelineHash;

		m_currentDependency.set(previousDependency);
	}
	else
		log::error << L"Unable to add dependency to source asset (" << type_name(sourceAsset) << L"); no pipeline found" << Endl;
}

void PipelineDependsParallel::jobAddDependency(Ref< PipelineDependency > parentDependency, Ref< const ISerializable > sourceAsset, std::wstring outputPath, Guid outputGuid, uint32_t flags)
{
	Ref< PipelineDependency > currentDependency;
	bool exists;

	// Don't add dependency multiple times.
	currentDependency = findOrCreateDependency(outputGuid, parentDependency, flags, exists);
	if (exists)
		return;

	addUniqueDependency(
		parentDependency,
		currentDependency,
		0,
		sourceAsset,
		outputPath,
		outputGuid
	);
}

void PipelineDependsParallel::jobAddDependency(Ref< PipelineDependency > parentDependency, Ref< db::Instance > sourceAssetInstance, uint32_t flags)
{
	Ref< PipelineDependency > currentDependency;
	bool exists;

	// Don't add dependency multiple times.
	currentDependency = findOrCreateDependency(sourceAssetInstance->getGuid(), parentDependency, flags, exists);
	if (exists)
		return;

	// Read source asset instance.
	Ref< ISerializable > sourceAsset = sourceAssetInstance->getObject();
	if (!sourceAsset)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetInstance->getName() << L"\"; failed to read instance object" << Endl;
		return;
	}

	addUniqueDependency(
		parentDependency,
		currentDependency,
		sourceAssetInstance,
		sourceAsset,
		sourceAssetInstance->getPath(),
		sourceAssetInstance->getGuid()
	);
}

void PipelineDependsParallel::jobAddDependency(Ref< PipelineDependency > parentDependency, Guid sourceAssetGuid, uint32_t flags)
{
	Ref< PipelineDependency > currentDependency;
	bool exists;

	// Don't add dependency multiple times.
	currentDependency = findOrCreateDependency(sourceAssetGuid, parentDependency, flags, exists);
	if (exists)
		return;

	// Get source asset instance from database.
	Ref< db::Instance > sourceAssetInstance = m_sourceDatabase->getInstance(sourceAssetGuid);
	if (!sourceAssetInstance)
	{
		if (parentDependency)
			log::error << L"Unable to add dependency to \"" << sourceAssetGuid.format() << L"\"; no such instance (parent \"" << parentDependency->outputPath << L"\")" << Endl;
		else
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
		parentDependency,
		currentDependency,
		sourceAssetInstance,
		sourceAsset,
		sourceAssetInstance->getPath(),
		sourceAssetInstance->getGuid()
	);
}

	}
}
