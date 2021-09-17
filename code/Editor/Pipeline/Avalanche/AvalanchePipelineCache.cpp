#include "Avalanche/Client/Client.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/OutputStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/Pipeline/Avalanche/AvalanchePipelineCache.h"
#include "Net/Network.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.AvalanchePipelineCache", AvalanchePipelineCache, IPipelineCache)

bool AvalanchePipelineCache::create(const PropertyGroup* settings)
{
	if (!net::Network::initialize())
		return false;

	std::wstring host = settings->getProperty< std::wstring >(L"Pipeline.AvalancheCache.Host", L"");
	int32_t port = settings->getProperty< int32_t >(L"Pipeline.AvalancheCache.Port", 40001);

	m_accessRead = settings->getProperty< bool >(L"Pipeline.FileCache.Read", true);
	m_accessWrite = settings->getProperty< bool >(L"Pipeline.FileCache.Write", true);

	m_client = new avalanche::Client(net::SocketAddressIPv4(host, port));
	return true;
}

AvalanchePipelineCache::~AvalanchePipelineCache()
{
	destroy();
	net::Network::finalize();
}

void AvalanchePipelineCache::destroy()
{
	safeDestroy(m_client);
}

Ref< IStream > AvalanchePipelineCache::get(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (!m_accessRead)
		return nullptr;

	// Combine guid and hash to generate 128-bit storage key.
	Guid gk = guid.permutation(Guid((const uint8_t*)&hash));
	const uint32_t* kv = (const uint32_t*)(const uint8_t*)gk;
	avalanche::Key key(kv[0], kv[1], kv[2], kv[3]);

	Ref< IStream > stream = m_client->get(key);
	if (!stream)
	{
		m_misses++;
		return nullptr;
	}

	m_hits++;
	return new BufferedStream(stream, 16384);
}

Ref< IStream > AvalanchePipelineCache::put(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (!m_accessWrite)
		return nullptr;

	// Combine guid and hash to generate 128-bit storage key.
	Guid gk = guid.permutation(Guid((const uint8_t*)&hash));
	const uint32_t* kv = (const uint32_t*)(const uint8_t*)gk;
	avalanche::Key key(kv[0], kv[1], kv[2], kv[3]);

	Ref< IStream > stream = m_client->put(key);
	if (!stream)
		return nullptr;

	return new BufferedStream(stream, 16384);
}

void AvalanchePipelineCache::getInformation(OutputStream& os)
{
	os << L"Avalanche cache (";
	if (m_accessRead && !m_accessWrite)
		os << L"read";
	else if (!m_accessRead && m_accessWrite)
		os << L"write";
	else if (m_accessRead && m_accessWrite)
		os << L"read+write";
	else
		os << L"disabled";
	if (m_accessRead)
		os << L", " << m_hits << L" hits, " << m_misses << L" misses";
	os << L")";
}

bool AvalanchePipelineCache::commit(const Guid& guid, const PipelineDependencyHash& hash)
{
	return true;
}

	}
}
