#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Thread/Acquire.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/AgentOpenFile.h"
#include "Editor/Pipeline/AgentStream.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/Stream/RemoteStream.h"
#include "Pipeline/Agent/App/PipelineBuilderWrapper.h"
#include "Pipeline/Agent/App/ReadOnlyObjectCache.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{
	
const uint16_t c_databasePort = 38000;
const uint16_t c_agentPort = 39000;
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineBuilderWrapper", PipelineBuilderWrapper, IPipelineBuilder)

PipelineBuilderWrapper::PipelineBuilderWrapper(
	PipelineFactory* pipelineFactory,
	net::BidirectionalObjectTransport* transport,
	const std::wstring& host,
	uint16_t streamServerPort,
	db::Database* sourceDatabase,
	db::Database* outputDatabase,
	ReadOnlyObjectCache* objectCache
)
:	m_pipelineFactory(pipelineFactory)
,	m_transport(transport)
,	m_host(host)
,	m_streamServerPort(streamServerPort)
,	m_sourceDatabase(sourceDatabase)
,	m_outputDatabase(outputDatabase)
,	m_objectCache(objectCache)
{
}

bool PipelineBuilderWrapper::build(const RefArray< PipelineDependency >& dependencies, bool rebuild)
{
	T_FATAL_ERROR;
	return false;
}

Ref< ISerializable > PipelineBuilderWrapper::buildOutput(const ISerializable* sourceAsset)
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

bool PipelineBuilderWrapper::buildOutput(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, const Object* buildParams)
{
	const TypeInfo* pipelineType;
	uint32_t pipelineHash;

	if (!m_pipelineFactory->findPipelineType(type_of(sourceAsset), pipelineType, pipelineHash))
		return 0;

	Ref< IPipeline > pipeline = m_pipelineFactory->findPipeline(*pipelineType);
	T_ASSERT (pipeline);

	if (!pipeline->buildOutput(
		this,
		0,
		0,
		sourceAsset,
		0,
		outputPath,
		outputGuid,
		buildParams,
		PbrSynthesized
	))
		return false;

	return true;
}

Ref< db::Database > PipelineBuilderWrapper::getSourceDatabase() const
{
	return m_sourceDatabase;
}

Ref< db::Database > PipelineBuilderWrapper::getOutputDatabase() const
{
	return m_outputDatabase;
}

Ref< db::Instance > PipelineBuilderWrapper::createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid)
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
		return instance;
	else
		return 0;
}

Ref< const ISerializable > PipelineBuilderWrapper::getObjectReadOnly(const Guid& instanceGuid)
{
	return m_objectCache->get(instanceGuid);
}

Ref< IStream > PipelineBuilderWrapper::openFile(const Path& basePath, const std::wstring& fileName)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_openFileLock);

	m_transport->flush< editor::AgentStream >();

	editor::AgentOpenFile agentOpenFile(basePath, fileName);
	if (!m_transport->send(&agentOpenFile))
		return 0;

	Ref< editor::AgentStream > agentStream;
	if (m_transport->recv< editor::AgentStream >(60000, agentStream) <= 0)
		return 0;

	return net::RemoteStream::connect(
		net::SocketAddressIPv4(m_host, m_streamServerPort),
		agentStream->getPublicId()
	);
}

Ref< IStream > PipelineBuilderWrapper::createTemporaryFile(const std::wstring& fileName)
{
	T_FATAL_ERROR;
	return 0;
}

Ref< IStream > PipelineBuilderWrapper::openTemporaryFile(const std::wstring& fileName)
{
	T_FATAL_ERROR;
	return 0;
}

Ref< IPipelineReport > PipelineBuilderWrapper::createReport(const std::wstring& name, const Guid& guid)
{
	return 0;
}

	}
}
