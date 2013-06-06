#if defined(_WIN32)
#	include <cfloat>
#endif
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/Save.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipeline.h"
#include "Editor/IPipelineDb.h"
#include "Editor/IPipelineDependencySet.h"
#include "Editor/PipelineDependency.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDependsParallel", PipelineDependsParallel, IPipelineDepends)

PipelineDependsParallel::PipelineDependsParallel(
	PipelineFactory* pipelineFactory,
	db::Database* sourceDatabase,
	IPipelineDependencySet* dependencySet,
	IPipelineDb* pipelineDb
)
:	m_pipelineFactory(pipelineFactory)
,	m_sourceDatabase(sourceDatabase)
,	m_dependencySet(dependencySet)
,	m_pipelineDb(pipelineDb)
{
	m_jobQueue = new JobQueue();
	m_jobQueue->create(
		OS::getInstance().getCPUCoreCount(),
		Thread::Above
	);
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_dependencySetLock);

	uint32_t dependencyIndex = m_dependencySet->get(guid);
	if (dependencyIndex != IPipelineDependencySet::DiInvalid)
	{
		PipelineDependency* dependency = m_dependencySet->get(dependencyIndex);
		T_ASSERT (dependency);

		dependency->flags |= flags;
		if (parentDependency)
			parentDependency->children.push_back(dependencyIndex);

		outExists = true;
		return dependency;
	}

	Ref< PipelineDependency > dependency = new PipelineDependency();
	dependency->flags = flags;
	
	dependencyIndex = m_dependencySet->add(guid, dependency);
	if (parentDependency)
		parentDependency->children.push_back(dependencyIndex);

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

	// Find appropriate pipeline.
	if (!m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
	{
		log::error << L"Unable to add dependency to \"" << outputPath << L"\"; no pipeline found" << Endl;
		return;
	}

	// Setup dependency.
	currentDependency->pipelineType = pipelineType;
	currentDependency->pipelineHash = pipelineHash;
	currentDependency->sourceInstanceGuid = sourceInstance ? sourceInstance->getGuid() : Guid();
	currentDependency->sourceAsset = sourceAsset;
	currentDependency->outputPath = outputPath;
	currentDependency->outputGuid = outputGuid;

	bool result = true;

	// Scan child dependencies.
	{
		Ref< PipelineDependency > previousDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());
		m_currentDependency.set(currentDependency);

		Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*currentDependency->pipelineType);
		T_ASSERT (pipeline);

		result = pipeline->buildDependencies(
			this,
			sourceInstance,
			sourceAsset,
			currentDependency->outputPath,
			currentDependency->outputGuid
		);

		m_currentDependency.set(previousDependency);
	}

	if (result)
		updateDependencyHashes(currentDependency, sourceInstance);
	else
		currentDependency->flags |= PdfFailed;
}

void PipelineDependsParallel::updateDependencyHashes(
	PipelineDependency* dependency,
	const db::Instance* sourceInstance
) const
{
	// Ensure FP is in known state.
#if defined(_WIN32)
	uint32_t dummy;
	_controlfp_s(&dummy, 0, 0);
	_controlfp_s(&dummy,_PC_24, _MCW_PC);
	_controlfp_s(&dummy,_RC_NEAR, _MCW_RC);
#endif

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
		if (m_pipelineDb)
		{
			Ref< File > file = FileSystem::getInstance().get(i->filePath);
			if (file)
			{
				IPipelineDb::FileHash fileHash;
				if (m_pipelineDb->getFile(i->filePath, fileHash))
				{
					if (fileHash.lastWriteTime == file->getLastWriteTime())
					{
						dependency->filesHash += fileHash.hash;
						continue;
					}
				}
			}
		}

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

			if (m_pipelineDb)
			{
				Ref< File > file = FileSystem::getInstance().get(i->filePath);
				if (file)
				{
					IPipelineDb::FileHash fileHash;
					fileHash.size = file->getSize();
					fileHash.lastWriteTime = file->getLastWriteTime();
					fileHash.hash = a32.get();
					m_pipelineDb->setFile(i->filePath, fileHash);
				}
			}
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
