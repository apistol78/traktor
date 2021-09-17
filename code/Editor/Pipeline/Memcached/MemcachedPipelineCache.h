#pragma once

#include "Core/Thread/Semaphore.h"
#include "Editor/IPipelineCache.h"
#include "Net/SocketAddressIPv4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class MemcachedProto;

class T_DLLCLASS MemcachedPipelineCache : public IPipelineCache
{
	T_RTTI_CLASS;

public:
	MemcachedPipelineCache();

	virtual ~MemcachedPipelineCache();

	virtual bool create(const PropertyGroup* settings) override final;

	virtual void destroy() override final;

	virtual Ref< IStream > get(const Guid& guid, const PipelineDependencyHash& hash) override final;

	virtual Ref< IStream > put(const Guid& guid, const PipelineDependencyHash& hash) override final;

	virtual bool commit(const Guid& guid, const PipelineDependencyHash& hash) override final;

	virtual void getInformation(OutputStream& os) override final;

private:
	friend class MemcachedGetStream;
	friend class MemcachedPutStream;

	net::SocketAddressIPv4 m_addr;
	bool m_accessRead = true;
	bool m_accessWrite = true;
	Semaphore m_lock;
	RefArray< MemcachedProto > m_protos;
	uint32_t m_hits = 0;
	uint32_t m_misses = 0;
	
	Ref< MemcachedProto > acquireProto();
};

	}
}

