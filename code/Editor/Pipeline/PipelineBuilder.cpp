#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
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
		namespace
		{

class LogTargetFilter : public ILogTarget
{
public:
	LogTargetFilter(ILogTarget* target)
	:	m_target(target)
	,	m_count(0)
	{
	}

	virtual void log(const std::wstring& str)
	{
		++m_count;
		if (m_target)
			m_target->log(str);
	}

	ILogTarget* getTarget() const { return m_target; }

	uint32_t getCount() const { return m_count; }

private:
	Ref< ILogTarget > m_target;
	uint32_t m_count;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineBuilder", PipelineBuilder, IPipelineBuilder)

PipelineBuilder::PipelineBuilder(
	PipelineFactory* pipelineFactory,
	db::Database* sourceDatabase,
	db::Database* outputDatabase,
	IPipelineCache* cache,
	IPipelineDb* db,
	IListener* listener,
	bool threadedBuildEnable
)
:	m_pipelineFactory(pipelineFactory)
,	m_sourceDatabase(sourceDatabase)
,	m_outputDatabase(outputDatabase)
,	m_cache(cache)
,	m_db(db)
,	m_listener(listener)
,	m_threadedBuildEnable(threadedBuildEnable)
,	m_progress(0)
,	m_progressEnd(0)
,	m_succeeded(0)
,	m_failed(0)
{
}

bool PipelineBuilder::build(const RefArray< PipelineDependency >& dependencies, bool rebuild)
{
	T_ANONYMOUS_VAR(ScopeIndent)(log::info);
	T_ANONYMOUS_VAR(ScopeIndent)(log::warning);
	T_ANONYMOUS_VAR(ScopeIndent)(log::error);
	T_ANONYMOUS_VAR(ScopeIndent)(log::debug);

	m_db->beginTransaction();

	Timer timer;
	timer.start();

	// Update dependency hashes.
	for (RefArray< PipelineDependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		updateLocalHashes(*i);

	T_DEBUG(L"Pipeline build; analyzed local hashes in " << int32_t(timer.getDeltaTime() * 1000) << L" ms");

	// Check which dependencies are dirty; ie. need to be rebuilt.
	for (RefArray< PipelineDependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		updateBuildReason(*i, rebuild);

	T_DEBUG(L"Pipeline build; analyzed build reasons in " << int32_t(timer.getDeltaTime() * 1000) << L" ms");

	m_db->endTransaction();

	m_progress = 0;
	m_progressEnd = dependencies.size();
	m_succeeded = 0;
	m_failed = 0;

	m_db->beginTransaction();

	// Split workload on threads; use as many threads as there are CPU cores.
	// If build set is less than twice number of cores we don't build asynchronously.
	uint32_t cpuCores = OS::getInstance().getCPUCoreCount();
	if (m_threadedBuildEnable && dependencies.size() >= cpuCores * 2)
	{
		std::vector< Thread* > threads(cpuCores, 0);

		// Spawn thread for each slice of the work load.
		for (uint32_t i = 0; i < cpuCores; ++i)
		{
			uint32_t offsetStart = (dependencies.size() * i) / cpuCores;
			uint32_t offsetEnd = (dependencies.size() * (i + 1)) / cpuCores;
			if (offsetStart < offsetEnd)
			{
				threads[i] = ThreadManager::getInstance().create(
					makeFunctor(
						this,
						&PipelineBuilder::buildThread,
						ThreadManager::getInstance().getCurrentThread(),
						dependencies.begin() + offsetStart,
						dependencies.begin() + offsetEnd,
						i
					),
					L"Build thread"
				);
				threads[i]->start();
			}
		}

		// Wait until all threads have finished.
		for (uint32_t i = 0; i < cpuCores; ++i)
		{
			if (threads[i])
			{
				threads[i]->wait();
				ThreadManager::getInstance().destroy(threads[i]);
			}
		}
	}
	else
	{
		// Invoke thread method directly to build all dependencies.
		buildThread(ThreadManager::getInstance().getCurrentThread(), dependencies.begin(), dependencies.end(), 0);
	}

	m_db->endTransaction();

	T_DEBUG(L"Pipeline build; total " << int32_t(timer.getElapsedTime() * 1000) << L" ms");

	// Log results.
	if (!ThreadManager::getInstance().getCurrentThread()->stopped())
	{
		//for (std::map< const TypeInfo*, double >::const_iterator i = m_buildTimes.begin(); i != m_buildTimes.end(); ++i)
		//	log::info << L"Pipeline \"" << i->first->getName() << L"\" " << int32_t(i->second * 1000.0) << L" ms" << Endl;

		log::info << L"Build finished; " << m_succeeded << L" succeeded, " << m_failed << L" failed" << Endl;
	}
	else
		log::info << L"Build finished; aborted" << Endl;

	return m_failed == 0;
}

Ref< ISerializable > PipelineBuilder::buildOutput(const ISerializable* sourceAsset)
{
	if (!sourceAsset)
		return 0;

	uint32_t sourceHash = DeepHash(sourceAsset).get();

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_builtCacheLock);

		std::map< uint32_t, built_cache_list_t >::iterator i = m_builtCache.find(sourceHash);
		if (i != m_builtCache.end())
		{
			built_cache_list_t& bcl = i->second;
			T_ASSERT (!bcl.empty());

			// Return same instance as before if pointer and hash match.
			for (built_cache_list_t::const_iterator j = bcl.begin(); j != bcl.end(); ++j)
			{
				if (j->sourceAsset == sourceAsset)
					return j->product;
			}

			// Hash matching but no product found; need to clone a product.
			Ref< ISerializable > product = DeepClone(i->second.front().product).create();
			if (!product)
				return 0;

			BuiltCacheEntry bce;
			bce.sourceAsset = sourceAsset;
			bce.product = product;
			bcl.push_back(bce);

			return product;
		}
	}

	Ref< IPipeline > pipeline;
	uint32_t pipelineHash;

	if (!m_pipelineFactory->findPipeline(type_of(sourceAsset), pipeline, pipelineHash))
		return 0;

	Timer timer;
	timer.start();

	Ref< ISerializable > product = pipeline->buildOutput(this, sourceAsset);
	if (!product)
		return 0;

	double buildTime = timer.getElapsedTime();

	//{
	//	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_buildTimesLock);
	//	m_buildTimes[&type_of(pipeline)] += buildTime;
	//}

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_builtCacheLock);
		
		BuiltCacheEntry bce;
		bce.sourceAsset = sourceAsset;
		bce.product = product;
		m_builtCache[sourceHash].push_back(bce);
	}

	return product;
}

