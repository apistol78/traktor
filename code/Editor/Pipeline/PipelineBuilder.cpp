#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Isolate.h"
#include "Editor/IPipelineCache.h"
#include "Editor/IPipelineDb.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/PipelineBuilder.h"
#include "Editor/Pipeline/PipelineDependency.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineBuilder", PipelineBuilder, IPipelineBuilder)

PipelineBuilder::PipelineBuilder(
	PipelineFactory* pipelineFactory,
	db::Database* sourceDatabase,
	db::Database* outputDatabase,
	IPipelineCache* cache,
	IPipelineDb* db,
	IListener* listener
)
:	m_pipelineFactory(pipelineFactory)
,	m_sourceDatabase(sourceDatabase)
,	m_outputDatabase(outputDatabase)
,	m_cache(cache)
,	m_db(db)
,	m_listener(listener)
{
}

bool PipelineBuilder::build(const RefArray< PipelineDependency >& dependencies, bool rebuild)
{
	int32_t succeeded = 0, failed = 0;
	Timer timer;

	timer.start();

	// Update dependency hashes.
	for (RefArray< PipelineDependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		updateLocalHashes(*i);

	// Check which dependencies are dirty; ie. need to be rebuilt.
	for (RefArray< PipelineDependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		updateBuildReason(*i, rebuild);

	log::debug << L"Pipeline build; analyze " << int32_t(timer.getElapsedTime() * 1000) << L" ms" << Endl;

	// Build assets which are dirty or have dirty dependency assets.
	for (RefArray< PipelineDependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		// Abort if current thread has been stopped; thread are stopped by worker dialog.
		if (ThreadManager::getInstance().getCurrentThread()->stopped())
			break;

		// Notify listener about we're beginning to build the asset.
		if (m_listener)
			m_listener->begunBuildingAsset(
			(*i)->name,
			uint32_t(std::distance(dependencies.begin(), i)),
			uint32_t(dependencies.size())
		);

		if (performBuild(*i))
			++succeeded;
		else
			++failed;
	}

	log::debug << L"Pipeline build; total " << int32_t(timer.getElapsedTime() * 1000) << L" ms" << Endl;

	// Log results.
	if (!ThreadManager::getInstance().getCurrentThread()->stopped())
		log::info << L"Build finished; " << succeeded << L" succeeded, " << failed << L" failed" << Endl;
	else
		log::info << L"Build finished; aborted" << Endl;

	return true;
}

bool PipelineBuilder::buildOutput(const ISerializable* sourceAsset, const Object* buildParams, const std::wstring& name, const std::wstring& outputPath, const Guid& outputGuid)
{
	Ref< IPipeline > pipeline;
	uint32_t pipelineHash;

	if (!m_pipelineFactory->findPipeline(type_of(sourceAsset), pipeline, pipelineHash))
		return false;

	return pipeline->buildOutput(this, sourceAsset, 0, buildParams, outputPath, outputGuid, PbrSourceModified);
}

Ref< db::Database > PipelineBuilder::getSourceDatabase() const
{
	return m_sourceDatabase;
}

Ref< db::Database > PipelineBuilder::getOutputDatabase() const
{
	return m_outputDatabase;
}

Ref< db::Instance > PipelineBuilder::createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid)
{
	Ref< db::Instance > instance;

	instance = m_outputDatabase->getInstance(instanceGuid);
	if (instance)
	{
		bool result = false;

		// Existing instance with same guid already exists; remove existing instance.
		if (instance->checkout())
		{
			result = instance->remove();
			result &= instance->commit();
		}

		if (!result)
			return 0;
	}

	instance = m_outputDatabase->createInstance(
		instancePath,
		db::CifReplaceExisting,
		&instanceGuid
	);
	if (instance)
	{
		m_builtInstances.push_back(instance);
		return instance;
	}
	else
		return 0;
}

Ref< const ISerializable > PipelineBuilder::getObjectReadOnly(const Guid& instanceGuid)
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

Ref< IPipelineReport > PipelineBuilder::createReport(const std::wstring& name, const Guid& guid)
{
	return m_db->createReport(name, guid);
}

void PipelineBuilder::updateLocalHashes(PipelineDependency* dependency)
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

		// File has either been modified or is new; calculate hash and update cache.
		previousFileHash.size = file->getSize();
		previousFileHash.lastWriteTime = file->getLastWriteTime();
		previousFileHash.hash = externalFileHash(*j);

		m_db->setFile(*j, previousFileHash);

		dependency->dependencyHash += previousFileHash.hash;
	}

	dependency->dependencyHash += dependency->pipelineHash;
	dependency->dependencyHash += dependency->sourceAssetHash;
}

