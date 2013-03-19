#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Save.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/PipelineDependency.h"
#include "Editor/Pipeline/PipelineDependencyCache.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDependsIncremental", PipelineDependsIncremental, IPipelineDepends)

PipelineDependsIncremental::PipelineDependsIncremental(
	PipelineFactory* pipelineFactory,
	PipelineDependencyCache* dependencyCache,
	db::Database* sourceDatabase,
	uint32_t recursionDepth
)
:	m_pipelineFactory(pipelineFactory)
,	m_dependencyCache(dependencyCache)
,	m_sourceDatabase(sourceDatabase)
,	m_maxRecursionDepth(recursionDepth)
,	m_currentRecursionDepth(0)
,	m_cacheReuseCount(0)
{
}

PipelineDependsIncremental::~PipelineDependsIncremental()
{
	log::debug << L"PipelineDependsIncremental; reused " << m_cacheReuseCount << L" cache entries" << Endl;
}

void PipelineDependsIncremental::addDependency(const ISerializable* sourceAsset)
{
	T_ASSERT(m_currentDependency);

	if (!sourceAsset)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	const TypeInfo* pipelineType;
	uint32_t pipelineHash;

	if (!m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
	{
		log::error << L"Unable to add dependency to source asset (" << type_name(sourceAsset) << L"); no pipeline found" << Endl;
		return;
	}

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*pipelineType);
	T_ASSERT (pipeline);

	pipeline->buildDependencies(this, 0, sourceAsset, L"", Guid());

	if (m_currentDependency)
		m_currentDependency->pipelineHash += pipelineHash;
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
	{
		Ref< File > file = FileSystem::getInstance().get(filePath);
		if (file)
		{
			PipelineDependency::ExternalFile externalFile;
			externalFile.filePath = filePath;
			externalFile.lastWriteTime = file->getLastWriteTime();
			m_currentDependency->files.push_back(externalFile);
		}
		else
			log::error << L"Unable to add dependency to \"" << filePath.getPathName() << L"\"; no such file" << Endl;
	}
}

void PipelineDependsIncremental::addDependency(
	const TypeInfo& sourceAssetType
)
{
	// Find pipeline which consume asset type.
	const TypeInfo* pipelineType;
	uint32_t pipelineHash;

	if (!m_pipelineFactory->findPipelineType(sourceAssetType, pipelineType, pipelineHash))
	{
		log::error << L"Unable to add dependency to source asset (" << sourceAssetType.getName() << L"); no pipeline found" << Endl;
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

PipelineDependency* PipelineDependsIncremental::findDependency(const Guid& guid) const
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
	const TypeInfo* pipelineType;
	uint32_t pipelineHash;

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

	// Create dependency, add to "parent" dependency as well.
	Ref< PipelineDependency > dependency = new PipelineDependency();
	dependency->pipelineType = pipelineType;
	dependency->pipelineHash = pipelineHash;
	dependency->sourceInstanceGuid = sourceInstance ? sourceInstance->getGuid() : Guid();
	dependency->sourceAsset = sourceAsset;
	dependency->outputPath = outputPath;
	dependency->outputGuid = outputGuid;
	dependency->flags = flags;
	dependency->reason = PbrNone;

	if (sourceInstance)
		sourceInstance->getLastModifyDate(dependency->sourceInstanceLastModifyDate);

	if (m_currentDependency)
		m_currentDependency->children.push_back(dependency);

	// Check if cached dependency is still valid.
	if (cachedDependency)
	{
		if (
			dependency->sourceInstanceGuid == cachedDependency->sourceInstanceGuid &&
			dependency->sourceInstanceLastModifyDate == cachedDependency->sourceInstanceLastModifyDate &&
			dependency->outputPath == cachedDependency->outputPath &&
			dependency->outputGuid == cachedDependency->outputGuid
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
				dependency->sourceAssetHash = cachedDependency->sourceAssetHash;
				dependency->sourceDataHash = cachedDependency->sourceDataHash;
				dependency->filesHash = cachedDependency->filesHash;
				dependency->files = cachedDependency->files;
				++m_cacheReuseCount;
			}
		}
		else
			cachedDependency = 0;
	}

	uint32_t dependencyIndex = uint32_t(m_dependencies.size());
	m_dependencies.push_back(dependency);
	m_dependencyMap.insert(std::make_pair(outputGuid, dependency));

	bool result = true;

	// Recurse scan child dependencies.
	if (m_currentRecursionDepth < m_maxRecursionDepth)
	{
		T_ANONYMOUS_VAR(Save< uint32_t >)(m_currentRecursionDepth, m_currentRecursionDepth + 1);
		T_ANONYMOUS_VAR(Save< Ref< PipelineDependency > >)(m_currentDependency, dependency);

		if (cachedDependency)
		{
			for (RefArray< PipelineDependency >::const_iterator i = cachedDependency->children.begin(); i != cachedDependency->children.end(); ++i)
				addCachedDependency(*i);
		}
		else
			result = false;

		if (!result)
		{
			Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*dependency->pipelineType);
			T_ASSERT (pipeline);

			result = pipeline->buildDependencies(
				this,
				sourceInstance,
				sourceAsset,
				outputPath,
				outputGuid
			);
		}
	}

	if (result)
	{
		// Calculate new hashes if no cached dependency was used.
		if (!cachedDependency)
			updateDependencyHashes(dependency, sourceInstance);

		// Add to dependency cache.
		if (m_dependencyCache)
			m_dependencyCache->put(outputGuid, dependency);
	}
	else
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

void PipelineDependsIncremental::addCachedDependency(PipelineDependency* dependency)
{
	if (dependency->sourceInstanceGuid.isNotNull())
	{
		if (PipelineDependency* existingDependency = findDependency(dependency->outputGuid))
		{
			existingDependency->flags |= dependency->flags;
			if (m_currentDependency)
				m_currentDependency->children.push_back(existingDependency);
			return;
		}

		Ref< db::Instance > sourceAssetInstance = m_sourceDatabase->getInstance(dependency->sourceInstanceGuid);
		if (!sourceAssetInstance)
		{
			log::error << L"Unable to add dependency to \"" << dependency->sourceInstanceGuid.format() << L"\"; no such instance" << Endl;
			return;
		}

		addUniqueDependency(
			sourceAssetInstance,
			dependency->sourceAsset,
			sourceAssetInstance->getPath(),
			sourceAssetInstance->getGuid(),
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

void PipelineDependsIncremental::updateDependencyHashes(
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

	}
}
