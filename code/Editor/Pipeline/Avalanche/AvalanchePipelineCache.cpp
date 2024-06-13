/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Avalanche/Client/Client.h"
#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/OutputStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/JobManager.h"
#include "Editor/Pipeline/Avalanche/AvalanchePipelineCache.h"
#include "Net/Network.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.AvalanchePipelineCache", AvalanchePipelineCache, IPipelineCache)

bool AvalanchePipelineCache::create(const PropertyGroup* settings)
{
	if (!net::Network::initialize())
		return false;

	const std::wstring host = settings->getProperty< std::wstring >(L"Pipeline.AvalancheCache.Host", L"");
	const int32_t port = settings->getProperty< int32_t >(L"Pipeline.AvalancheCache.Port", 40001);

	m_accessRead = settings->getProperty< bool >(L"Pipeline.AvalancheCache.Read", true);
	m_accessWrite = settings->getProperty< bool >(L"Pipeline.AvalancheCache.Write", true);

	m_client = new avalanche::Client(net::SocketAddressIPv4(host, port));
	if (!m_client->ping())
	{
		safeDestroy(m_client);
		return false;
	}

	return true;
}

AvalanchePipelineCache::~AvalanchePipelineCache()
{
	destroy();
	net::Network::finalize();
}

void AvalanchePipelineCache::destroy()
{
	if (m_statsJob)
	{
		m_statsJob->wait();
		m_statsJob = nullptr;
	}
	safeDestroy(m_client);
}

Ref< IStream > AvalanchePipelineCache::get(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (!m_accessRead)
		return nullptr;

	// Combine guid and hash to generate 128-bit storage key.
	const Guid gk = guid.permutation(Guid((const uint8_t*)&hash));
	const uint32_t* kv = (const uint32_t*)(const uint8_t*)gk;
	const Key key(kv[0], kv[1], kv[2], kv[3]);

	Ref< IStream > stream = m_client->get(key);
	if (!stream)
	{
		m_misses++;
		return nullptr;
	}

	m_hits++;
	return new compress::InflateStreamLzf(stream);
}

Ref< IStream > AvalanchePipelineCache::put(const Guid& guid, const PipelineDependencyHash& hash)
{
	if (!m_accessWrite)
		return nullptr;

	// Combine guid and hash to generate 128-bit storage key.
	const Guid gk = guid.permutation(Guid((const uint8_t*)&hash));
	const uint32_t* kv = (const uint32_t*)(const uint8_t*)gk;
	const Key key(kv[0], kv[1], kv[2], kv[3]);

	Ref< IStream > stream = m_client->put(key);
	if (!stream)
		return nullptr;

	return new compress::DeflateStreamLzf(stream, 16384);
}

Ref< IStream > AvalanchePipelineCache::get(const Key& key)
{
	if (!m_accessRead)
		return nullptr;

	Ref< IStream > stream = m_client->get(key);
	if (!stream)
		return nullptr;

	return new compress::InflateStreamLzf(stream);
}

Ref< IStream > AvalanchePipelineCache::put(const Key& key)
{
	if (!m_accessWrite)
		return nullptr;

	Ref< IStream > stream = m_client->put(key);
	if (!stream)
		return nullptr;

	return new compress::DeflateStreamLzf(stream, 16384);
}

void AvalanchePipelineCache::getInformation(OutputStream& os)
{
	if (m_statsJob && m_statsJob->wait(0))
		m_statsJob = nullptr;

	if (m_statsJob == nullptr)
	{
		m_statsJob = JobManager::getInstance().add([=, this](){
			m_client->stats(m_stats);
		});
	}

	os << L"Avalanche cache (";
	if (m_accessRead && !m_accessWrite)
		os << L"read";
	else if (!m_accessRead && m_accessWrite)
		os << L"write";
	else if (m_accessRead && m_accessWrite)
		os << L"read+write";
	else
		os << L"disabled";
	os << L", " << m_stats.blobCount << L" blobs, " << formatByteSize(m_stats.memoryUsage);
	if (m_accessRead)
		os << L", " << m_hits << L" hits, " << m_misses << L" misses";
	os << L")";
}

bool AvalanchePipelineCache::commit(const Guid& guid, const PipelineDependencyHash& hash)
{
	return true;
}

}