bool PipelineBuilder::buildOutput(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, const Object* buildParams)
{
	Ref< IPipeline > pipeline;
	uint32_t pipelineHash;

	if (!m_pipelineFactory->findPipeline(type_of(sourceAsset), pipeline, pipelineHash))
		return false;

	Timer timer;
	timer.start();

	if (!pipeline->buildOutput(this, sourceAsset, 0, outputPath, outputGuid, buildParams, PbrSourceModified))
		return false;

	double buildTime = timer.getElapsedTime();

	//{
	//	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_buildTimesLock);
	//	m_buildTimes[&type_of(pipeline)] += buildTime;
	//}

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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_createOutputLock);
	Ref< db::Instance > instance;

	if (instanceGuid.isNull() || !instanceGuid.isValid())
	{
		log::error << L"Invalid guid for output instance" << Endl;
		return 0;
	}

	instance = m_outputDatabase->getInstance(instanceGuid);
	if (instance && instancePath != instance->getPath())
	{
		// Instance with given guid already exist somewhere else, we need to
		// remove it first.
		bool result = false;
		if (instance->checkout())
		{
			result = instance->remove();
			result &= instance->commit();
		}
		if (!result)
		{
			log::error << L"Unable to remove existing instance \"" << instance->getPath() << L"\"" << Endl;
			return 0;
		}
	}

	instance = m_outputDatabase->createInstance(
		instancePath,
		db::CifReplaceExisting,
		&instanceGuid
	);
	if (instance)
	{
		RefArray< db::Instance >* builtInstances = reinterpret_cast< RefArray< db::Instance >* >(m_buildInstances.get());
		if (builtInstances)
			builtInstances->push_back(instance);
		return instance;
	}
	else
		return 0;
}

