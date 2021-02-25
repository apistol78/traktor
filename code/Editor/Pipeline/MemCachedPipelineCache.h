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

class MemCachedProto;

class T_DLLCLASS MemCachedPipelineCache : public IPipelineCache
{
	T_RTTI_CLASS;

public:
	virtual ~MemCachedPipelineCache();

	virtual bool create(const PropertyGroup* settings) override final;

	virtual void destroy() override final;

	virtual Ref< IStream > get(const Guid& guid, const PipelineDependencyHash& hash) override final;

	virtual Ref< IStream > put(const Guid& guid, const PipelineDependencyHash& hash) override final;

	virtual bool commit(const Guid& guid, const PipelineDependencyHash& hash) override final;

private:
	friend class MemCachedGetStream;
	friend class MemCachedPutStream;

	net::SocketAddressIPv4 m_addr;
	bool m_accessRead = true;
	bool m_accessWrite = true;
	Semaphore m_lock;
	RefArray< MemCachedProto > m_protos;

	Ref< MemCachedProto > acquireProto();
};

	}
}

