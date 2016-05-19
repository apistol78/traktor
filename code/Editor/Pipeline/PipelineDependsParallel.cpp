#if defined(_WIN32)
#	include <cfloat>
#endif
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
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
#include "Editor/IPipelineInstanceCache.h"
#include "Editor/PipelineDependency.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

class ExternalFilePred
{
public:
	ExternalFilePred(const Path& path)
	:	m_path(path)
	{
	}

	bool operator () (const PipelineDependency::ExternalFile& file) const
	{
		return file.filePath == m_path;
	}

private:
	const Path& m_path;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDependsParallel", PipelineDependsParallel, IPipelineDepends)

PipelineDependsParallel::PipelineDependsParallel(
	PipelineFactory* pipelineFactory,
	db::Database* sourceDatabase,
	db::Database* outputDatabase,
	IPipelineDependencySet* dependencySet,
	IPipelineDb* pipelineDb,
	IPipelineInstanceCache* instanceCache
)
:	m_pipelineFactory(pipelineFactory)
,	m_sourceDatabase(sourceDatabase)
,	m_outputDatabase(outputDatabase)
,	m_dependencySet(dependencySet)
,	m_pipelineDb(pipelineDb)
,	m_instanceCache(instanceCache)
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

	const TypeInfo* pipelineType;
	uint32_t pipelineHash;

	if (m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
	{
		Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());

		Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*pipelineType);
		T_ASSERT (pipeline);

		pipeline->buildDependencies(this, 0, sourceAsset, L"", Guid());

		// Merge hash of dependent pipeline with parent's pipeline hash.
		if (parentDependency)
			parentDependency->pipelineHash += pipelineHash;
	}
	else
		log::error << L"Unable to add dependency to source asset (" << type_name(sourceAsset) << L"); no pipeline found." << Endl;
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
		T_FATAL_ASSERT_M(parentDependency->sourceAssetHash == 0, L"Dependency already hashed");

		Path filePath = FileSystem::getInstance().getAbsolutePath(basePath, fileName);
		if (std::find_if(parentDependency->files.begin(), parentDependency->files.end(), ExternalFilePred(filePath)) == parentDependency->files.end())
		{
			Ref< File > file = FileSystem::getInstance().get(filePath);
			if (file)
			{
				PipelineDependency::ExternalFile externalFile;
				externalFile.filePath = filePath;
				externalFile.lastWriteTime = file->getLastWriteTime();
				parentDependency->files.push_back(externalFile);
			}
			else
				log::error << L"Unable to add dependency to \"" << filePath.getPathName() << L"\"; no such file." << Endl;
		}
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
			log::error << L"Unable to add dependency to source asset (" << sourceAssetType.getName() << L"); no pipeline found." << Endl;
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

Ref< db::Database > PipelineDependsParallel::getOutputDatabase() const
{
	return m_outputDatabase;
}

Ref< const ISerializable > PipelineDependsParallel::getObjectReadOnly(const Guid& instanceGuid)
{
	if (instanceGuid.isNotNull())
		return m_instanceCache->getObjectReadOnly(instanceGuid);
	else
		return 0;
}

Ref< IStream > PipelineDependsParallel::openFile(const Path& basePath, const std::wstring& fileName)
{
	Path filePath = FileSystem::getInstance().getAbsolutePath(basePath + Path(fileName));
	Ref< IStream > fileStream = FileSystem::getInstance().open(filePath, File::FmRead);
	return fileStream ? new BufferedStream(fileStream) : 0;
}

Ref< IStream > PipelineDependsParallel::createTemporaryFile(const std::wstring& fileName)
{
	Ref< IStream > fileStream = FileSystem::getInstance().open(L"data/temp/" + fileName, File::FmWrite);
	return fileStream ? new BufferedStream(fileStream) : 0;
}

Ref< IStream > PipelineDependsParallel::openTemporaryFile(const std::wstring& fileName)
{
	Ref< IStream > fileStream = FileSystem::getInstance().open(L"data/temp/" + fileName, File::FmRead);
	return fileStream ? new BufferedStream(fileStream) : 0;
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
			parentDependency->children.insert(dependencyIndex);

		outExists = true;
		return dependency;
	}

	Ref< PipelineDependency > dependency = new PipelineDependency();
	dependency->flags = flags;
	
	dependencyIndex = m_dependencySet->add(guid, dependency);
	if (parentDependency)
		parentDependency->children.insert(dependencyIndex);

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
		currentDependency->flags |= PdfFailed;
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
	PipelineFileHash fileHash;
	DateTime lastWriteTime;
	bool haveLastWriteTime = false;
	std::wstring fauxDataPath;

	T_FATAL_ASSERT_M (dependency->sourceAssetHash == 0, L"Hash already calculated, thread issue?");

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
			if (m_pipelineDb)
			{
				haveLastWriteTime = sourceInstance->getDataLastWriteTime(*i, lastWriteTime);
				if (haveLastWriteTime)
				{
					fauxDataPath = sourceInstance->getGuid().format() + L"/" + *i;
					if (m_pipelineDb->getFile(fauxDataPath, fileHash))
					{
						if (fileHash.lastWriteTime == lastWriteTime)
						{
							dependency->sourceDataHash += fileHash.hash;
							continue;
						}
					}
				}
			}

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

				if (m_pipelineDb && haveLastWriteTime)
				{
					fileHash.size = 0;
					fileHash.lastWriteTime = lastWriteTime;
					fileHash.hash = a32.get();
					m_pipelineDb->setFile(fauxDataPath, fileHash);
				}
			}
			else
				log::error << L"Unable to open data stream \"" << *i << L"\"; hash inconsistent." << Endl;
		}
	}

	// Calculate external file hashes.
	dependency->filesHash = 0;
	for (PipelineDependency::external_files_t::iterator i = dependency->files.begin(); i != dependency->files.end(); ++i)
	{
		if (m_pipelineDb)
		{
			Ref< File > file = FileSystem::getInstance().get(i->filePath);
			if (file)
			{
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
					fileHash.size = file->getSize();
					fileHash.lastWriteTime = file->getLastWriteTime();
					fileHash.hash = a32.get();
					m_pipelineDb->setFile(i->filePath, fileHash);
				}
			}
		}
		else
			log::error << L"Unable to open file stream \"" << i->filePath.getPathName() << L"\"; hash inconsistent." << Endl;
	}
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
	Ref< ISerializable > sourceAsset = m_instanceCache->getObjectReadOnly(sourceAssetInstance->getGuid());
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
	bool exists;

	// Create dependency, another thread might have raced us to this so also ensure it hasn't been created.
	Ref< PipelineDependency > currentDependency = findOrCreateDependency(sourceAssetGuid, parentDependency, flags, exists);
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
	Ref< ISerializable > sourceAsset = m_instanceCache->getObjectReadOnly(sourceAssetGuid);
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
