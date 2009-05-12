#include <limits>
#include <algorithm>
#include "Editor/PipelineManager.h"
#include "Editor/PipelineHash.h"
#include "Editor/Pipeline.h"
#include "Editor/Asset.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Io/FileSystem.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Misc/Save.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineManager", PipelineManager, Object)

PipelineManager::PipelineManager(
	db::Database* sourceDatabase,
	db::Database* outputDatabase,
	const RefArray< Pipeline >& pipelines,
	PipelineHash* hash,
	Listener* listener
)
:	m_sourceDatabase(sourceDatabase)
,	m_outputDatabase(outputDatabase)
,	m_pipelines(pipelines)
,	m_hash(hash)
,	m_listener(listener)
{
}

Pipeline* PipelineManager::findPipeline(const Type& sourceType) const
{
	uint32_t best = std::numeric_limits< uint32_t >::max();
	Pipeline* pipeline = 0;

	for (RefArray< Pipeline >::const_iterator i = m_pipelines.begin(); i != m_pipelines.end(); ++i)
	{
		TypeSet typeSet = (*i)->getAssetTypes();
		for (TypeSet::iterator j = typeSet.begin(); j != typeSet.end(); ++j)
		{
			uint32_t distance = 0;

			// Calculate distance in type hierarchy.
			const Type* type = &sourceType;
			while (type)
			{
				if (type == *j)
					break;

				++distance;
				type = type->getSuper();
			}

			// Keep closest matching type.
			if (type && distance < best)
			{
				pipeline = *i;
				if ((best = distance) == 0)
					break;
			}
		}
	}

	return pipeline;
}

void PipelineManager::addDependency(const Object* sourceAsset)
{
    if (!sourceAsset)
		return;
   
	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

    Ref< Pipeline > pipeline = findPipeline(sourceAsset->getType());
    if (pipeline)
	{
		Ref< const Object > dummyBuildParams;
		pipeline->buildDependencies(this, sourceAsset, dummyBuildParams);
		T_ASSERT_M (!dummyBuildParams, L"Build parameters not used with non-producing dependencies");
	}
	else
		log::error << L"Unable to add dependency to source asset (" << type_name(sourceAsset) << L"); no pipeline found" << Endl;
}

void PipelineManager::addDependency(const Object* sourceAsset, const std::wstring& name, const std::wstring& outputPath, const Guid& outputGuid, bool build)
{
	if (!sourceAsset)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Don't add dependency multiple times.
	if (Dependency* dependency = findDependency(outputGuid))
	{
		// Still need to add to current dependency so one asset can be dependent upon from several others.
		if (m_currentDependency)
			m_currentDependency->dependencies.push_back(dependency);
		return;
	}

	addUniqueDependency(
		sourceAsset,
		name,
		outputPath,
		outputGuid,
		build
	);
}

void PipelineManager::addDependency(db::Instance* sourceAssetInstance, bool build)
{
    if (!sourceAssetInstance)
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Don't add dependency multiple times.
	if (Dependency* dependency = findDependency(sourceAssetInstance->getGuid()))
	{
		// Still need to add to current dependency so one asset can be dependent upon from several others.
		if (m_currentDependency)
			m_currentDependency->dependencies.push_back(dependency);
		return;
	}

	// Checkout source asset instance.
	Ref< Object > sourceAsset = sourceAssetInstance->checkout(db::CfReadOnly);
	if (!sourceAsset)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetInstance->getName() << L"\"; failed to checkout instance" << Endl;
		return;
	}

	addUniqueDependency(
		sourceAsset,
		sourceAssetInstance->getName(),
		sourceAssetInstance->getPath(),
		sourceAssetInstance->getGuid(),
		build
	);
}