Ref< const ISerializable > PipelineBuilder::getObjectReadOnly(const Guid& instanceGuid)
{
	Ref< ISerializable > object;

	// Get object from cache if already acquired.
	{
		m_readCacheLock.acquireReader();
		std::map< Guid, Ref< ISerializable > >::iterator i = m_readCache.find(instanceGuid);
		if (i != m_readCache.end())
			object = i->second;
		m_readCacheLock.releaseReader();
	}

	// If not acquired then read from database.
	if (!object)
	{
		m_readCacheLock.acquireWriter();
		object = m_sourceDatabase->getObjectReadOnly(instanceGuid);
		m_readCache[instanceGuid] = object;
		m_readCacheLock.releaseWriter();
	}

	return object;
}

Ref< IStream > PipelineBuilder::openFile(const Path& basePath, const std::wstring& fileName)
{
	Path filePath = FileSystem::getInstance().getAbsolutePath(basePath + Path(fileName));
	Ref< IStream > fileStream = FileSystem::getInstance().open(filePath, File::FmRead);
	return fileStream ? new BufferedStream(fileStream) : 0;
}

Ref< IStream > PipelineBuilder::createTemporaryFile(const std::wstring& fileName)
{
	Ref< IStream > fileStream = FileSystem::getInstance().open(L"data/temp/" + fileName, File::FmWrite);
	return fileStream ? new BufferedStream(fileStream) : 0;
}

Ref< IStream > PipelineBuilder::openTemporaryFile(const std::wstring& fileName)
{
	Ref< IStream > fileStream = FileSystem::getInstance().open(L"data/temp/" + fileName, File::FmRead);
	return fileStream ? new BufferedStream(fileStream) : 0;
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
		else
		{
			// File doesn't exist in cache; ensure cycle is reset.
			previousFileHash.hash = 0;
		}

		// File has either been modified or is new; calculate hash and update cache.
		previousFileHash.size = file->getSize();
		previousFileHash.lastWriteTime = file->getLastWriteTime();
		previousFileHash.hash++;

		m_db->setFile(*j, previousFileHash);

		dependency->dependencyHash += previousFileHash.hash;
	}

	dependency->dependencyHash += dependency->pipelineHash;
	dependency->dependencyHash += dependency->sourceDataHash;
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
			log::info << L"Asset \"" << dependency->outputPath << L"\" modified; not hashed" << Endl;
			dependency->reason |= PbrSourceModified;
		}
		else if (previousDependencyHash.pipelineVersion != type_of(dependency->pipeline).getVersion())
		{
			log::info << L"Asset \"" << dependency->outputPath << L"\" modified; pipeline version differ" << Endl;
			dependency->reason |= PbrSourceModified;
		}
		else if (previousDependencyHash.hash != dependencyHash)
		{
			log::info << L"Asset \"" << dependency->outputPath << L"\" modified; source has been modified" << Endl;
			dependency->reason |= PbrSourceModified;
		}
	}
	else
		dependency->reason |= PbrForced;
}

