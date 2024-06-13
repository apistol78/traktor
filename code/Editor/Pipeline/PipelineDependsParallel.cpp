/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if defined(_WIN32)
#	include <cfloat>
#endif
#include "Core/Io/FileSystem.h"
#include "Core/Io/IMappedFile.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/Save.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipeline.h"
#include "Editor/IPipelineDb.h"
#include "Editor/PipelineDependencySet.h"
#include "Editor/IPipelineInstanceCache.h"
#include "Editor/PipelineDependency.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDependsParallel", PipelineDependsParallel, IPipelineDepends)

PipelineDependsParallel::PipelineDependsParallel(
	PipelineFactory* pipelineFactory,
	db::Database* sourceDatabase,
	db::Database* outputDatabase,
	PipelineDependencySet* dependencySet,
	IPipelineDb* pipelineDb,
	IPipelineInstanceCache* instanceCache
)
:	m_pipelineFactory(pipelineFactory)
,	m_sourceDatabase(sourceDatabase)
,	m_outputDatabase(outputDatabase)
,	m_dependencySet(dependencySet)
,	m_pipelineDb(pipelineDb)
,	m_instanceCache(instanceCache)
,	m_result(true)
{
}

PipelineDependsParallel::~PipelineDependsParallel()
{
	waitUntilFinished();
}

void PipelineDependsParallel::addDependency(const ISerializable* sourceAsset)
{
	if (!sourceAsset || !m_result)
		return;

	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	const TypeInfo* pipelineType;
	uint32_t pipelineHash;

	if (m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
	{
		Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());

		Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*pipelineType);
		T_ASSERT(pipeline);

		pipeline->buildDependencies(this, nullptr, sourceAsset, L"", Guid());

		// Merge hash of dependent pipeline with parent's pipeline hash.
		if (parentDependency)
			parentDependency->pipelineHash += pipelineHash;
	}
	else
	{
		log::error << L"Unable to add dependency to source asset (" << type_name(sourceAsset) << L"); no pipeline found." << Endl;
		m_result = false;
	}
}

void PipelineDependsParallel::addDependency(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, uint32_t flags)
{
	if (!sourceAsset || !m_result)
		return;

	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	Ref< const ISerializable > sourceAssetRef = sourceAsset;
	Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());

	Ref< Job > job = JobManager::getInstance().add([=](){
		jobAddDependency(parentDependency, sourceAssetRef, outputPath, outputGuid, flags);
	});
	if (job)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_jobsLock);
		m_jobs.push_back(job);
	}
	else
		m_result = false;
}

void PipelineDependsParallel::addDependency(db::Instance* sourceAssetInstance, uint32_t flags)
{
	if (!sourceAssetInstance || !m_result)
		return;

	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	Ref< db::Instance > sourceAssetInstanceRef = sourceAssetInstance;
	Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());

	Ref< Job > job = JobManager::getInstance().add([=](){
		jobAddDependency(parentDependency, sourceAssetInstanceRef, flags);
	});
	if (job)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_jobsLock);
		m_jobs.push_back(job);
	}
	else
		m_result = false;
}

void PipelineDependsParallel::addDependency(const Guid& sourceAssetGuid, uint32_t flags)
{
	if (sourceAssetGuid.isNull() || !sourceAssetGuid.isValid() || !m_result)
		return;

	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	Ref< PipelineDependency > parentDependency = reinterpret_cast< PipelineDependency* >(m_currentDependency.get());

	Ref< Job > job = JobManager::getInstance().add([=](){
		jobAddDependency(parentDependency, sourceAssetGuid, flags);
	});
	if (job)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_jobsLock);
		m_jobs.push_back(job);
	}
	else
		m_result = false;
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

		const Path filePath = FileSystem::getInstance().getAbsolutePath(basePath, fileName);
		const auto it = std::find_if(parentDependency->files.begin(), parentDependency->files.end(), [&](const PipelineDependency::ExternalFile& file) {
			return file.filePath == filePath;
		});
		if (it == parentDependency->files.end())
		{
			Ref< File > file = FileSystem::getInstance().get(filePath);
			if (file && !file->isDirectory())
			{
				PipelineDependency::ExternalFile externalFile;
				externalFile.filePath = filePath;
				externalFile.lastWriteTime = file->getLastWriteTime();
				parentDependency->files.push_back(externalFile);
			}
			else
			{
				log::error << L"Unable to add dependency to \"" << filePath.getPathName() << L"\"; no such file." << Endl;
				m_result = false;
			}
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
			m_result = false;
			return;
		}

		// Merge hash of dependent pipeline with parent pipeline hash.
		parentDependency->pipelineHash += pipelineHash;
	}
}

bool PipelineDependsParallel::waitUntilFinished()
{
	Ref< Job > job;
	for (;;)
	{
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_jobsLock);
			if (m_jobs.empty())
				break;
			job = m_jobs.front();
			m_jobs.pop_front();
		}
		job->wait();
	}
	return m_result;
}

db::Database* PipelineDependsParallel::getSourceDatabase() const
{
	return m_sourceDatabase;
}

