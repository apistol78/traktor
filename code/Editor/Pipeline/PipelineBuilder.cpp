#if defined(_WIN32)
#	include <cfloat>
#endif
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
#include "Core/Thread/ThreadPool.h"
#include "Core/Timer/Timer.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Isolate.h"
#include "Editor/IPipeline.h"
#include "Editor/IPipelineCache.h"
#include "Editor/IPipelineDb.h"
#include "Editor/IPipelineDependencySet.h"
#include "Editor/IPipelineInstanceCache.h"
#include "Editor/PipelineDependency.h"
#include "Editor/Pipeline/PipelineBuilder.h"
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

	virtual void log(int32_t level, const std::wstring& str)
	{
		++m_count;
		if (m_target)
			m_target->log(level, str);
	}

	ILogTarget* getTarget() const { return m_target; }

	uint32_t getCount() const { return m_count; }

private:
	Ref< ILogTarget > m_target;
	uint32_t m_count;
};

struct WorkSetSortPredicate
{
	bool operator () (const std::pair< uint32_t, Ref< const Object > >& a, const std::pair< uint32_t, Ref< const Object > >& b) const
	{
		if (a.second && !b.second)
			return true;
		else
			return false;
	}
};

void calculateGlobalHash(
	const IPipelineDependencySet* dependencySet,
	const PipelineDependency* dependency,
	uint32_t& outPipelineHash,
	uint32_t& outSourceAssetHash,
	uint32_t& outSourceDataHash,
	uint32_t& outFilesHash
)
{
	outPipelineHash += dependency->pipelineHash;
	outSourceAssetHash += dependency->sourceAssetHash;
	outSourceDataHash += dependency->sourceDataHash;
	outFilesHash += dependency->filesHash;

	for (std::vector< uint32_t >::const_iterator i = dependency->children.begin(); i != dependency->children.end(); ++i)
	{
		const PipelineDependency* childDependency = dependencySet->get(*i);
		T_ASSERT (childDependency);

		if ((childDependency->flags & PdfUse) != 0)
			calculateGlobalHash(
				dependencySet,
				childDependency,
				outPipelineHash,
				outSourceAssetHash,
				outSourceDataHash,
				outFilesHash
			);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineBuilder", PipelineBuilder, IPipelineBuilder)

PipelineBuilder::PipelineBuilder(
	PipelineFactory* pipelineFactory,
	db::Database* sourceDatabase,
	db::Database* outputDatabase,
	IPipelineCache* cache,
	IPipelineDb* pipelineDb,
	IPipelineInstanceCache* instanceCache,
	IListener* listener,
	bool threadedBuildEnable
)
:	m_pipelineFactory(pipelineFactory)
,	m_sourceDatabase(sourceDatabase)
,	m_outputDatabase(outputDatabase)
,	m_cache(cache)
,	m_pipelineDb(pipelineDb)
,	m_instanceCache(instanceCache)
,	m_listener(listener)
,	m_threadedBuildEnable(threadedBuildEnable)
,	m_progress(0)
,	m_progressEnd(0)
,	m_succeeded(0)
,	m_succeededBuilt(0)
,	m_failed(0)
{
}

bool PipelineBuilder::build(const IPipelineDependencySet* dependencySet, bool rebuild)
{
	T_ANONYMOUS_VAR(ScopeIndent)(log::info);
	T_ANONYMOUS_VAR(ScopeIndent)(log::warning);
	T_ANONYMOUS_VAR(ScopeIndent)(log::error);
	T_ANONYMOUS_VAR(ScopeIndent)(log::debug);

	Timer timer;
	timer.start();

	// Ensure FP is in known state.
#if defined(_WIN32)
	uint32_t dummy;
	_controlfp_s(&dummy, 0, 0);
	_controlfp_s(&dummy,_PC_24, _MCW_PC);
	_controlfp_s(&dummy,_RC_NEAR, _MCW_RC);
#endif

	log::info << L"Analyzing build conditions..." << Endl;

	// Determine build reasons.
	uint32_t dependencyCount = dependencySet->size();
	
	m_reasons.resize(dependencyCount, 0);
	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const PipelineDependency* dependency = dependencySet->get(i);
		T_ASSERT (dependency);

		if ((dependency->flags & PdfFailed) != 0)
			continue;

		// Have source asset been modified?
		if (!rebuild)
		{
			uint32_t pipelineHash = 0;
			uint32_t sourceAssetHash = 0;
			uint32_t sourceDataHash = 0;
			uint32_t filesHash = 0;

			calculateGlobalHash(
				dependencySet,
				dependency,
				pipelineHash,
				sourceAssetHash,
				sourceDataHash,
				filesHash
			);

			uint32_t hash = pipelineHash + sourceAssetHash + sourceDataHash + filesHash;

			// Get hash entry from database.
			IPipelineDb::DependencyHash previousDependencyHash;
			if (!m_pipelineDb->getDependency(dependency->outputGuid, previousDependencyHash))
			{
#if defined(_DEBUG)
				log::info << L"Asset \"" << dependency->outputPath << L"\" modified; not hashed" << Endl;
#endif
				m_reasons[i] |= PbrSourceModified;
			}
			else if (previousDependencyHash.pipelineVersion != dependency->pipelineType->getVersion())
			{
#if defined(_DEBUG)
				log::info << L"Asset \"" << dependency->outputPath << L"\" modified; pipeline version differ" << Endl;
#endif
				m_reasons[i] |= PbrSourceModified;
			}
			else if (previousDependencyHash.hash != hash)
			{
#if defined(_DEBUG)
				log::info << L"Asset \"" << dependency->outputPath << L"\" modified; source has been modified" << Endl;
				log::info << IncreaseIndent;
				log::info << L"("; FormatHex(log::info, previousDependencyHash.hash, 8); log::info << L" != "; FormatHex(log::info, hash, 8); log::info << L")" << Endl;
				log::info << L"Pipeline hash "; FormatHex(log::info, pipelineHash, 8); log::info << Endl;
				log::info << L"Source asset hash "; FormatHex(log::info, sourceAssetHash, 8); log::info << Endl;
				log::info << L"Source data hash "; FormatHex(log::info, sourceDataHash, 8); log::info << Endl;
				log::info << L"File(s) hash "; FormatHex(log::info, filesHash, 8); log::info << Endl;
				log::info << DecreaseIndent;
#endif

				m_reasons[i] |= PbrSourceModified;
			}
		}
		else
			m_reasons[i] |= PbrForced;
	}

	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const PipelineDependency* dependency = dependencySet->get(i);
		T_ASSERT (dependency);

		std::vector< uint32_t > children = dependency->children;
		std::set< uint32_t > visited;

		visited.insert(i);
		while (!children.empty())
		{
			if (visited.find(children.back()) != visited.end())
			{
				children.pop_back();
				continue;
			}

			const PipelineDependency* childDependency = dependencySet->get(children.back());
			T_ASSERT (childDependency);

			if ((childDependency->flags & PdfUse) == 0)
			{
				children.pop_back();
				continue;
			}

			if ((m_reasons[children.back()] & PbrSourceModified) != 0)
				m_reasons[i] |= PbrDependencyModified;
			
			visited.insert(children.back());
			children.pop_back();
			children.insert(children.end(), childDependency->children.begin(), childDependency->children.end());
		}

		if (m_reasons[i] != 0)
			m_workSet.push_back(std::make_pair(i, Ref< const Object >()));
	}

	T_DEBUG(L"Pipeline build; analyzed build reasons in " << int32_t(timer.getDeltaTime() * 1000) << L" ms");

	log::info << L"Dispatching builds..." << Endl;

	// Sort work set to build those with user parameters first.
	m_workSet.sort(WorkSetSortPredicate());

	m_progress = 0;
	m_progressEnd = m_workSet.size();
	m_succeeded = dependencyCount - m_progressEnd;
	m_succeededBuilt = 0;
	m_failed = 0;

	int32_t cpuCores = OS::getInstance().getCPUCoreCount();
	if (
		m_threadedBuildEnable &&
		m_workSet.size() >= cpuCores * 2
	)
	{
		std::vector< Thread* > threads(cpuCores, (Thread*)0);
		for (int32_t i = 0; i < cpuCores; ++i)
		{
			ThreadPool::getInstance().spawn(
				makeFunctor
				<
					PipelineBuilder,
					const IPipelineDependencySet*,
					Thread*,
					int32_t
				>
				(
					this,
					&PipelineBuilder::buildThread,
					dependencySet,
					ThreadManager::getInstance().getCurrentThread(),
					i
				),
				threads[i]
			);
		}

		for (int32_t i = 0; i < cpuCores; ++i)
		{
			if (threads[i])
			{
				ThreadPool::getInstance().join(threads[i]);
				threads[i] = 0;
			}
		}
	}
	else
	{
		buildThread(
			dependencySet,
			ThreadManager::getInstance().getCurrentThread(),
			0
		);
	}

	T_DEBUG(L"Pipeline build; total " << int32_t(timer.getElapsedTime() * 1000) << L" ms");

	// Log results.
	if (!ThreadManager::getInstance().getCurrentThread()->stopped())
		log::info << L"Build finished; " << m_succeeded << L" succeeded (" << m_succeededBuilt << L" built), " << m_failed << L" failed" << Endl;
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
		}
	}

	const TypeInfo* pipelineType;
	uint32_t pipelineHash;

	if (!m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
		return 0;

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*pipelineType);
	T_ASSERT (pipeline);

	Ref< ISerializable > product = pipeline->buildOutput(this, sourceAsset);
	if (!product)
		return 0;

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
	const TypeInfo* pipelineType;
	uint32_t pipelineHash;

	if (!m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
		return false;

#if 1

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*pipelineType);
	T_ASSERT (pipeline);

	if (!pipeline->buildOutput(
		this,
		0,
		0,
		0,
		sourceAsset,
		0,
		outputPath,
		outputGuid,
		buildParams,
		PbrSourceModified
	))
		return false;

