#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/Save.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/PipelineDependency.h"
#include "Editor/Pipeline/PipelineDependencyCache.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDependsParallel", PipelineDependsParallel, IPipelineDepends)

PipelineDependsParallel::PipelineDependsParallel(
	PipelineFactory* pipelineFactory,
	PipelineDependencyCache* dependencyCache,
	db::Database* sourceDatabase
)
:	m_pipelineFactory(pipelineFactory)
,	m_dependencyCache(dependencyCache)
,	m_sourceDatabase(sourceDatabase)
,	m_cacheReuseCount(0)
{
	m_jobQueue = new JobQueue();
	m_jobQueue->create(4);
}

PipelineDependsParallel::~PipelineDependsParallel()
{
	safeDestroy(m_jobQueue);
	log::debug << L"PipelineDependsParallel; reused " << m_cacheReuseCount << L" cache entries" << Endl;
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
		Ref< File > file = FileSystem::getInstance().get(filePath);
		if (file)
		{
			PipelineDependency::ExternalFile externalFile;
			externalFile.filePath = filePath;
			externalFile.lastWriteTime = file->getLastWriteTime();
			parentDependency->files.push_back(externalFile);
		}
		else
			log::error << L"Unable to add dependency to \"" << filePath.getPathName() << L"\"; no such file" << Endl;
	}
}

void PipelineDependsParallel::addDependency(
	const TypeInfo& sourceAssetType
)
{
	Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());
	if (parentDependency)
	{
		// Find pipeline which consume asset type.
		const TypeInfo* pipelineType;
		uint32_t pipelineHash;

		if (!m_pipelineFactory->findPipelineType(sourceAssetType, pipelineType, pipelineHash))
		{
			log::error << L"Unable to add dependency to source asset (" << sourceAssetType.getName() << L"); no pipeline found" << Endl;
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
	Ref< ISerializable > object;

	{
		T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_readCacheLock);
		std::map< Guid, Ref< ISerializable > >::iterator i = m_readCache.find(instanceGuid);
		if (i != m_readCache.end())
			object = i->second;
	}

	if (!object)
	{
		object = m_sourceDatabase->getObjectReadOnly(instanceGuid);
		{
			T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_readCacheLock);
			m_readCache[instanceGuid] = object;
		}
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
	const TypeInfo* pipelineType;
	uint32_t pipelineHash;
	uint32_t dependencyIndex;

	// Find appropriate pipeline.
	if (!m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
	{
		log::error << L"Unable to add dependency to \"" << outputPath << L"\"; no pipeline found" << Endl;
		return;
	}

	// Get cached dependency from last build.
	Ref< PipelineDependency > cachedDependency = m_dependencyCache ? m_dependencyCache->get(outputGuid) : 0;
	if (cachedDependency)
	{
		if (
			cachedDependency->pipelineType != pipelineType ||
			cachedDependency->pipelineHash != pipelineHash
		)
			cachedDependency = 0;
	}

	// Setup dependency.
	currentDependency->pipelineType = pipelineType;
	currentDependency->pipelineHash = pipelineHash;
	currentDependency->sourceInstanceGuid = sourceInstance ? sourceInstance->getGuid() : Guid();
	currentDependency->sourceAsset = sourceAsset;
	currentDependency->outputPath = outputPath;
	currentDependency->outputGuid = outputGuid;
	currentDependency->reason = PbrNone;

	if (sourceInstance)
		sourceInstance->getLastModifyDate(currentDependency->sourceInstanceLastModifyDate);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_dependenciesLock);
		dependencyIndex = uint32_t(m_dependencies.size());
		m_dependencies.push_back(currentDependency);
	}

	// Check if cached dependency is still valid.
	if (cachedDependency)
	{
		if (
			currentDependency->sourceInstanceGuid == cachedDependency->sourceInstanceGuid &&
			currentDependency->sourceInstanceLastModifyDate == cachedDependency->sourceInstanceLastModifyDate &&
			currentDependency->outputPath == cachedDependency->outputPath &&
			currentDependency->outputGuid == cachedDependency->outputGuid
		)
		{
			// Check time stamps of each external file.
			for (std::vector< PipelineDependency::ExternalFile >::const_iterator i = cachedDependency->files.begin(); i != cachedDependency->files.end(); ++i)
			{
				Ref< File > file = FileSystem::getInstance().get(i->filePath);
				if (!file || file->getLastWriteTime() != i->lastWriteTime)
				{
					cachedDependency = 0;
					break;
				}
			}

			// Reuse hashes from cached dependency.
			if (cachedDependency)
			{
				currentDependency->sourceAssetHash = cachedDependency->sourceAssetHash;
				currentDependency->sourceDataHash = cachedDependency->sourceDataHash;
				currentDependency->filesHash = cachedDependency->filesHash;
				currentDependency->files = cachedDependency->files;
				Atomic::increment(m_cacheReuseCount);
			}
		}
		else
			cachedDependency = 0;
	}

	bool result = true;

	// Scan child dependencies.
	{
		Ref< PipelineDependency > previousDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());
		m_currentDependency.set(currentDependency);

		if (cachedDependency)
		{
			for (RefArray< PipelineDependency >::const_iterator i = cachedDependency->children.begin(); i != cachedDependency->children.end(); ++i)
				addCachedDependency(*i);
		}
		else
			result = false;

		if (!result)
		{
			Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*currentDependency->pipelineType);
			T_ASSERT (pipeline);

			result = pipeline->buildDependencies(
				this,
				sourceInstance,
				sourceAsset,
				currentDependency->outputPath,
				currentDependency->outputGuid
			);
		}

		m_currentDependency.set(previousDependency);
	}

	if (result)
	{
		// Calculate new hashes if no cached dependency was used.
		if (!cachedDependency)
			updateDependencyHashes(currentDependency, sourceInstance);

		// Add to dependency cache.
		if (m_dependencyCache)
			m_dependencyCache->put(outputGuid, currentDependency);
	}
	else
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