IPipelineBuilder::BuildResult PipelineBuilder::performBuild(PipelineDependency* dependency)
{
	IPipelineDb::DependencyHash currentDependencyHash;

	// Create hash entry.
	currentDependencyHash.pipelineVersion = type_of(dependency->pipeline).getVersion();
	currentDependencyHash.hash = calculateGlobalHash(dependency);

	// Skip no-build asset; just update hash.
	if ((dependency->flags & PdfBuild) == 0)
	{
		if ((dependency->reason & PbrSourceModified) != 0)
			m_db->setDependency(dependency->outputGuid, currentDependencyHash);
		return BrSucceeded;
	}

	// Check if we need to build asset; check the entire dependency chain (will update reason if dependency dirty).
	if (!needBuild(dependency))
		return BrSucceeded;

	T_ANONYMOUS_VAR(ScopeIndent)(log::info);

	log::info << L"Building asset \"" << dependency->outputPath << L"\" (" << type_name(dependency->pipeline) << L")..." << Endl;
	log::info << IncreaseIndent;

	// Get output instances from cache.
	if (m_cache)
	{
		if (getInstancesFromCache(dependency->outputGuid, currentDependencyHash.hash, currentDependencyHash.pipelineVersion))
		{
			log::info << L"Cached instance(s) used" << Endl;
			m_db->setDependency(dependency->outputGuid, currentDependencyHash);
			return BrSucceeded;
		}
	}

	// Build output instances; keep an array of written instances as we
	// need them to update the cache.
	RefArray< db::Instance > builtInstances;
	m_buildInstances.set(&builtInstances);

	LogTargetFilter infoTarget(log::info.getLocalTarget());
	LogTargetFilter warningTarget(log::warning.getLocalTarget());
	LogTargetFilter errorTarget(log::error.getLocalTarget());

	log::info.setLocalTarget(&infoTarget);
	log::warning.setLocalTarget(&warningTarget);
	log::error.setLocalTarget(&errorTarget);

	Timer timer;
	timer.start();

	bool result = dependency->pipeline->buildOutput(
		this,
		dependency->sourceAsset,
		dependency->sourceAssetHash,
		dependency->outputPath,
		dependency->outputGuid,
		0,
		dependency->reason
	);

	double buildTime = timer.getElapsedTime();

	log::info.setLocalTarget(infoTarget.getTarget());
	log::warning.setLocalTarget(warningTarget.getTarget());
	log::error.setLocalTarget(errorTarget.getTarget());

	if (errorTarget.getCount() > 0)
		result = false;

	if (result)
	{
		if (!builtInstances.empty())
		{
			log::info << L"Instance(s) built:" << Endl;
			log::info << IncreaseIndent;

			for (RefArray< db::Instance >::const_iterator j = builtInstances.begin(); j != builtInstances.end(); ++j)
				log::info << L"\"" << (*j)->getPath() << L"\" " << (*j)->getGuid().format() << Endl;

			if (m_cache)
				putInstancesInCache(
					dependency->outputGuid,
					currentDependencyHash.hash,
					currentDependencyHash.pipelineVersion,
					builtInstances
				);

			log::info << DecreaseIndent;
		}

		m_db->setDependency(dependency->outputGuid, currentDependencyHash);
	}

	log::info << DecreaseIndent;
	log::info << (result ? L"Build successful" : L"Build failed") << Endl;

	if (result)
		return warningTarget.getCount() > 0 ? BrSucceededWithWarnings : BrSucceeded;
	else
		return BrFailed;
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

void PipelineBuilder::buildThread(Thread* controlThread, RefArray< PipelineDependency >::const_iterator begin, RefArray< PipelineDependency >::const_iterator end, uint32_t core)
{
	for (RefArray< PipelineDependency >::const_iterator i = begin; i != end; ++i)
	{
		// Abort if control thread has been stopped; thread are stopped by worker dialog.
		if (controlThread->stopped())
			break;

		if (m_listener)
			m_listener->beginBuild(
				core,
				m_progress,
				m_progressEnd,
				*i
			);

		BuildResult result = performBuild(*i);
		if (result == BrSucceeded || result == BrSucceededWithWarnings)
			++m_succeeded;
		else
			++m_failed;

		if (m_listener)
			m_listener->endBuild(
				core,
				m_progress,
				m_progressEnd,
				*i,
				result
			);

		++m_progress;
	}
}

	}
}