#else

	Ref< PipelineDependency > dependency = new PipelineDependency();
	dependency->pipelineType = pipelineType;
	dependency->sourceAsset = sourceAsset;
	dependency->outputPath = outputPath;
	dependency->outputGuid = outputGuid;
	dependency->pipelineHash = pipelineHash;
	dependency->flags = PdfBuild;
	dependency->reason = PbrSynthesized;

	{

		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_workSetLock);
		m_workSet.push_back(std::make_pair(
			dependency,
			buildParams
		));
	}

#endif

	return true;
}

Ref< ISerializable > PipelineBuilder::getBuildProduct(const ISerializable* sourceAsset)
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
		}
	}

	return 0;
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
		db::CifDefault,
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
	{
		log::error << L"Unable to create output instance" << Endl;
		return 0;
	}
}

Ref< const ISerializable > PipelineBuilder::getObjectReadOnly(const Guid& instanceGuid)
{
	return m_instanceCache->getObjectReadOnly(instanceGuid);
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
	return m_pipelineDb->createReport(name, guid);
}

IPipelineBuilder::BuildResult PipelineBuilder::performBuild(const IPipelineDependencySet* dependencySet, const PipelineDependency* dependency, const Object* buildParams, uint32_t reason)
{
	IPipelineDb::DependencyHash currentDependencyHash;

	// Ensure FP is in known state.
#if defined(_WIN32)
	uint32_t dummy;
	_controlfp_s(&dummy, 0, 0);
	_controlfp_s(&dummy,_PC_24, _MCW_PC);
	_controlfp_s(&dummy,_RC_NEAR, _MCW_RC);
#endif

	if (!dependency->pipelineType)
		return BrFailed;

	uint32_t pipelineHash = 0;
	uint32_t sourceAssetHash = 0;
	uint32_t sourceDataHash = 0;
	uint32_t filesHash = 0;

	calculateGlobalHash(
		dependencySet,
		dependency,
		pipelineHash,
		sourceAssetHash,
		sourceDataHash,
		filesHash
	);

	// Create hash entry.
	currentDependencyHash.pipelineVersion = dependency->pipelineType->getVersion();
	currentDependencyHash.hash = pipelineHash + sourceAssetHash + sourceDataHash + filesHash;

	// Skip no-build asset; just update hash.
	if ((dependency->flags & PdfBuild) == 0)
	{
		m_pipelineDb->setDependency(dependency->outputGuid, currentDependencyHash);
		return BrSucceeded;
	}

	T_ANONYMOUS_VAR(ScopeIndent)(log::info);

	log::info << L"Building asset \"" << dependency->outputPath << L"\" (" << dependency->pipelineType->getName() << L")..." << Endl;
	log::info << IncreaseIndent;

	// Get output instances from cache.
	if (m_cache && !buildParams)
	{
		if (getInstancesFromCache(dependency->outputGuid, currentDependencyHash.hash, currentDependencyHash.pipelineVersion))
		{
			log::info << L"Cached instance(s) used" << Endl;
			m_pipelineDb->setDependency(dependency->outputGuid, currentDependencyHash);
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

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*dependency->pipelineType);
	T_ASSERT (pipeline);

	bool result = pipeline->buildOutput(
		this,
		dependencySet,
		dependency,
		dependency->sourceInstanceGuid.isNotNull() ? m_sourceDatabase->getInstance(dependency->sourceInstanceGuid) : 0,
		dependency->sourceAsset,
		dependency->sourceAssetHash,
		dependency->outputPath,
		dependency->outputGuid,
		buildParams,
		reason
	);
	if (result)
		Atomic::increment(m_succeededBuilt);

	double buildTime = timer.getElapsedTime();

	log::info.setLocalTarget(infoTarget.getTarget());
	log::warning.setLocalTarget(warningTarget.getTarget());
	log::error.setLocalTarget(errorTarget.getTarget());

	if (result)
	{
		if (!builtInstances.empty())
		{
			log::info << L"Instance(s) built:" << Endl;
			log::info << IncreaseIndent;

			for (RefArray< db::Instance >::const_iterator j = builtInstances.begin(); j != builtInstances.end(); ++j)
				log::info << L"\"" << (*j)->getPath() << L"\" " << (*j)->getGuid().format() << Endl;

			if (m_cache && !buildParams)
				putInstancesInCache(
					dependency->outputGuid,
					currentDependencyHash.hash,
					currentDependencyHash.pipelineVersion,
					builtInstances
				);

			log::info << DecreaseIndent;
		}

		m_pipelineDb->setDependency(dependency->outputGuid, currentDependencyHash);
	}

	log::info << DecreaseIndent;
	log::info << (result ? L"Build successful" : L"Build failed") << Endl;

	if (result)
		return (warningTarget.getCount() + errorTarget.getCount()) > 0 ? BrSucceededWithWarnings : BrSucceeded;
	else
		return BrFailed;
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

void PipelineBuilder::buildThread(
	const IPipelineDependencySet* dependencySet,
	Thread* controlThread,
	int32_t cpuCore
)
{
	while (!controlThread->stopped())
	{
		uint32_t workDependencyIndex;
		Ref< const Object > workBuildParams;

		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_workSetLock);
			if (!m_workSet.empty())
			{
				workDependencyIndex = m_workSet.back().first;
				workBuildParams = m_workSet.back().second;
				m_workSet.pop_back();
			}
			else
				break;
		}

		const PipelineDependency* workDependency = dependencySet->get(workDependencyIndex);
		T_ASSERT (workDependency);

		if (m_listener)
			m_listener->beginBuild(
				cpuCore,
				m_progress,
				m_progressEnd,
				workDependency
			);

		BuildResult result = performBuild(dependencySet, workDependency, workBuildParams, m_reasons[workDependencyIndex]);
		if (result == BrSucceeded || result == BrSucceededWithWarnings)
			++m_succeeded;
		else
			++m_failed;

		if (m_listener)
			m_listener->endBuild(
				cpuCore,
				m_progress,
				m_progressEnd,
				workDependency,
				result
			);

		if (!workBuildParams)
			Atomic::increment(m_progress);
	}
}

	}
}