void PipelineManager::addDependency(const Guid& sourceAssetGuid, bool build)
{
	if (sourceAssetGuid.isNull() || !sourceAssetGuid.isValid())
		return;

	// Don't add dependency if thread is about to be stopped.
	if (ThreadManager::getInstance().getCurrentThread()->stopped())
		return;

	// Don't add dependency multiple times.
	if (Dependency* dependency = findDependency(sourceAssetGuid))
	{
		// Still need to add to current dependency so one asset can be dependent upon from several others.
		if (m_currentDependency)
			m_currentDependency->dependencies.push_back(dependency);
		return;
	}

	// Get source asset instance from database.
	Ref< db::Instance > sourceAssetInstance = m_sourceDatabase->getInstance(sourceAssetGuid);
	if (!sourceAssetInstance)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetGuid.format() << L"\"; no such instance" << Endl;
		return;
	}

	// Checkout source asset instance.
	Ref< Object > sourceAsset = sourceAssetInstance->checkout(db::CfReadOnly);
	if (!sourceAsset)
	{
		log::error << L"Unable to add dependency to \"" << sourceAssetInstance->getName() << L"\"; failed to checkout instance" << Endl;
		return;
	}

	addUniqueDependency(
		sourceAsset,
		sourceAssetInstance->getName(),
		sourceAssetInstance->getPath(),
		sourceAssetInstance->getGuid(),
		build
	);
}

bool PipelineManager::build(bool rebuild)
{
	//MD5 lastBuildHash;
	//uint32_t pipelineVersion;
	PipelineHash::Hash hash;
	uint32_t succeeded = 0;
	uint32_t failed = 0;

	// Check which dependencies are dirty; ie. need to be rebuilt.
	for (RefArray< Dependency >::iterator i = m_dependencies.begin(); i != m_dependencies.end(); ++i)
	{
		(*i)->checksum = DeepHash(checked_type_cast< const Serializable* >((*i)->sourceAsset)).getMD5();

		// Have source asset been modified?
		if (!rebuild)
		{
			if (!m_hash->get((*i)->outputGuid, hash))
			{
				log::info << L"Asset \"" << (*i)->name << L"\" modified; not hashed" << Endl;
				(*i)->reason |= Pipeline::BrSourceModified;
			}
			else if (hash.checksum != (*i)->checksum)
			{
				log::info << L"Asset \"" << (*i)->name << L"\" modified; source has been modified" << Endl;
				(*i)->reason |= Pipeline::BrSourceModified;
			}
			else if (hash.pipelineVersion != (*i)->pipeline->getVersion())
			{
				log::info << L"Asset \"" << (*i)->name << L"\" modified; pipeline version differ" << Endl;
				(*i)->reason |= Pipeline::BrSourceModified;
			}
			else
			{
				// Not modified, check if source file has been modified.
				const Asset* asset = dynamic_type_cast< const Asset* >((*i)->sourceAsset);
				if (asset)
				{
					Ref< File > sourceFile = FileSystem::getInstance().get(asset->getFileName());
					if (sourceFile && sourceFile->getLastWriteTime() != hash.assetTimestamp)
					{
						// Time stamps doesn't match; assume it has been modified.
						log::info << L"Asset \"" << (*i)->name << L"\" modified; data has been modified" << Endl;
						(*i)->reason |= Pipeline::BrSourceModified;
					}
				}
			}
		}
		else
			(*i)->reason |= Pipeline::BrForced;
	}

	// Build assets which are dirty or have dirty dependency assets.
	for (RefArray< Dependency >::iterator i = m_dependencies.begin(); i != m_dependencies.end(); ++i)
	{
		// Abort if current thread has been stopped; thread are stopped by worker dialog.
		if (ThreadManager::getInstance().getCurrentThread()->stopped())
			break;

		// Update hash entry; don't write it yet though.
		hash.checksum = (*i)->checksum;
		hash.pipelineVersion = (*i)->pipeline->getVersion();

		const Asset* asset = dynamic_type_cast< const Asset* >((*i)->sourceAsset);
		if (asset)
		{
			Ref< File > sourceFile = FileSystem::getInstance().get(asset->getFileName());
			if (sourceFile)
				hash.assetTimestamp = sourceFile->getLastWriteTime();
			else
				log::warning << L"Unable to read timestamp of asset " << (*i)->name << Endl;
		}

		// Skip building asset; just update hash.
		if (!(*i)->build)
		{
			m_hash->set((*i)->outputGuid, hash);
			continue;
		}

		// Check if we need to build asset; check the entire dependency chain (will update reason if dependency dirty).
		if ((*i)->needBuild())
		{
			ScopeIndent scopeIndent(log::info);

			log::info << L"Building asset \"" << (*i)->name << L"\" (" << type_name((*i)->pipeline) << L")..." << Endl;
			log::info << IncreaseIndent;

			// Notify listener about we're beginning to build the asset.
			if (m_listener)
				m_listener->begunBuildingAsset(
					(*i)->name,
					uint32_t(std::distance(m_dependencies.begin(), i)),
					uint32_t(m_dependencies.size())
				);

			// Create a clone of the source asset; we need to have a non-const copy to add as database instances.
			Ref< Serializable > sourceAsset = DeepClone(checked_type_cast< const Serializable* >((*i)->sourceAsset)).create();
			T_ASSERT (sourceAsset);

			bool result = (*i)->pipeline->buildOutput(
				this,
				sourceAsset,
				(*i)->buildParams,
				(*i)->outputPath,
				(*i)->outputGuid,
				(*i)->reason
			);

			if (result)
			{
				m_hash->set((*i)->outputGuid, hash);
				succeeded++;
			}
			else
				failed++;

			log::info << DecreaseIndent;
			log::info << (result ? L"Build successful" : L"Build failed") << Endl;
		}
	}

	if (!ThreadManager::getInstance().getCurrentThread()->stopped())
		log::info << L"Build finished; " << succeeded << L" succeeded, " << failed << L" failed" << Endl;
	else
		log::info << L"Build finished; aborted" << Endl;

	m_dependencies.resize(0);
	return true;
}