void PipelineDependsParallel::addCachedDependency(PipelineDependency* dependency)
{
	if (dependency->sourceInstanceGuid.isNotNull())
	{
		addDependency(
			dependency->sourceInstanceGuid,
			dependency->flags
		);
	}
	else
	{
		addDependency(
			dependency->sourceAsset,
			dependency->outputPath,
			dependency->outputGuid,
			dependency->flags
		);
	}
}

void PipelineDependsParallel::updateDependencyHashes(
	PipelineDependency* dependency,
	const db::Instance* sourceInstance
) const
{
	// Calculate source of source asset.
	dependency->sourceAssetHash = DeepHash(dependency->sourceAsset).get();

	// Calculate hash of instance data.
	dependency->sourceDataHash = 0;
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

	// Calculate external file hashes.
	dependency->filesHash = 0;
	for (std::vector< PipelineDependency::ExternalFile >::iterator i = dependency->files.begin(); i != dependency->files.end(); ++i)
	{
		Ref< IStream > fileStream = FileSystem::getInstance().open(i->filePath, File::FmRead);
		if (fileStream)
		{
			uint8_t buffer[4096];
			Adler32 a32;
			int32_t r;

			a32.begin();
			while ((r = fileStream->read(buffer, sizeof(buffer))) > 0)
				a32.feed(buffer, r);
			a32.end();

			dependency->filesHash += a32.get();
			fileStream->close();
		}
	}
}

void PipelineDependsParallel::jobAddDependency(Ref< PipelineDependency > parentDependency, Ref< const ISerializable > sourceAsset)
{
	const TypeInfo* pipelineType;
	uint32_t pipelineHash;

	if (m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
	{
		Ref< PipelineDependency > previousDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());
		m_currentDependency.set(parentDependency);

		Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*pipelineType);
		T_ASSERT (pipeline);

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
