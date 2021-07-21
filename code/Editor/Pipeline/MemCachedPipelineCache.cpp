#include <sstream>
#include "Core/Io/IStream.h"
#include "Core/Io/OutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Acquire.h"
#include "Editor/Pipeline/MemCachedPipelineCache.h"
#include "Editor/Pipeline/MemCachedProto.h"
#include "Editor/Pipeline/MemCachedGetStream.h"
#include "Editor/Pipeline/MemCachedPutStream.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

std::string generateKey(const Guid& guid, const PipelineDependencyHash& hash)
{
	std::stringstream ss;
	ss << wstombs(guid.format()) << ":" << hash.pipelineHash << ":" << hash.sourceAssetHash << ":" << hash.sourceDataHash << ":" << hash.filesHash;
	return ss.str();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.MemCachedPipelineCache", MemCachedPipelineCache, IPipelineCache)

MemCachedPipelineCache::~MemCachedPipelineCache()
{
	destroy();
}

bool MemCachedPipelineCache::create(const PropertyGroup* settings)
{
	std::wstring host = settings->getProperty< std::wstring >(L"Pipeline.MemCached.Host");
	int32_t port = settings->getProperty< int32_t >(L"Pipeline.MemCached.Port", 11211);
	m_addr = net::SocketAddressIPv4(host, port);
	m_accessRead = settings->getProperty< bool >(L"Pipeline.MemCached.Read", true);
	m_accessWrite = settings->getProperty< bool >(L"Pipeline.MemCached.Write", true);

	Ref< MemCachedProto > proto = acquireProto();
	if (!proto)
	{
		log::error << L"Memcached pipeline cache failed; unable to connect to memcached at \"" << host << L"\" @ " << port << L"." << Endl;
		return false;
	}
	m_protos.push_back(proto);

	return true;
}

void MemCachedPipelineCache::destroy()
{
	m_protos.clear();
}

Ref< IStream > MemCachedPipelineCache::get(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (m_accessRead)
	{
		Ref< MemCachedProto > proto = acquireProto();
		if (!proto)
			return nullptr;

		Ref< MemCachedGetStream > stream = new MemCachedGetStream(this, proto, generateKey(guid, hash));

		// Request end block; do not try to open non-finished, uncommitted cache streams.
		if (!stream->requestEndBlock())
			return nullptr;

		// Request first block of data.
		if (!stream->requestNextBlock())
			return nullptr;

		return stream;
	}
	else
		return nullptr;
}

Ref< IStream > MemCachedPipelineCache::put(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (m_accessWrite)
	{
		Ref< MemCachedProto > proto = acquireProto();
		if (proto)
			return new MemCachedPutStream(this, proto, generateKey(guid, hash));
	}
	return nullptr;
}

bool MemCachedPipelineCache::commit(const Guid& guid, const PipelineDependencyHash& hash)
{
	std::stringstream ss;
	std::string command;
	std::string reply;

	Ref< MemCachedProto > proto = acquireProto();
	if (!proto)
		return false;

	ss << "set " << generateKey(guid, hash) << ":END 0 0 1";

	command = ss.str();
	T_DEBUG(mbstows(command));

	if (!proto->sendCommand(command))
	{
		log::error << L"Unable to store cache block; unable to send command." << Endl;
		return false;
	}

	uint8_t endData[3] = { 0x22, 0x00, 0x00 };
	if (!proto->writeData(endData, 1))
	{
		log::error << L"Unable to store cache block; unable to write data." << Endl;
		return false;
	}

	if (!proto->readReply(reply))
	{
		log::error << L"Unable to store cache block; unable to read reply." << Endl;
		return false;
	}

	if (reply != "STORED")
	{
		log::error << L"Unable to store cache block; server unable to store data." << Endl;
		return false;
	}

	return true;
}

void MemCachedPipelineCache::getInformation(OutputStream& os)
{
	os << L"Memcached cache (";
	if (m_accessRead && !m_accessWrite)
		os << L"read";
	else if (!m_accessRead && m_accessWrite)
		os << L"write";
	else if (m_accessRead && m_accessWrite)
		os << L"read+write";
	else
		os << L"disabled";

	Ref< MemCachedProto > proto = acquireProto();
	if (!proto)
	{
		os << L")";
		return;
	}

	if (!proto->sendCommand("stats"))
	{
		os << L")";
		return;
	}

	SmallMap< std::string, std::string > stats;
	for(;;)
	{
		std::string reply;
		if (!proto->readReply(reply))
			break;

		if (reply == "END")
			break;

		std::vector< std::string > args;
		Split< std::string >::any(reply, " ", args);
		if (args.size() < 3)
			continue;

		stats[args[1]] = args[2];
	}

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_protos.push_back(proto);
	}

	uint64_t currentItems = parseString< uint64_t >(stats["curr_items"], 0);
	os << L", " << currentItems << L" items";

	uint64_t bytes = parseString< uint64_t >(stats["bytes"], 0);
	uint64_t maxBytes = parseString< uint64_t >(stats["limit_maxbytes"], 0);
	os << L", " << formatByteSize(bytes);
	if (maxBytes > 0)
		os << L", " << ((bytes * 100) / maxBytes) << L" %";

#if defined(_DEBUG)
	uint64_t evictions = parseString< uint64_t >(stats["evictions"], 0);
	uint64_t reclaimed = parseString< uint64_t >(stats["reclaimed"], 0);
	os << L", " << evictions << L" evictions, " << reclaimed << L" reclaimed";
#endif

	os << L")";
}

Ref< MemCachedProto > MemCachedPipelineCache::acquireProto()
{
	Ref< MemCachedProto > proto;

	// Try to get an already established connection.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (!m_protos.empty())
		{
			proto = m_protos.back();
			m_protos.pop_back();
		}
	}

	// If no existing connection found then create a new one.
	if (!proto)
	{
		Ref< net::TcpSocket > socket = new net::TcpSocket();
		if (!socket->connect(m_addr))
			return nullptr;
		proto = new MemCachedProto(socket);
	}

	return proto;
}

	}
}
