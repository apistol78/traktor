#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
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
#include "Editor/DataAccessCache.h"
#include "Editor/IPipeline.h"
#include "Editor/IPipelineCache.h"
#include "Editor/IPipelineDb.h"
#include "Editor/IPipelineInstanceCache.h"
#include "Editor/PipelineDependency.h"
#include "Editor/PipelineDependencySet.h"
#include "Editor/Pipeline/PipelineBuilder.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Editor/Pipeline/PipelineProfiler.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

class LogTargetFilter : public ILogTarget
{
public:
	LogTargetFilter(ILogTarget* target, bool muted)
	:	m_target(target)
	,	m_muted(muted)
	,	m_count(0)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		++m_count;
		if (m_target && !m_muted)
			m_target->log(threadId, level, str);
	}

	ILogTarget* getTarget() const { return m_target; }

	uint32_t getCount() const { return m_count; }

private:
	Ref< ILogTarget > m_target;
	bool m_muted;
	uint32_t m_count;
};

void calculateGlobalHash(
	const PipelineDependencySet* dependencySet,
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

	for (auto child : dependency->children)
	{
		const PipelineDependency* childDependency = dependencySet->get(child);
		T_ASSERT(childDependency);

		if (childDependency == dependency)
			continue;

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
	bool threadedBuildEnable,
	bool verbose
)
:	m_pipelineFactory(pipelineFactory)
,	m_sourceDatabase(sourceDatabase)
,	m_outputDatabase(outputDatabase)
,	m_cache(cache)
,	m_pipelineDb(pipelineDb)
,	m_instanceCache(instanceCache)
,	m_listener(listener)
,	m_threadedBuildEnable(threadedBuildEnable)
,	m_verbose(verbose)
,	m_rebuild(false)
,	m_profiler(new PipelineProfiler())
,	m_progress(0)
,	m_progressEnd(0)
,	m_succeeded(0)
,	m_succeededBuilt(0)
,	m_failed(0)
,	m_cacheHit(0)
,	m_cacheMiss(0)
,	m_cacheVoid(0)
{
	// Create data access memento cache.
	m_dataAccessCache = new DataAccessCache(cache);
}

bool PipelineBuilder::build(const PipelineDependencySet* dependencySet, bool rebuild)
{
	T_ANONYMOUS_VAR(ScopeIndent)(log::info);
	T_ANONYMOUS_VAR(ScopeIndent)(log::warning);
	T_ANONYMOUS_VAR(ScopeIndent)(log::error);
	T_ANONYMOUS_VAR(ScopeIndent)(log::debug);

	Timer timer;

	uint32_t dependencyCount = dependencySet->size();
	uint32_t modifiedCount = 0;

	if (m_verbose && !rebuild)
		log::info << L"Analyzing conditions of " << dependencyCount << L" build item(s)..." << Endl;

	// Determine build reasons.
	AlignedVector< uint32_t > reasons(dependencyCount, 0);
	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const PipelineDependency* dependency = dependencySet->get(i);
		T_ASSERT(dependency);

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

			// Get hash entry from database.
			PipelineDependencyHash previousDependencyHash;
			if (!m_pipelineDb->getDependency(dependency->outputGuid, previousDependencyHash))
			{
				if (m_verbose)
					log::info << L"Asset \"" << dependency->outputPath << L"\" modified; not hashed." << Endl;
				reasons[i] |= PbrSourceModified;
				++modifiedCount;
			}
			else if (
				previousDependencyHash.pipelineHash != pipelineHash ||
				previousDependencyHash.sourceAssetHash != sourceAssetHash ||
				previousDependencyHash.sourceDataHash != sourceDataHash ||
				previousDependencyHash.filesHash != filesHash
			)
			{
				if (m_verbose)
				{
					log::info << L"Asset \"" << dependency->outputPath << L"\" modified; source has been modified (";

					bool prepend = false;
					if (previousDependencyHash.pipelineHash != pipelineHash)
					{
						log::info << L"pipeline";
						prepend = true;
					}
					if (previousDependencyHash.sourceAssetHash != sourceAssetHash)
					{
						if (prepend)
							log::info << L"-, ";
						log::info << L"source asset";
						prepend = true;
					}
					if (previousDependencyHash.sourceDataHash != sourceDataHash)
					{
						if (prepend)
							log::info << L"-, ";
						log::info << L"source data";
						prepend = true;
					}
					if (previousDependencyHash.filesHash != filesHash)
					{
						if (prepend)
							log::info << L"-, ";
						log::info << L"file";
						prepend = true;
					}

					log::info << L" hash mismatch)." << Endl;
				}

#if defined(_DEBUG)
				log::info << IncreaseIndent;
				log::info << L"Pipeline hash "; FormatHex(log::info, pipelineHash, 8); log::info << L" ("; FormatHex(log::info, previousDependencyHash.pipelineHash, 8); log::info << L")" << Endl;
				log::info << L"Source asset hash "; FormatHex(log::info, sourceAssetHash, 8); log::info << L" ("; FormatHex(log::info, previousDependencyHash.sourceAssetHash, 8); log::info << L")" << Endl;
				log::info << L"Source data hash "; FormatHex(log::info, sourceDataHash, 8); log::info << L" ("; FormatHex(log::info, previousDependencyHash.sourceDataHash, 8); log::info << L")" << Endl;
				log::info << L"File(s) hash "; FormatHex(log::info, filesHash, 8); log::info << L" ("; FormatHex(log::info, previousDependencyHash.filesHash, 8); log::info << L")" << Endl;
				log::info << L"---" << Endl;
				dependency->dump(log::info);
				log::info << DecreaseIndent;
#endif
				reasons[i] |= PbrSourceModified;
				++modifiedCount;
			}
		}
		else
			reasons[i] |= PbrForced;
	}

	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const PipelineDependency* dependency = dependencySet->get(i);
		T_ASSERT(dependency);

		SmallSet< uint32_t > visited;
		visited.insert(i);

		AlignedVector< uint32_t > children;
		children.insert(children.end(), dependency->children.begin(), dependency->children.end());

		while (!children.empty())
		{
			if (visited.find(children.back()) != visited.end())
			{
				children.pop_back();
				continue;
			}

			const PipelineDependency* childDependency = dependencySet->get(children.back());
			T_ASSERT(childDependency);

			if ((childDependency->flags & PdfUse) == 0)
			{
				children.pop_back();
				continue;
			}

			if ((reasons[children.back()] & PbrSourceModified) != 0)
				reasons[i] |= PbrDependencyModified;

			visited.insert(children.back());

			children.pop_back();
			children.insert(children.end(), childDependency->children.begin(), childDependency->children.end());
		}

		if (reasons[i] != 0)
		{
			auto& we = m_workSet.push_back();
			we.dependency = dependency;
			we.buildParams = nullptr;
			we.reason = reasons[i];
		}
	}

	T_DEBUG(L"Pipeline build; analyzed build reasons in " << int32_t(timer.getDeltaTime() * 1000) << L" ms");

	if (m_verbose && !m_workSet.empty())
		log::info << L"Dispatching " << (int32_t)m_workSet.size() << L" build(s)..." << Endl;

	m_rebuild = rebuild;
	m_progress = 0;
	m_progressEnd = (int32_t)m_workSet.size();
	m_succeeded = dependencyCount - m_progressEnd;
	m_succeededBuilt = 0;
	m_failed = 0;
	m_cacheHit = 0;
	m_cacheMiss = 0;
	m_cacheVoid = 0;	// No hash on source asset will result in a void.

	if (!m_workSet.empty())
	{
		int32_t cpuCores = OS::getInstance().getCPUCoreCount();
		if (m_threadedBuildEnable)
		{
			std::vector< Thread* > threads(cpuCores, (Thread*)0);
			for (int32_t i = 0; i < cpuCores; ++i)
			{
				ThreadPool::getInstance().spawn(
					[&, i]() { buildThread(dependencySet, ThreadManager::getInstance().getCurrentThread(), i); },
					threads[i]
				);
			}

			for (int32_t i = 0; i < cpuCores; ++i)
			{
				if (threads[i])
				{
					ThreadPool::getInstance().join(threads[i]);
					threads[i] = nullptr;
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
	}

	// Log cache performance.
	if (m_cache && m_verbose)
		log::info << L"Pipeline cache; " << m_cacheHit << L" hit(s), " << m_cacheMiss << L" miss(es), " << m_cacheVoid << L" uncachable(s)." << Endl;

	// Log results.
	if (!ThreadManager::getInstance().getCurrentThread()->stopped())
	{
		log::info << L"Build finished in " << formatDuration(timer.getElapsedTime()) << L"; " << m_succeeded << L" succeeded (" << m_succeededBuilt << L" built), " << m_failed << L" failed." << Endl;
		if (m_verbose)
		{
			AlignedVector< std::pair< const TypeInfo*, PipelineProfiler::Duration > > durations(m_profiler->getDurations().begin(), m_profiler->getDurations().end());
			std::sort(durations.begin(), durations.end(), [](const std::pair< const TypeInfo*, PipelineProfiler::Duration >& lh, const std::pair< const TypeInfo*, PipelineProfiler::Duration >& rh) {
				return lh.second.seconds > rh.second.seconds;
			});
			double totalDuration = 0.0;
			for (const auto& duration : durations)
				totalDuration += duration.second.seconds;
			for (const auto& duration : durations)
				log::info << formatDuration(duration.second.seconds) << L" (" << duration.second.count << L", " << str(L"%.1f", (100.0 * duration.second.seconds) / totalDuration) << L"%) in " << duration.first->getName() << Endl;
		}
	}
	else
		log::info << L"Build finished; aborted." << Endl;

	return m_failed == 0;
}

Ref< ISerializable > PipelineBuilder::buildOutput(const db::Instance* sourceInstance, const ISerializable* sourceAsset, const Object* buildParams)
{
	if (!sourceAsset)
		return nullptr;

	uint32_t sourceHash = DeepHash(sourceAsset).get();

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_builtCacheLock);

		std::map< uint32_t, built_cache_list_t >::iterator i = m_builtCache.find(sourceHash);
		if (i != m_builtCache.end())
		{
			built_cache_list_t& bcl = i->second;
			T_ASSERT(!bcl.empty());

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
		return nullptr;

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*pipelineType);
	T_ASSERT(pipeline);

	m_profiler->begin(*pipelineType);
	Ref< ISerializable > product = pipeline->buildOutput(this, sourceInstance, sourceAsset, buildParams);
	m_profiler->end(*pipelineType);
	if (!product)
		return nullptr;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_builtCacheLock);

		BuiltCacheEntry bce;
		bce.sourceAsset = sourceAsset;
		bce.product = product;
		m_builtCache[sourceHash].push_back(bce);
	}

	return product;
}