void PipelineBuilder::updateBuildReason(PipelineDependency* dependency, bool rebuild)
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
			log::info << L"Asset \"" << dependency->name << L"\" modified; not hashed" << Endl;
			dependency->reason |= PbrSourceModified;
		}
		else if (previousDependencyHash.pipelineVersion != type_of(dependency->pipeline).getVersion())
		{
			log::info << L"Asset \"" << dependency->name << L"\" modified; pipeline version differ" << Endl;
			dependency->reason |= PbrSourceModified;
		}
		else if (previousDependencyHash.hash != dependencyHash)
		{
			log::info << L"Asset \"" << dependency->name << L"\" modified; source has been modified" << Endl;
			dependency->reason |= PbrSourceModified;
		}
	}
	else
		dependency->reason |= PbrForced;
}

bool PipelineBuilder::performBuild(PipelineDependency* dependency)
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
		T_ANONYMOUS_VAR(ScopeIndent)(log::info);

		log::info << L"Building asset \"" << dependency->name << L"\" (" << type_name(dependency->pipeline) << L")..." << Endl;
		log::info << IncreaseIndent;

		// Get output instances from cache.
		if (m_cache)
		{
			result = getInstancesFromCache(dependency->outputGuid, currentDependencyHash.hash, currentDependencyHash.pipelineVersion);
			if (result)
			{
				log::info << L"Cached instance(s) used" << Endl;
				m_db->setDependency(dependency->outputGuid, currentDependencyHash);
			}
		}
		else
			result = false;

		if (!result)
		{
			// Build output instances; keep an array of written instances as we
			// need them to update the cache.
			m_builtInstances.resize(0);

			result = dependency->pipeline->buildOutput(
				this,
				dependency->sourceAsset,
				dependency->sourceAssetHash,
				dependency->buildParams,
				dependency->outputPath,
				dependency->outputGuid,
				dependency->reason
			);

			if (result)
			{
				if (!m_builtInstances.empty())
				{
					log::info << L"Instance(s) built:" << Endl;
					log::info << IncreaseIndent;

					for (RefArray< db::Instance >::const_iterator j = m_builtInstances.begin(); j != m_builtInstances.end(); ++j)
						log::info << L"\"" << (*j)->getPath() << L"\"" << Endl;

					if (m_cache)
						putInstancesInCache(
							dependency->outputGuid,
							currentDependencyHash.hash,
							currentDependencyHash.pipelineVersion,
							m_builtInstances
						);

					log::info << DecreaseIndent;
				}

				m_db->setDependency(dependency->outputGuid, currentDependencyHash);
			}
		}

		log::info << DecreaseIndent;
		log::info << (result ? L"Build successful" : L"Build failed") << Endl;
	}

	return result;
}

uint32_t PipelineBuilder::calculateGlobalHash(const PipelineDependency* dependency) const
{
	uint32_t hash = dependency->dependencyHash;
	for (RefArray< PipelineDependency >::const_iterator i = dependency->children.begin(); i != dependency->children.end(); ++i)
	{
		if (((*i)->flags & PdfUse) != 0)
			hash += calculateGlobalHash(*i);
	}
	return hash;
}

bool PipelineBuilder::needBuild(PipelineDependency* dependency) const
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

bool PipelineBuilder::putInstancesInCache(const Guid& guid, uint32_t hash, int32_t version, const RefArray< db::Instance >& instances)
{
	bool result = false;

	Ref< IStream > stream = m_cache->put(guid, hash, version);
	if (stream)
	{
		Writer writer(stream);

		writer << uint32_t(instances.size());
		for (uint32_t i = 0; i < uint32_t(instances.size()); ++i)
		{
			std::wstring groupPath = instances[i]->getParent()->getPath();
			writer << groupPath;

			result = db::Isolate::createIsolatedInstance(instances[i], stream);
			if (!result)
				break;
		}

		stream->close();
	}

	return result;
}

bool PipelineBuilder::getInstancesFromCache(const Guid& guid, uint32_t hash, int32_t version)
{
	bool result = false;

	Ref< IStream > stream = m_cache->get(guid, hash, version);
	if (stream)
	{
		Reader reader(stream);

		uint32_t instanceCount;
		reader >> instanceCount;

		result = true;

		for (uint32_t i = 0; i < instanceCount; ++i)
		{
			std::wstring groupPath;
			reader >> groupPath;

			Ref< db::Group > group = m_outputDatabase->createGroup(groupPath);
			if (!group)
			{
				result = false;
				break;
			}

			if (!db::Isolate::createInstanceFromIsolation(group, stream))
			{
				result = false;
				break;
			}
		}
	
		stream->close();
	}

	return result;
}

uint32_t PipelineBuilder::externalFileHash(const Path& path)
{
	Adler32 adler;
	adler.begin();

	Ref< IStream > stream = FileSystem::getInstance().open(path, File::FmRead);
	if (stream)
	{
		uint8_t buffer[4096];
		int32_t nread;
		while ((nread = stream->read(buffer, sizeof(buffer))) > 0)
			adler.feed(buffer, nread);
		stream->close();
	}
	else
		log::warning << L"Unable to open file \"" << path.getPathName() << L"\"; inconsistent hash" << Endl;

	adler.end();
	uint32_t hash = adler.get();

	return hash;
}

	}
}
