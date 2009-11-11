#include <sstream>
#include "Editor/MemCachedPipelineCache.h"
#include "Editor/MemCachedProto.h"
#include "Editor/MemCachedGetStream.h"
#include "Editor/MemCachedPutStream.h"
#include "Editor/Settings.h"
#include "Net/Network.h"
#include "Net/TcpSocket.h"
#include "Net/SocketAddressIPv4.h"
#include "Core/Io/Stream.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

std::string generateKey(const Guid& guid, uint32_t hash)
{
	std::stringstream ss;
	ss << wstombs(guid.format()) << ":" << hash;
	return ss.str();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.MemCachedPipelineCache", MemCachedPipelineCache, IPipelineCache)

MemCachedPipelineCache::MemCachedPipelineCache()
:	m_accessRead(true)
,	m_accessWrite(true)
{
	net::Network::initialize();
}

MemCachedPipelineCache::~MemCachedPipelineCache()
{
	destroy();
	net::Network::finalize();
}

bool MemCachedPipelineCache::create(const Settings* settings)
{
	std::wstring host = settings->getProperty< PropertyString >(L"Pipeline.MemCached.Host");
	int32_t port = settings->getProperty< PropertyInteger >(L"Pipeline.MemCached.Port", 11211);

	m_socket = gc_new< net::TcpSocket >();
	if (!m_socket->connect(net::SocketAddressIPv4(host, port)))
		return false;

	m_accessRead = settings->getProperty< PropertyBoolean >(L"Pipeline.MemCached.Read", true);
	m_accessWrite = settings->getProperty< PropertyBoolean >(L"Pipeline.MemCached.Write", true);
	m_proto = gc_new< MemCachedProto >(m_socket);

	return true;
}

void MemCachedPipelineCache::destroy()
{
	if (m_socket)
	{
		m_socket->close();
		m_socket = 0;
	}

	m_proto = 0;
}

Ref< Stream > MemCachedPipelineCache::get(const Guid& guid, uint32_t hash)
{
	if (m_accessRead)
	{
		Ref< MemCachedGetStream > stream = gc_new< MemCachedGetStream >(m_proto, generateKey(guid, hash));
		return stream->requestNextBlock() ? stream : 0;
	}
	else
		return 0;
}

Ref< Stream > MemCachedPipelineCache::put(const Guid& guid, uint32_t hash)
{
	if (m_accessWrite)
		return gc_new< MemCachedPutStream >(m_proto, generateKey(guid, hash));
	else
		return 0;
}

	}
}