bool PipelineBuilder::buildAdHocOutput(const ISerializable* sourceAsset, const Guid& outputGuid, const Object* buildParams)
{
	std::wstring outputPath = L"Generated/" + outputGuid.format();
	return buildAdHocOutput(sourceAsset, outputPath, outputGuid, buildParams);
}

bool PipelineBuilder::buildAdHocOutput(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, const Object* buildParams)
{
	const bool cacheable = (bool)(buildParams == nullptr || is_a< ISerializable >(buildParams));

	// Scan dependencies of source asset.
	PipelineDependencySet dependencySet;
	if (m_threadedBuildEnable)
	{
		PipelineDependsParallel pipelineDepends(m_pipelineFactory, m_sourceDatabase, m_outputDatabase, &dependencySet, m_pipelineDb, m_instanceCache);
		pipelineDepends.addDependency(
			sourceAsset,
			outputPath,
			outputGuid,
			PdfBuild
		);
		pipelineDepends.waitUntilFinished();
	}
	else
	{
		PipelineDependsIncremental pipelineDepends(m_pipelineFactory, m_sourceDatabase, m_outputDatabase, &dependencySet, m_pipelineDb, m_instanceCache);
		pipelineDepends.addDependency(
			sourceAsset,
			outputPath,
			outputGuid,
			PdfBuild
		);
		pipelineDepends.waitUntilFinished();
	}

	uint32_t index = dependencySet.get(outputGuid);
	if (index == PipelineDependencySet::DiInvalid)
		return false;

	const PipelineDependency* dependency = dependencySet.get(index);
	T_ASSERT(dependency != nullptr);

	if (m_listener)
		m_listener->beginBuild(
			m_progress,
			m_progressEnd,
			dependency
		);

	// Calculate hash entry.
	PipelineDependencyHash currentDependencyHash;
	calculateGlobalHash(
		&dependencySet,
		dependency,
		currentDependencyHash.pipelineHash,
		currentDependencyHash.sourceAssetHash,
		currentDependencyHash.sourceDataHash,
		currentDependencyHash.filesHash
	);

	// Add hash of build params to source data hash.
	if (cacheable)
		currentDependencyHash.sourceDataHash += DeepHash(static_cast< const ISerializable* >(buildParams)).get();

	// Check database if this build already exist in output.
	if (!m_rebuild && cacheable)
	{
		PipelineDependencyHash previousDependencyHash;
		if (m_pipelineDb->getDependency(dependency->outputGuid, previousDependencyHash))
		{
			if (currentDependencyHash == previousDependencyHash)
				return true;
		}
	}

	T_ANONYMOUS_VAR(ScopeIndent)(log::info);

	if (m_verbose)
		log::info << L"Building asset \"" << dependency->outputPath << L"\" (" << type_name(sourceAsset) << L")..." << Endl;

	log::info << IncreaseIndent;

	// Build output instances; keep an array of written instances as we
	// need them to update the cache.
	RefArray< db::Instance >* previousBuiltInstances = reinterpret_cast< RefArray< db::Instance >* >(m_buildInstances.get());
	RefArray< db::Instance > builtInstances;
	m_buildInstances.set(&builtInstances);

	// Get output instances from cache.
	if (m_cache && cacheable)
	{
		if (getInstancesFromCache(dependency, currentDependencyHash, builtInstances))
		{
			if (m_verbose)
				log::info << L"Cached output used for \"" << dependency->outputPath << L"\"; " << (uint32_t)builtInstances.size() << L" instance(s)." << Endl;

			m_pipelineDb->setDependency(dependency->outputGuid, currentDependencyHash);

			m_cacheHit++;
			m_succeededBuilt++;

			if (m_listener)
				m_listener->endBuild(
					m_progress,
					m_progressEnd,
					dependency,
					BrSucceeded
				);

			// Restore previous set but also insert built instances from synthesized build;
			// when caching is enabled then synthesized built instances should be included in parent build as well.
			if (previousBuiltInstances)
				previousBuiltInstances->insert(previousBuiltInstances->end(), builtInstances.begin(), builtInstances.end());
			m_buildInstances.set(previousBuiltInstances);
			return true;
		}
		else
			m_cacheMiss++;
	}
	else if (m_cache)
		m_cacheVoid++;

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*dependency->pipelineType);
	T_ASSERT(pipeline);

	m_profiler->begin(*dependency->pipelineType);
	bool result = pipeline->buildOutput(
		this,
		nullptr,
		nullptr,
		nullptr,
		sourceAsset,
		outputPath,
		outputGuid,
		buildParams,
		PbrSourceModified
	);
	m_profiler->end(*dependency->pipelineType);

	if (
		!ThreadManager::getInstance().getCurrentThread()->stopped() &&
		result
	)
	{
		if (m_cache && cacheable)
			putInstancesInCache(
				dependency->outputGuid,
				currentDependencyHash,
				builtInstances
			);

#if defined(_DEBUG)
		if (m_verbose && !builtInstances.empty())
		{
			log::info << L"Instance(s) built:" << Endl;
			log::info << IncreaseIndent;

			for (auto builtInstance : builtInstances)
				log::info << L"\"" << builtInstance->getPath() << L"\" " << builtInstance->getGuid().format() << Endl;

			log::info << DecreaseIndent;
		}
#endif

		m_pipelineDb->setDependency(dependency->outputGuid, currentDependencyHash);
	}

	log::info << DecreaseIndent;
	if (m_verbose)
	{
		if (result)
			log::info << L"Build \"" << dependency->outputPath << L"\" successful." << Endl;
		else
			log::info << L"Build \"" << dependency->outputPath << L"\" failed." << Endl;
	}

	if (m_listener)
		m_listener->endBuild(
			m_progress,
			m_progressEnd,
			dependency,
			result ? BrSucceeded : BrFailed
		);

	// Restore previous set but also insert built instances from synthesized build;
	// when caching is enabled then synthesized built instances should be included in parent build as well.
	if (previousBuiltInstances)
		previousBuiltInstances->insert(previousBuiltInstances->end(), builtInstances.begin(), builtInstances.end());
	m_buildInstances.set(previousBuiltInstances);

	return result;
}

