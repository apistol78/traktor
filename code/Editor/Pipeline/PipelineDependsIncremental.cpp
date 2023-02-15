/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/Save.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipeline.h"
#include "Editor/IPipelineDb.h"
#include "Editor/PipelineDependencySet.h"
#include "Editor/IPipelineInstanceCache.h"
#include "Editor/PipelineDependency.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDependsIncremental", PipelineDependsIncremental, IPipelineDepends)

PipelineDependsIncremental::PipelineDependsIncremental(
	PipelineFactory* pipelineFactory,
	db::Database* sourceDatabase,
	db::Database* outputDatabase,
	PipelineDependencySet* dependencySet,
	IPipelineDb* pipelineDb,
	IPipelineInstanceCache* instanceCache,
	const std::function< bool(const Guid&) >& excludeDependencyFilter,
	uint32_t recursionDepth
)
:	m_pipelineFactory(pipelineFactory)
,	m_sourceDatabase(sourceDatabase)
,	m_outputDatabase(outputDatabase)
,	m_dependencySet(dependencySet)
,	m_pipelineDb(pipelineDb)
,	m_instanceCache(instanceCache)
,	m_excludeDependencyFilter(excludeDependencyFilter)
,	m_maxRecursionDepth(recursionDepth)
,	m_currentRecursionDepth(0)
,	m_result(true)
{
}

void PipelineDependsIncremental::addDependency(const ISerializable* sourceAsset)
{
	if (!sourceAsset || !m_result)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	const TypeInfo* pipelineType;
	uint32_t pipelineHash;

	if (!m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
	{
		log::error << L"Unable to add dependency to source asset (" << type_name(sourceAsset) << L"); no pipeline found." << Endl;
		m_result = false;
		return;
	}

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*pipelineType);
	T_ASSERT(pipeline);

	pipeline->buildDependencies(this, nullptr, sourceAsset, L"", Guid());

	if (m_currentDependency)
		m_currentDependency->pipelineHash += pipelineHash;
}

void PipelineDependsIncremental::addDependency(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, uint32_t flags)
{
	if (!sourceAsset || !m_result)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Check if dependency should be excluded.
	if ((flags & PdfForceAdd) == 0 && m_excludeDependencyFilter != nullptr && m_excludeDependencyFilter(outputGuid) == false)
		return;

	// Don't add dependency multiple times.
	uint32_t dependencyIndex = m_dependencySet->get(outputGuid);
	if (dependencyIndex != PipelineDependencySet::DiInvalid)
	{
		PipelineDependency* dependency = m_dependencySet->get(dependencyIndex);
		T_ASSERT(dependency);

		dependency->flags |= flags;
		if (m_currentDependency)
			m_currentDependency->children.insert(dependencyIndex);

		return;
	}

	addUniqueDependency(
		nullptr,
		sourceAsset,
		outputPath,
		outputGuid,
		flags
	);
}

void PipelineDependsIncremental::addDependency(db::Instance* sourceAssetInstance, uint32_t flags)
{
	if (!sourceAssetInstance || !m_result)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Check if dependency should be excluded.
	if ((flags & PdfForceAdd) == 0 && m_excludeDependencyFilter != nullptr && m_excludeDependencyFilter(sourceAssetInstance->getGuid()) == false)
		return;

	// Don't add dependency multiple times.
	uint32_t dependencyIndex = m_dependencySet->get(sourceAssetInstance->getGuid());
	if (dependencyIndex != PipelineDependencySet::DiInvalid)
	{
		PipelineDependency* dependency = m_dependencySet->get(dependencyIndex);
		T_ASSERT(dependency);

		dependency->flags |= flags;
		if (m_currentDependency)
			m_currentDependency->children.insert(dependencyIndex);

		return;
	}

	// Checkout source asset instance.
	Ref< const ISerializable > sourceAsset = m_instanceCache->getObjectReadOnly(sourceAssetInstance->getGuid());
	if (!sourceAsset)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetInstance->getName() << L"\"; failed to checkout instance." << Endl;
		m_result = false;
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
	if (sourceAssetGuid.isNull() || !sourceAssetGuid.isValid() || !m_result)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Check if dependency should be excluded.
	if ((flags & PdfForceAdd) == 0 && m_excludeDependencyFilter != nullptr && m_excludeDependencyFilter(sourceAssetGuid) == false)
		return;

	// Don't add dependency multiple times.
	const uint32_t dependencyIndex = m_dependencySet->get(sourceAssetGuid);
	if (dependencyIndex != PipelineDependencySet::DiInvalid)
	{
		PipelineDependency* dependency = m_dependencySet->get(dependencyIndex);
		T_ASSERT(dependency);

		dependency->flags |= flags;
		if (m_currentDependency)
			m_currentDependency->children.insert(dependencyIndex);

		return;
	}

	// Get source asset instance from database.
	Ref< db::Instance > sourceAssetInstance = m_sourceDatabase->getInstance(sourceAssetGuid);
	if (!sourceAssetInstance)
	{
		if (m_currentDependency)
			log::error << L"Unable to add dependency to \"" << sourceAssetGuid.format() << L"\"; no such instance (referenced by \"" << m_currentDependency->information() << L"\")." << Endl;
		else
			log::error << L"Unable to add dependency to \"" << sourceAssetGuid.format() << L"\"; no such instance." << Endl;

		m_result = false;
		return;
	}

	// Checkout source asset instance.
	Ref< const ISerializable > sourceAsset = m_instanceCache->getObjectReadOnly(sourceAssetGuid);
	if (!sourceAsset)
	{
		if (m_currentDependency)
			log::error << L"Unable to add dependency to \"" << sourceAssetGuid.format() << L"\"; failed to checkout instance (referenced by \"" << m_currentDependency->information() << L"\")." << Endl;
		else
			log::error << L"Unable to add dependency to \"" << sourceAssetGuid.format() << L"\"; failed to checkout instance." << Endl;
		m_result = false;
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
	if (!m_result)
		return;

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
		{
			log::error << L"Unable to add dependency to \"" << filePath.getPathName() << L"\"; no such file." << Endl;
			m_result = false;
		}
	}
}

