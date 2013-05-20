#ifndef traktor_editor_PipelineBuilderWrapper_H
#define traktor_editor_PipelineBuilderWrapper_H

#include "Core/Thread/ReaderWriterLock.h"
#include "Core/Thread/Semaphore.h"
#include "Editor/IPipelineBuilder.h"

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace editor
	{

class PipelineFactory;
class ReadOnlyObjectCache;

class PipelineBuilderWrapper : public IPipelineBuilder
{
	T_RTTI_CLASS;

public:
	PipelineBuilderWrapper(
		PipelineFactory* pipelineFactory,
		net::BidirectionalObjectTransport* transport,
		const std::wstring& host,
		uint16_t streamServerPort,
		db::Database* sourceDatabase,
		db::Database* outputDatabase,
		ReadOnlyObjectCache* objectCache
	);

	virtual bool build(const IPipelineDependencySet* dependencySet, bool rebuild);

	virtual Ref< ISerializable > buildOutput(const ISerializable* sourceAsset);

	virtual bool buildOutput(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, const Object* buildParams);

	virtual Ref< ISerializable > getBuildProduct(const ISerializable* sourceAsset);

	virtual Ref< db::Database > getSourceDatabase() const;

	virtual Ref< db::Database > getOutputDatabase() const;

	virtual Ref< db::Instance > createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid);

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid);

	virtual Ref< IStream > openFile(const Path& basePath, const std::wstring& fileName);

	virtual Ref< IStream > createTemporaryFile(const std::wstring& fileName);

	virtual Ref< IStream > openTemporaryFile(const std::wstring& fileName);

	virtual Ref< IPipelineReport > createReport(const std::wstring& name, const Guid& guid);

private:
	struct BuiltCacheEntry
	{
		const ISerializable* sourceAsset;
		Ref< ISerializable > product;
	};

	typedef std::list< BuiltCacheEntry > built_cache_list_t;

	Ref< PipelineFactory > m_pipelineFactory;
	Ref< net::BidirectionalObjectTransport > m_transport;
	std::wstring m_host;
	uint16_t m_streamServerPort;
	Ref< db::Database > m_sourceDatabase;
	Ref< db::Database > m_outputDatabase;
	Ref< ReadOnlyObjectCache > m_objectCache;
	Semaphore m_openFileLock;
	Semaphore m_createOutputLock;
	Semaphore m_builtCacheLock;
	std::map< uint32_t, built_cache_list_t > m_builtCache;
};

	}
}

#endif	// traktor_editor_PipelineBuilderWrapper_H