uint32_t PipelineBuilder::calculateInclusiveHash(const ISerializable* sourceAsset) const
{
	// Scan dependencies of source asset.
	PipelineDependencySet dependencySet;
	if (m_threadedBuildEnable)
	{
		PipelineDependsParallel pipelineDepends(m_pipelineFactory, m_sourceDatabase, m_outputDatabase, &dependencySet, m_pipelineDb, m_instanceCache);
		pipelineDepends.addDependency(sourceAsset);
		pipelineDepends.waitUntilFinished();
	}
	else
	{
		PipelineDependsIncremental pipelineDepends(m_pipelineFactory, m_sourceDatabase, m_outputDatabase, &dependencySet, m_pipelineDb, m_instanceCache);
		pipelineDepends.addDependency(sourceAsset);
		pipelineDepends.waitUntilFinished();
	}

	// Calculate hash of source.
	uint32_t hash = DeepHash(sourceAsset).get();

	// Append hashes of all dependencies.
	for (uint32_t i = 0; i < dependencySet.size(); ++i)
	{
		const PipelineDependency* dependency = dependencySet.get(i);
		T_ASSERT(dependency);

		hash += dependency->pipelineHash;
		hash += dependency->sourceAssetHash;
		hash += dependency->sourceDataHash;
		hash += dependency->filesHash;
	}

	return hash;
}