void PipelineDependsIncremental::addDependency(
	const TypeInfo& sourceAssetType
)
{
	if (!m_result)
		return;

	// Find pipeline which consume asset type.
	const TypeInfo* pipelineType;
	uint32_t pipelineHash;

	if (!m_pipelineFactory->findPipelineType(sourceAssetType, pipelineType, pipelineHash))
	{
		log::error << L"Unable to add dependency to source asset (" << sourceAssetType.getName() << L"); no pipeline found." << Endl;
		m_result = false;
		return;
	}

	// Merge hash of dependent pipeline with current pipeline hash.
	if (m_currentDependency)
		m_currentDependency->pipelineHash += pipelineHash;
}

bool PipelineDependsIncremental::waitUntilFinished()
{
	return m_result;
}

db::Database* PipelineDependsIncremental::getSourceDatabase() const
{
	return m_sourceDatabase;
}

Ref< const ISerializable > PipelineDependsIncremental::getObjectReadOnly(const Guid& instanceGuid)
{
	if (instanceGuid.isNotNull())
		return m_instanceCache->getObjectReadOnly(instanceGuid);
	else
		return nullptr;
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

	// Ensure arguments are valid.
	if (outputPath.empty())
	{
		log::error << L"Unable to add dependency \"" << outputGuid.format() << L"\"; no output path." << Endl;
		m_result = false;
		return;
	}

	// Find appropriate pipeline.
	if (!m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
	{
		log::error << L"Unable to add dependency to \"" << outputPath << L"\"; no pipeline found." << Endl;
		m_result = false;
		return;
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

	uint32_t dependencyIndex = m_dependencySet->add(outputGuid, dependency);

	if (m_currentDependency)
		m_currentDependency->children.insert(dependencyIndex);

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*dependency->pipelineType);
	T_ASSERT(pipeline);

	bool result = true;

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

	if (result)
		updateDependencyHashes(dependency, pipeline, sourceInstance);
	else
	{
		dependency->flags |= PdfFailed;
		m_result = false;
	}
}

void PipelineDependsIncremental::updateDependencyHashes(
	PipelineDependency* dependency,
	const IPipeline* pipeline,
	const db::Instance* sourceInstance
)
{
	// Calculate source of source asset.
	dependency->sourceAssetHash = pipeline->hashAsset(dependency->sourceAsset);

	// Calculate hash of instance data.
	dependency->sourceDataHash = 0;
	if (sourceInstance)
	{
		std::vector< std::wstring > dataNames;
		DateTime lastWriteTime;

		sourceInstance->getDataNames(dataNames);
		for (const auto& dataName : dataNames)
		{
			std::wstring fauxDataPath = sourceInstance->getPath() + L"$" + dataName;

			if (m_pipelineDb && sourceInstance->getDataLastWriteTime(dataName, lastWriteTime))
			{
				PipelineFileHash fileHash;
				if (m_pipelineDb->getFile(fauxDataPath, fileHash))
				{
					if (fileHash.lastWriteTime == lastWriteTime)
					{
						dependency->sourceDataHash += fileHash.hash;
						continue;
					}
				}
			}

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

				if (m_pipelineDb)
				{
					PipelineFileHash fileHash;
					fileHash.size = 0;
					fileHash.lastWriteTime = lastWriteTime;
					fileHash.hash = a32.get();
					m_pipelineDb->setFile(fauxDataPath, fileHash);
				}
			}
			else
				log::warning << L"Unable to read dependency instance data \"" << dataName << L"\", hash will be inconsistent." << Endl;
		}
	}

	// Calculate external file hashes.
	dependency->filesHash = 0;
	for (const auto& dependencyFile : dependency->files)
	{
		if (m_pipelineDb)
		{
			Ref< File > file = FileSystem::getInstance().get(dependencyFile.filePath);
			if (file)
			{
				PipelineFileHash fileHash;
				if (m_pipelineDb->getFile(dependencyFile.filePath, fileHash))
				{
					if (fileHash.lastWriteTime == file->getLastWriteTime())
					{
						dependency->filesHash += fileHash.hash;
						continue;
					}
				}
			}
		}

		Ref< IStream > fileStream = FileSystem::getInstance().open(dependencyFile.filePath, File::FmRead);
		if (fileStream)
		{
			uint8_t buffer[4096];
			Murmur3 a32;
			int64_t r;

			a32.begin();
			while ((r = fileStream->read(buffer, sizeof(buffer))) > 0)
				a32.feed(buffer, r);
			a32.end();

			dependency->filesHash += a32.get();
			fileStream->close();

			if (m_pipelineDb)
			{
				Ref< File > file = FileSystem::getInstance().get(dependencyFile.filePath);
				if (file)
				{
					PipelineFileHash fileHash;
					fileHash.size = file->getSize();
					fileHash.lastWriteTime = file->getLastWriteTime();
					fileHash.hash = a32.get();
					m_pipelineDb->setFile(dependencyFile.filePath, fileHash);
				}
			}
		}
		else
			log::warning << L"Unable to read dependency file \"" << dependencyFile.filePath.getPathName() << L"\", hash will be inconsistent." << Endl;
	}
}

}
