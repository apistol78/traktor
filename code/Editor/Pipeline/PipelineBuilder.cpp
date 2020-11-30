#if defined(_WIN32)
#	include <cfloat>
#endif
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
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
#include "Editor/IPipelineInstanceCache.h"
#include "Editor/PipelineDependency.h"
#include "Editor/PipelineDependencySet.h"
#include "Editor/Pipeline/PipelineBuilder.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

const Guid c_synthesisSeedGuid(L"{59CFF56A-2A77-4218-AA06-69019B52B9B1}");

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

	for (SmallSet< uint32_t >::const_iterator i = dependency->children.begin(); i != dependency->children.end(); ++i)
	{
		const PipelineDependency* childDependency = dependencySet->get(*i);
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
,	m_progress(0)
,	m_progressEnd(0)
,	m_succeeded(0)
,	m_succeededBuilt(0)
,	m_failed(0)
,	m_cacheHit(0)
,	m_cacheMiss(0)
,	m_cacheVoid(0)
{
}

bool PipelineBuilder::build(const PipelineDependencySet* dependencySet, bool rebuild)
{
	T_ANONYMOUS_VAR(ScopeIndent)(log::info);
	T_ANONYMOUS_VAR(ScopeIndent)(log::warning);
	T_ANONYMOUS_VAR(ScopeIndent)(log::error);
	T_ANONYMOUS_VAR(ScopeIndent)(log::debug);

	Timer timer;
	timer.start();

	// Ensure FP is in known state.
#if defined(_WIN32) && !defined(_WIN64)
	uint32_t dummy;
	_controlfp_s(&dummy, 0, 0);
	_controlfp_s(&dummy,_PC_24, _MCW_PC);
	_controlfp_s(&dummy,_RC_NEAR, _MCW_RC);
#endif

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
#if defined(_DEBUG)
				log::info << L"Asset \"" << dependency->outputPath << L"\" modified; not hashed." << Endl;
#endif
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
#if defined(_DEBUG)
				log::info << L"Asset \"" << dependency->outputPath << L"\" modified; source has been modified (or new pipeline version)." << Endl;
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

	if (!rebuild && (modifiedCount > 0 || m_verbose))
		log::info << modifiedCount << L" modified instance(s)." << Endl;

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
			WorkEntry we;
			we.dependency = dependency;
			we.buildParams = nullptr;
			we.reason = reasons[i];
			m_workSet.push_back(we);
		}
	}

	T_DEBUG(L"Pipeline build; analyzed build reasons in " << int32_t(timer.getDeltaTime() * 1000) << L" ms");

	if (m_verbose && !m_workSet.empty())
		log::info << L"Dispatching " << (int32_t)m_workSet.size() << L" build(s)..." << Endl;

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
					makeFunctor
					<
						PipelineBuilder,
						const PipelineDependencySet*,
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
	if (m_cache)
		log::info << L"Pipeline cache; " << m_cacheHit << L" hit(s), " << m_cacheMiss << L" miss(es), " << m_cacheVoid << L" uncachable(s)." << Endl;

	// Log results.
	if (!ThreadManager::getInstance().getCurrentThread()->stopped())
	{
		log::info << L"Build finished in " << (int32_t)(timer.getElapsedTime() * 1000) << L" ms; " << m_succeeded << L" succeeded (" << m_succeededBuilt << L" built), " << m_failed << L" failed." << Endl;
		if (m_verbose)
		{
			for (auto duration : m_buildDurations)
				log::info << (int32_t)(duration.second * 1000) << L" ms in " << duration.first->getName() << Endl;
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

	Ref< ISerializable > product = pipeline->buildOutput(this, sourceInstance, sourceAsset, buildParams);
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

	PipelineDependency* dependency = dependencySet.get(index);
	T_ASSERT(dependency != nullptr);

	// Append hash of build params.
	if (auto hashableBuildParams = dynamic_type_cast< const ISerializable* >(buildParams))
		dependency->sourceDataHash += DeepHash(hashableBuildParams).get();

	const bool rebuild = false;

	uint32_t dependencyCount = dependencySet.size();
	uint32_t modifiedCount = 0;

	AlignedVector< uint32_t > reasons(dependencyCount, 0);
	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const PipelineDependency* dependency = dependencySet.get(i);
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
				&dependencySet,
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
				reasons[i] |= PbrSourceModified;
				++modifiedCount;
			}
		}
		else
			reasons[i] |= PbrForced;
	}

	std::list< WorkEntry > workSet;
	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		const PipelineDependency* dependency = dependencySet.get(i);
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

			const PipelineDependency* childDependency = dependencySet.get(children.back());
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
			WorkEntry we;
			we.dependency = dependency;
			we.buildParams = nullptr;
			we.reason = reasons[i];

			if (dependency->outputGuid == outputGuid)
				we.buildParams = buildParams;

			workSet.push_back(we);
		}
	}

	for (const auto& we : workSet)
		performBuild(&dependencySet, we.dependency, we.buildParams, we.reason | PbrSynthesized);

	return true;
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
		db::CifDefault,
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

Ref< db::Database > PipelineBuilder::getOutputDatabase() const
{
	return m_outputDatabase;
}

Ref< db::Database > PipelineBuilder::getSourceDatabase() const
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