db::Database* PipelineManager::getSourceDatabase() const
{
	return m_sourceDatabase;
}

db::Database* PipelineManager::getOutputDatabase() const
{
	return m_outputDatabase;
}

db::Instance* PipelineManager::createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid, Object* object)
{
	if (!is_a< Serializable >(object))
	{
		log::error << L"Unable to create output instance; object not serializable" << Endl;
		return 0;
	}

	return m_outputDatabase->createInstance(
		instancePath,
		static_cast< Serializable* >(object),
		db::CifReplaceExisting,
		&instanceGuid
	);
}

const Serializable* PipelineManager::getObjectReadOnly(const Guid& instanceGuid)
{
	Ref< Serializable > object;

	std::map< Guid, Ref< Serializable > >::iterator i = m_readCache.find(instanceGuid);
	if (i != m_readCache.end())
		object = i->second;
	else
	{
		object = m_sourceDatabase->getObjectReadOnly(instanceGuid);
		m_readCache[instanceGuid] = object;
	}

	return object;
}

PipelineManager::Dependency* PipelineManager::findDependency(const Guid& guid) const
{
	for (RefArray< Dependency >::const_iterator i = m_dependencies.begin(); i != m_dependencies.end(); ++i)
	{
		if ((*i)->outputGuid == guid)
			return *i;
	}
	return 0;
}

void PipelineManager::addUniqueDependency(const Object* sourceAsset, const std::wstring& name, const std::wstring& outputPath, const Guid& outputGuid, bool build)
{
	// Find appropriate pipeline.
    Ref< Pipeline > pipeline = findPipeline(sourceAsset->getType());
    if (!pipeline)
	{
		log::error << L"Unable to add dependency to \"" << name << L"\"; no pipeline found" << Endl;
		return;
	}

	// Register dependency, add to "parent" dependency as well.
	Ref< Dependency > dependency = gc_new< Dependency >();
	dependency->name = name;
	dependency->pipeline = pipeline;
	dependency->sourceAsset = sourceAsset;
	dependency->outputPath = outputPath;
	dependency->outputGuid = outputGuid;
	dependency->build = build;
	dependency->reason = Pipeline::BrNone;

	if (m_currentDependency)
		m_currentDependency->dependencies.push_back(dependency);

	bool result;
	{
		Save< Ref< Dependency > > save(m_currentDependency, dependency);
		result = pipeline->buildDependencies(
			this,
			sourceAsset,
			dependency->buildParams
		);
	}

	if (result)
		m_dependencies.push_back(dependency);
}

bool PipelineManager::Dependency::needBuild()
{
	if (reason != Pipeline::BrNone)
		return true;

	for (RefArray< Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if ((*i)->needBuild())
		{
			reason |= Pipeline::BrDependencyModified;
			return true;
		}
	}

	return false;
}

	}
}