Ref< ISerializable > PipelineBuilder::getBuildProduct(const ISerializable* sourceAsset)
{
	if (!sourceAsset)
		return nullptr;

	uint32_t sourceHash = DeepHash(sourceAsset).get();

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_builtCacheLock);

		std::map< uint32_t, built_cache_list_t >::iterator i = m_builtCache.find(sourceHash);
		if (i != m_builtCache.end())
		{
			built_cache_list_t& bcl = i->second;
			T_ASSERT(!bcl.empty());

			// Return same instance as before if pointer and hash match.
			for (built_cache_list_t::const_iterator j = bcl.begin(); j != bcl.end(); ++j)
			{
				if (j->sourceAsset == sourceAsset)
					return j->product;
			}
		}
	}

	return nullptr;
}

Ref< db::Instance > PipelineBuilder::createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_createOutputLock);
	Ref< db::Instance > instance;

	if (instanceGuid.isNull() || !instanceGuid.isValid())
	{
		log::error << L"Invalid guid for output instance." << Endl;
		return nullptr;
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
			log::error << L"Unable to remove existing instance \"" << instance->getPath() << L"\"." << Endl;
			return nullptr;
		}
	}

	instance = m_outputDatabase->createInstance(
		instancePath,
		db::CifReplaceExisting,
		&instanceGuid
	);
	if (instance)
	{
		RefArray< db::Instance >* builtInstances = (RefArray< db::Instance >*)m_buildInstances.get();
		if (builtInstances)
			builtInstances->push_back(instance);
		return instance;
	}
	else
	{
		log::error << L"Unable to create output instance \"" << instancePath << L"\"." << Endl;
		return nullptr;
	}
}

