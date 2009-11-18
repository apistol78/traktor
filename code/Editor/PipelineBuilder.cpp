#include "Editor/PipelineBuilder.h"
#include "Editor/PipelineDependency.h"
#include "Editor/PipelineHash.h"
#include "Editor/IPipelineCache.h"
#include "Editor/IPipeline.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Isolate.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Misc/Adler32.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineBuilder", PipelineBuilder, IPipelineBuilder)

PipelineBuilder::PipelineBuilder(
	db::Database* sourceDatabase,
	db::Database* outputDatabase,
	IPipelineCache* cache,
	PipelineHash* hash,
	IListener* listener
)
:	m_sourceDatabase(sourceDatabase)
,	m_outputDatabase(outputDatabase)
,	m_cache(cache)
,	m_hash(hash)
,	m_listener(listener)
,	m_succeeded(0)
,	m_failed(0)
{
}

bool PipelineBuilder::build(const RefArray< PipelineDependency >& dependencies, bool rebuild)
{
	PipelineHash::Hash hash;

	// Check which dependencies are dirty; ie. need to be rebuilt.
	for (RefArray< PipelineDependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		(*i)->sourceAssetHash = DeepHash(checked_type_cast< const ISerializable* >((*i)->sourceAsset)).get();

		// Have source asset been modified?
		if (!rebuild)
		{
			if (!m_hash->get((*i)->outputGuid, hash))
			{
				log::info << L"Asset \"" << (*i)->name << L"\" modified; not hashed" << Endl;
				(*i)->reason |= IPipeline::BrSourceModified;
			}
			else if (hash.pipelineVersion != (*i)->pipeline->getVersion())
			{
				log::info << L"Asset \"" << (*i)->name << L"\" modified; pipeline version differ" << Endl;
				(*i)->reason |= IPipeline::BrSourceModified;
			}
			else if (hash.pipelineHash != (*i)->pipelineHash)
			{
				log::info << L"Asset \"" << (*i)->name << L"\" modified; pipeline settings differ" << Endl;
				(*i)->reason |= IPipeline::BrSourceModified;
			}
			else if (hash.sourceAssetHash != (*i)->sourceAssetHash)
			{
				log::info << L"Asset \"" << (*i)->name << L"\" modified; source has been modified" << Endl;
				(*i)->reason |= IPipeline::BrSourceModified;
			}
			else
			{
				const std::set< Path >& files = (*i)->files;
				for (std::set< Path >::const_iterator j = files.begin(); j != files.end(); ++j)
				{
					std::map< Path, DateTime >::const_iterator timeStampIt = hash.timeStamps.find(*j);
					if (timeStampIt != hash.timeStamps.end())
					{
						Ref< File > sourceFile = FileSystem::getInstance().get(*j);
						if (sourceFile && sourceFile->getLastWriteTime() != timeStampIt->second)
						{
							log::info << L"Asset \"" << (*i)->name << L"\" modified; file \"" << j->getPathName() << L" has been modified" << Endl;
							(*i)->reason |= IPipeline::BrSourceModified | IPipeline::BrAssetModified;
							break;
						}
					}
					else
					{
						log::info << L"Asset \"" << (*i)->name << L"\" modified; file \"" << j->getPathName() << L" has not been hashed" << Endl;
						(*i)->reason |= IPipeline::BrSourceModified | IPipeline::BrAssetModified;
						break;
					}
				}
			}
		}
		else
			(*i)->reason |= IPipeline::BrForced;
	}

	// Build assets which are dirty or have dirty dependency assets.
	m_succeeded = 0;
	m_failed = 0;

	for (RefArray< PipelineDependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		// Abort if current thread has been stopped; thread are stopped by worker dialog.
		if (ThreadManager::getInstance().getCurrentThread()->stopped())
			break;

		// Update hash entry; don't write it yet though.
		hash.pipelineVersion = (*i)->pipeline->getVersion();
		hash.pipelineHash = (*i)->pipelineHash;
		hash.sourceAssetHash = (*i)->sourceAssetHash;

		const std::set< Path >& files = (*i)->files;
		for (std::set< Path >::const_iterator j = files.begin(); j != files.end(); ++j)
		{
			Ref< File > sourceFile = FileSystem::getInstance().get(*j);
			if (sourceFile)
				hash.timeStamps[*j] = sourceFile->getLastWriteTime();
			else
				log::warning << L"Unable to read timestamp of file " << j->getPathName() << Endl;
		}

		// Skip building asset; just update hash.
		if (!(*i)->build)
		{
			m_hash->set((*i)->outputGuid, hash);
			continue;
		}

		// Check if we need to build asset; check the entire dependency chain (will update reason if dependency dirty).
		if (needBuild(*i))
		{
			ScopeIndent scopeIndent(log::info);
			uint32_t cacheHash;
			bool result;

			log::info << L"Building asset \"" << (*i)->name << L"\" (" << type_name((*i)->pipeline) << L")..." << Endl;
			log::info << IncreaseIndent;

			// Notify listener about we're beginning to build the asset.
			if (m_listener)
				m_listener->begunBuildingAsset(
					(*i)->name,
					uint32_t(std::distance(dependencies.begin(), i)),
					uint32_t(dependencies.size())
				);

			// Get output instances from cache.
			if (m_cache)
			{
				cacheHash = dependencyCacheHash(*i);
				result = getInstancesFromCache((*i)->outputGuid, cacheHash);
				if (result)
				{
					log::info << L"Cached instance(s) used" << Endl;

					m_hash->set((*i)->outputGuid, hash);
					m_succeeded++;
				}
			}
			else
				result = false;

			if (!result)
			{
				// Build output instances; keep an array of written instances as we
				// need them to update the cache.
				m_builtInstances.resize(0);

				result = (*i)->pipeline->buildOutput(
					this,
					(*i)->sourceAsset,
					(*i)->sourceAssetHash,
					(*i)->buildParams,
					(*i)->outputPath,
					(*i)->outputGuid,
					(*i)->reason
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
								(*i)->outputGuid,
								cacheHash,
								m_builtInstances
							);

						log::info << DecreaseIndent;
					}

					m_hash->set((*i)->outputGuid, hash);
					m_succeeded++;
				}
				else
					m_failed++;
			}

			log::info << DecreaseIndent;
			log::info << (result ? L"Build successful" : L"Build failed") << Endl;
		}
	}

	// Log results.
	if (!ThreadManager::getInstance().getCurrentThread()->stopped())
		log::info << L"Build finished; " << m_succeeded << L" succeeded, " << m_failed << L" failed" << Endl;
	else
		log::info << L"Build finished; aborted" << Endl;

	return true;
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
	Ref< db::Instance > instance = m_outputDatabase->createInstance(
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

bool PipelineBuilder::needBuild(PipelineDependency* dependency) const
{
	if (dependency->reason != IPipeline::BrNone)
		return true;

	for (RefArray< PipelineDependency >::const_iterator i = dependency->children.begin(); i != dependency->children.end(); ++i)
	{
		if (needBuild(*i))
		{
			dependency->reason |= IPipeline::BrDependencyModified;
			return true;
		}
	}

	return false;
}

bool PipelineBuilder::putInstancesInCache(const Guid& guid, uint32_t hash, const RefArray< db::Instance >& instances)
{
	bool result = false;

	Ref< IStream > stream = m_cache->put(guid, hash);
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

bool PipelineBuilder::getInstancesFromCache(const Guid& guid, uint32_t hash)
{
	bool result = false;

	Ref< IStream > stream = m_cache->get(guid, hash);
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

uint32_t PipelineBuilder::dependencyCacheHash(const PipelineDependency* dependency)
{
	uint32_t hash = 0UL;

	const std::set< Path >& files = dependency->files;
	for (std::set< Path >::const_iterator i = files.begin(); i != files.end(); ++i)
		hash += externalFileHash(*i);

	hash += dependency->pipelineHash;
	hash += dependency->sourceAssetHash;

	const RefArray< PipelineDependency >& children = dependency->children;
	for (RefArray< PipelineDependency >::const_iterator i = children.begin(); i != children.end(); ++i)
		hash += dependencyCacheHash(*i);

	return hash;
}

uint32_t PipelineBuilder::externalFileHash(const Path& path)
{
	std::map< Path, uint32_t >::const_iterator i = m_externalFileHash.find(path);
	if (i != m_externalFileHash.end())
		return i->second;

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

	m_externalFileHash.insert(std::make_pair(path, hash));
	return hash;
}

	}
}