Ref< const ISerializable > PipelineDependsParallel::getObjectReadOnly(const Guid& instanceGuid)
{
	if (instanceGuid.isNotNull())
		return m_instanceCache->getObjectReadOnly(instanceGuid);
	else
		return nullptr;
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
	if (dependencyIndex != PipelineDependencySet::DiInvalid)
	{
		PipelineDependency* dependency = m_dependencySet->get(dependencyIndex);
		T_ASSERT(dependency);

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

	// Ensure arguments are valid.
	if (outputPath.empty())
	{
		log::error << L"Unable to add dependency \"" << outputGuid.format() << L"\"; no output path." << Endl;
		currentDependency->flags |= PdfFailed;
		m_result = false;
		return;
	}

	// Find appropriate pipeline.
	if (!m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
	{
		log::error << L"Unable to add dependency to \"" << outputPath << L"\"; no pipeline found." << Endl;
		currentDependency->flags |= PdfFailed;
		m_result = false;
		return;
	}

	// Setup dependency.
	currentDependency->pipelineType = pipelineType;
	currentDependency->pipelineHash = pipelineHash;
	currentDependency->sourceInstanceGuid = sourceInstance ? sourceInstance->getGuid() : Guid();
	currentDependency->sourceAsset = sourceAsset;
	currentDependency->outputPath = outputPath;
	currentDependency->outputGuid = outputGuid;

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*currentDependency->pipelineType);
	T_ASSERT(pipeline);

	bool result = true;

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

	if (result)
		updateDependencyHashes(currentDependency, pipeline, sourceInstance);
	else
	{
		currentDependency->flags |= PdfFailed;
		m_result = false;
	}
}

void PipelineDependsParallel::updateDependencyHashes(
	PipelineDependency* dependency,
	const IPipeline* pipeline,
	const db::Instance* sourceInstance
) const
{
	PipelineFileHash fileHash;
	DateTime lastWriteTime;
	bool haveLastWriteTime = false;
	std::wstring fauxDataPath;

	T_FATAL_ASSERT_M (dependency->sourceAssetHash == 0, L"Hash already calculated, thread issue?");

	// Calculate source of source asset.
	dependency->sourceAssetHash = pipeline->hashAsset(dependency->sourceAsset);
	
	// Calculate hash of instance data.
	dependency->sourceDataHash = 0;
	if (sourceInstance)
	{
		AlignedVector< std::wstring > dataNames;
		sourceInstance->getDataNames(dataNames);
		for (const auto& dataName : dataNames)
		{
			// Reuse hash from database if file hasn't been written to since last build.
			if (m_pipelineDb)
			{
				haveLastWriteTime = sourceInstance->getDataLastWriteTime(dataName, lastWriteTime);
				if (haveLastWriteTime)
				{
					fauxDataPath = sourceInstance->getGuid().format() + L"/" + dataName;
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

			// Calculate hash of instance's data.
			Ref< IStream > dataStream = sourceInstance->readData(dataName);
			if (dataStream)
			{
				uint8_t buffer[4096];
				Murmur3 a32;
				int64_t r;

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
			{
				log::error << L"Unable to open data stream \"" << dataName << L"\"; hash inconsistent." << Endl;
				m_result = false;
			}
		}
	}

	// Calculate external file hashes.
	dependency->filesHash = 0;
	for (const auto& dependencyFile : dependency->files)
	{
		// Reuse hash from database if file hasn't been written to since last build.
		if (m_pipelineDb)
		{
			Ref< File > file = FileSystem::getInstance().get(dependencyFile.filePath);
			if (file)
			{
				if (m_pipelineDb->getFile(dependencyFile.filePath, fileHash))
				{
					if (fileHash.lastWriteTime == file->getLastWriteTime())
					{
						T_FATAL_ASSERT(fileHash.size == file->getSize());
						dependency->filesHash += fileHash.hash;
						continue;
					}
				}
			}
		}

		// Calculate hash of external file.
		Ref< IMappedFile > mf = FileSystem::getInstance().map(dependencyFile.filePath);
		if (mf)
		{
			Murmur3 a32;
			a32.begin();
			a32.feed(mf->getBase(), mf->getSize());
			a32.end();

			mf = nullptr;

			dependency->filesHash += a32.get();

			if (m_pipelineDb)
			{
				Ref< File > file = FileSystem::getInstance().get(dependencyFile.filePath);
				if (file)
				{
					fileHash.size = file->getSize();
					fileHash.lastWriteTime = file->getLastWriteTime();
					fileHash.hash = a32.get();
					m_pipelineDb->setFile(dependencyFile.filePath, fileHash);
				}
			}
		}
		else
		{
			log::error << L"Unable to open file stream \"" << dependencyFile.filePath.getPathName() << L"\"; hash inconsistent." << Endl;
			m_result = false;
		}
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
		nullptr,
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
	Ref< const ISerializable > sourceAsset = m_instanceCache->getObjectReadOnly(sourceAssetInstance->getGuid());
	if (!sourceAsset)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetInstance->getName() << L"\"; failed to read instance object." << Endl;
		m_result = false;
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
			log::error << L"Unable to add dependency to \"" << sourceAssetGuid.format() << L"\"; no such instance (parent \"" << parentDependency->information() << L"\")." << Endl;
		else
			log::error << L"Unable to add dependency to \"" << sourceAssetGuid.format() << L"\"; no such instance." << Endl;

		m_result = false;
		return;
	}

	// Checkout source asset instance.
	Ref< const ISerializable > sourceAsset = m_instanceCache->getObjectReadOnly(sourceAssetGuid);
	if (!sourceAsset)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetInstance->getName() << L"\"; failed to checkout instance." << Endl;
		m_result = false;
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