db::Database* PipelineBuilder::getOutputDatabase() const
{
	return m_outputDatabase;
}

db::Database* PipelineBuilder::getSourceDatabase() const
{
	return m_sourceDatabase;
}

Ref< const ISerializable > PipelineBuilder::getObjectReadOnly(const Guid& instanceGuid)
{
	if (instanceGuid.isNotNull())
		return m_instanceCache->getObjectReadOnly(instanceGuid);
	else
		return nullptr;
}

DataAccessCache* PipelineBuilder::getDataAccessCache() const
{
	return m_dataAccessCache;
}

PipelineProfiler* PipelineBuilder::getProfiler() const
{
	return m_profiler;
}

IPipelineBuilder::BuildResult PipelineBuilder::performBuild(
	const PipelineDependencySet* dependencySet,
	const PipelineDependency* dependency,
	const Object* buildParams,
	uint32_t reason
)
{
	if (!dependency->pipelineType)
		return BrFailed;

	// Calculate recursive hash entry.
	PipelineDependencyHash currentDependencyHash;
	calculateGlobalHash(
		dependencySet,
		dependency,
		currentDependencyHash.pipelineHash,
		currentDependencyHash.sourceAssetHash,
		currentDependencyHash.sourceDataHash,
		currentDependencyHash.filesHash
	);

	// Skip no-build asset; just update hash.
	if ((dependency->flags & PdfBuild) == 0)
	{
		m_pipelineDb->setDependency(dependency->outputGuid, currentDependencyHash);
		return BrSucceeded;
	}

	T_ANONYMOUS_VAR(ScopeIndent)(log::info);

	log::info << L"Building \"" << dependency->outputPath << L"\"..." << Endl;
	log::info << IncreaseIndent;

	// Build output instances; keep an array of written instances as we
	// need them to update the cache.
	RefArray< db::Instance >* previousBuiltInstances = (RefArray< db::Instance >*)m_buildInstances.get();
	RefArray< db::Instance > builtInstances;
	m_buildInstances.set(&builtInstances);

	// Get output instances from cache.
	if (m_cache)
	{
		if (getInstancesFromCache(dependency, currentDependencyHash, builtInstances))
		{
			if (m_verbose)
			{
				log::info << L"Cached output used for \"" << dependency->outputPath << L"\"; " << (uint32_t)builtInstances.size() << L" instance(s):" << Endl;
				for (auto builtInstance : builtInstances)
					log::info << L"\t\"" << builtInstance->getPath() << L"\" " << builtInstance->getGuid().format() << Endl;
			}

			m_pipelineDb->setDependency(dependency->outputGuid, currentDependencyHash);

			m_cacheHit++;
			m_succeededBuilt++;

			m_buildInstances.set(previousBuiltInstances);
			return BrSucceeded;
		}
		else
			m_cacheMiss++;
	}
	else
		m_cacheVoid++;

	LogTargetFilter infoTarget(log::info.getLocalTarget(), !m_verbose);
	LogTargetFilter warningTarget(log::warning.getLocalTarget(), false);
	LogTargetFilter errorTarget(log::error.getLocalTarget(), false);

	log::info.setLocalTarget(&infoTarget);
	log::warning.setLocalTarget(&warningTarget);
	log::error.setLocalTarget(&errorTarget);

	Timer timer;

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*dependency->pipelineType);
	T_ASSERT(pipeline);

	m_profiler->begin(*dependency->pipelineType);
	bool result = pipeline->buildOutput(
		this,
		dependencySet,
		dependency,
		dependency->sourceInstanceGuid.isNotNull() ? m_sourceDatabase->getInstance(dependency->sourceInstanceGuid) : nullptr,
		dependency->sourceAsset,
		dependency->outputPath,
		dependency->outputGuid,
		buildParams,
		reason
	);
	m_profiler->end(*dependency->pipelineType);

	if (result)
		m_succeededBuilt++;

	double buildTime = timer.getElapsedTime();

	log::info.setLocalTarget(infoTarget.getTarget());
	log::warning.setLocalTarget(warningTarget.getTarget());
	log::error.setLocalTarget(errorTarget.getTarget());

	if (
		!ThreadManager::getInstance().getCurrentThread()->stopped() &&
		result
	)
	{
		if (m_cache)
			putInstancesInCache(
				dependency->outputGuid,
				currentDependencyHash,
				builtInstances
			);

#if defined(_DEBUG)
		if (m_verbose && !builtInstances.empty())
		{
			log::info << L"Instance(s) built:" << Endl;
			log::info << IncreaseIndent;

			for (auto builtInstance : builtInstances)
				log::info << L"\t\"" << builtInstance->getPath() << L"\" " << builtInstance->getGuid().format() << Endl;

			log::info << DecreaseIndent;
		}
#endif

		m_pipelineDb->setDependency(dependency->outputGuid, currentDependencyHash);
	}

	log::info << DecreaseIndent;

	if (m_verbose)
	{
		if (result)
			log::info << L"Build \"" << dependency->outputPath << L"\" successful." << Endl;
		else
			log::info << L"Build \"" << dependency->outputPath << L"\" failed." << Endl;
	}

	m_buildInstances.set(previousBuiltInstances);

	if (result)
		return (warningTarget.getCount() + errorTarget.getCount()) > 0 ? BrSucceededWithWarnings : BrSucceeded;
	else
		return BrFailed;
}