IPipelineBuilder::BuildResult PipelineBuilder::performBuild(const PipelineDependencySet* dependencySet, const PipelineDependency* dependency, const Object* buildParams, uint32_t reason)
{
	// Ensure FP is in known state.
#if defined(_WIN32) && !defined(_WIN64)
	uint32_t dummy;
	_controlfp_s(&dummy, 0, 0);
	_controlfp_s(&dummy,_PC_24, _MCW_PC);
	_controlfp_s(&dummy,_RC_NEAR, _MCW_RC);
#endif

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
				log::info << L"Cached output used for \"" << dependency->outputPath << L"\"; " << (uint32_t)builtInstances.size() << L" instance(s)." << Endl;

			m_pipelineDb->setDependency(dependency->outputGuid, currentDependencyHash);
			Atomic::increment(m_cacheHit);
			Atomic::increment(m_succeededBuilt);

			// Restore previous set but also insert built instances from synthesized build;
			// when caching is enabled then synthesized built instances should be included in parent build as well.
			if (previousBuiltInstances)
				previousBuiltInstances->insert(previousBuiltInstances->end(), builtInstances.begin(), builtInstances.end());
			m_buildInstances.set(previousBuiltInstances);
			return BrSucceeded;
		}
		else
			Atomic::increment(m_cacheMiss);
	}
	else if (m_cache)
		Atomic::increment(m_cacheVoid);

	// Expose this dependency's hash since synthesized builds need to take it into account for caching.
	m_parentHash.set(&currentDependencyHash);

	LogTargetFilter infoTarget(log::info.getLocalTarget(), !m_verbose);
	LogTargetFilter warningTarget(log::warning.getLocalTarget(), false);
	LogTargetFilter errorTarget(log::error.getLocalTarget(), false);

	log::info.setLocalTarget(&infoTarget);
	log::warning.setLocalTarget(&warningTarget);
	log::error.setLocalTarget(&errorTarget);

	Timer timer;
	timer.start();

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*dependency->pipelineType);
	T_ASSERT(pipeline);

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

	if (result)
		Atomic::increment(m_succeededBuilt);

	double buildTime = timer.getElapsedTime();

	log::info.setLocalTarget(infoTarget.getTarget());
	log::warning.setLocalTarget(warningTarget.getTarget());
	log::error.setLocalTarget(errorTarget.getTarget());

	if (result)
	{
		if (m_cache)
			putInstancesInCache(
				dependency->outputGuid,
				currentDependencyHash,
				builtInstances
			);

		if (m_verbose && !builtInstances.empty())
		{
			log::info << L"Instance(s) built:" << Endl;
			log::info << IncreaseIndent;

			for (auto builtInstance : builtInstances)
				log::info << L"\"" << builtInstance->getPath() << L"\" " << builtInstance->getGuid().format() << Endl;

			log::info << DecreaseIndent;
		}

		m_pipelineDb->setDependency(dependency->outputGuid, currentDependencyHash);

		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_buildDurationsLock);
			m_buildDurations[&type_of(pipeline)] += buildTime;
		}
	}

	log::info << DecreaseIndent;

	if (m_verbose)
	{
		if (result)
			log::info << L"Build \"" << dependency->outputPath << L"\" successful." << Endl;
		else
			log::info << L"Build \"" << dependency->outputPath << L"\" failed." << Endl;
	}

	// Restore previous set but also insert built instances from synthesized build;
	// when caching is enabled then synthesized built instances should be included in parent build as well.
	if (previousBuiltInstances)
		previousBuiltInstances->insert(previousBuiltInstances->end(), builtInstances.begin(), builtInstances.end());
	m_buildInstances.set(previousBuiltInstances);

	if (result)
		return (warningTarget.getCount() + errorTarget.getCount()) > 0 ? BrSucceededWithWarnings : BrSucceeded;
	else
		return BrFailed;
}

bool PipelineBuilder::putInstancesInCache(const Guid& guid, const PipelineDependencyHash& hash, const RefArray< db::Instance >& instances)
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

bool PipelineBuilder::getInstancesFromCache(const PipelineDependency* dependency, const PipelineDependencyHash& hash, RefArray< db::Instance >& outInstances)
{
	bool result = false;

	Ref< IStream > stream = m_cache->get(dependency->outputGuid, hash);
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

			Ref< db::Instance > instance = db::Isolate::createInstanceFromIsolation(group, stream);
			if (instance)
				outInstances.push_back(instance);
			else
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
	const PipelineDependencySet* dependencySet,
	Thread* controlThread,
	int32_t cpuCore
)
{
	while (!controlThread->stopped())
	{
		WorkEntry we = { nullptr, nullptr, 0 };

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

		T_ASSERT(we.dependency);

		if (m_listener)
			m_listener->beginBuild(
				m_progress,
				m_progressEnd,
				we.dependency
			);

		BuildResult result = performBuild(dependencySet, we.dependency, we.buildParams, we.reason);
		if (result == BrSucceeded || result == BrSucceededWithWarnings)
			Atomic::increment(m_succeeded);
		else
			Atomic::increment(m_failed);

		if (m_listener)
			m_listener->endBuild(
				m_progress,
				m_progressEnd,
				we.dependency,
				result
			);

		if (!we.buildParams)
			Atomic::increment(m_progress);
	}
}

	}
}