bool PipelineBuilder::putInstancesInCache(const Guid& guid, const PipelineDependencyHash& hash,	const RefArray< db::Instance >& instances)
{
	Ref< IStream > stream = m_cache->put(guid, hash);
	if (!stream)
		return false;

	Writer writer(stream);

	// Write directory.
	writer << (uint32_t)instances.size();
	for (uint32_t i = 0; i < (uint32_t)instances.size(); ++i)
	{
		const Guid instanceId = instances[i]->getGuid();
		const std::wstring instancePath = instances[i]->getPath();
		if (writer.write((const uint8_t*)instanceId, 16) != 16)
			return false;
		writer << instancePath;
	}

	// Write instances.
	for (uint32_t i = 0; i < (uint32_t)instances.size(); ++i)
	{
		if (!db::Isolate::createIsolatedInstance(instances[i], stream))
			return false;
	}

	stream->close();
	
	// Commit cached item.
	if (!m_cache->commit(guid, hash))
		return false;

	return true;
}

bool PipelineBuilder::getInstancesFromCache(const PipelineDependency* dependency, const PipelineDependencyHash& hash, RefArray< db::Instance >& outInstances)
{
	struct DirectoryEntry
	{
		Guid instanceId;
		std::wstring instancePath;
	};

	bool create = true;
	bool result = true;

	// Open stream to cached blob.
	Ref< IStream > stream = m_cache->get(dependency->outputGuid, hash);
	if (!stream)
		return false;

	// Compare hash to last output to determine if we need to create output instances or if
	// they should already exist in output database.
	PipelineDependencyHash lastOutputHash;
	if (m_pipelineDb->getDependency(dependency->outputGuid, lastOutputHash))
	{
		if (lastOutputHash == hash)
			create = false;
	}

	Reader reader(stream);

	// Read directory from stream.
	uint32_t instanceCount = 0;
	reader >> instanceCount;
	if (instanceCount == 0)
	{
		stream->close();
		return true;
	}

	AlignedVector< DirectoryEntry > directory(instanceCount);
	for (uint32_t i = 0; i < instanceCount; ++i)
	{
		uint8_t instanceId[16];
		if (reader.read(instanceId, 16) != 16)
			return false;
		if (!(directory[i].instanceId = Guid(instanceId)).isNotNull())
			return false;
		reader >> directory[i].instancePath;
		if (directory[i].instancePath.empty())
			return false;
	}

	// Fetch or create instances.
	if (create)
	{
		for (uint32_t i = 0; i < instanceCount; ++i)
		{
			Path instancePath(directory[i].instancePath);

			Ref< db::Group > group = m_outputDatabase->createGroup(instancePath.getPathOnlyNoVolume());
			if (!group)
			{
				result = false;
				break;
			}

			Ref< db::Instance > instance = db::Isolate::createInstanceFromIsolation(group, stream);
			if (instance)
				outInstances.push_back(instance);
			else
			{
				result = false;
				break;
			}
		}
	}
	else
	{
		for (uint32_t i = 0; i < instanceCount; ++i)
		{
			Ref< db::Instance > instance = m_outputDatabase->getInstance(directory[i].instanceId);
			if (instance)
				outInstances.push_back(instance);
			else
			{
				result = false;
				break;
			}
		}
	}

	stream->close();
	return result;
}

void PipelineBuilder::buildThread(
	const PipelineDependencySet* dependencySet,
	Thread* controlThread,
	int32_t cpuCore
)
{
	while (!controlThread->stopped())
	{
		WorkEntry we;
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_workSetLock);
			if (!m_workSet.empty())
			{
				we = m_workSet.back();
				m_workSet.pop_back();
			}
			else
				break;
		}

		if (m_listener)
			m_listener->beginBuild(
				m_progress,
				m_progressEnd,
				we.dependency
			);

		BuildResult result = performBuild(dependencySet, we.dependency, we.buildParams, we.reason);
		if (result == BrSucceeded || result == BrSucceededWithWarnings)
			m_succeeded++;
		else
			m_failed++;

		if (m_listener)
			m_listener->endBuild(
				m_progress,
				m_progressEnd,
				we.dependency,
				result
			);

		m_progress++;
	}
}

	}
}
