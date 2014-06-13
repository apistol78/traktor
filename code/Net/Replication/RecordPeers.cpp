#include "Core/Io/Writer.h"
#include "Core/Misc/SafeDestroy.h"
#include "Net/Replication/RecordPeers.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

struct PeerInfoPredicate
{
	bool operator () (const IReplicatorPeers::PeerInfo& lh, const IReplicatorPeers::PeerInfo& rh) const
	{
		return
			lh.handle == rh.handle &&
			lh.name == rh.name &&
			lh.direct == rh.direct;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.RecordPeers", RecordPeers, Object)

RecordPeers::RecordPeers(IReplicatorPeers* peers, IStream* stream)
:	m_peers(peers)
,	m_stream(stream)
{
	m_timer.start();
}

RecordPeers::~RecordPeers()
{
	destroy();
}

void RecordPeers::destroy()
{
	safeDestroy(m_peers);
	m_stream = 0;
}

bool RecordPeers::update()
{
	bool result = m_peers->update();

	if (
		m_peers->getHandle() != 0 &&
		(m_peers->getHandle() != m_handle || m_peers->getName() != m_name)
	)
	{
		Writer w(m_stream);

		w << uint8_t(0x00);
		w << uint32_t(m_timer.getElapsedTime() * 1000.0f);

		w << uint32_t(1);
		w << m_peers->getHandle();
		w << m_peers->getName();
		w << uint8_t(0);
		w << bool(true);
		w << uint64_t(0);

		m_handle = m_peers->getHandle();
		m_name = m_peers->getName();
	}

	std::vector< PeerInfo > info;
	m_peers->getPeers(info);

	if (
		info.size() != m_info.size() ||
		!std::equal(info.begin(), info.end(), m_info.begin(), PeerInfoPredicate())
	)
	{
		Writer w(m_stream);

		w << uint8_t(0x00);
		w << uint32_t(m_timer.getElapsedTime() * 1000.0f);

		w << uint32_t(info.size());
		for (uint32_t i = 0; i < info.size(); ++i)
		{
			w << info[i].handle;
			w << info[i].name;
			w << info[i].direct;
		}

		m_info = info;
	}

	return result;
}

handle_t RecordPeers::getHandle() const
{
	return m_peers->getHandle();
}

std::wstring RecordPeers::getName() const
{
	return m_peers->getName();
}

handle_t RecordPeers::getPrimaryPeerHandle() const
{
	return m_peers->getPrimaryPeerHandle();
}

bool RecordPeers::setPrimaryPeerHandle(handle_t handle)
{
	return m_peers->setPrimaryPeerHandle(handle);
}

uint32_t RecordPeers::getPeers(std::vector< PeerInfo >& outPeers) const
{
	outPeers = m_info;
	return uint32_t(outPeers.size());
}

int32_t RecordPeers::receive(void* data, int32_t size, handle_t& outFromHandle)
{
	int32_t result = m_peers->receive(data, size, outFromHandle);
	if (result > 0)
	{
		Writer w(m_stream);
		
		w << uint8_t(0x01);
		w << uint32_t(m_timer.getElapsedTime() * 1000.0f);

		w << outFromHandle;
		w << result;

		w.write(data, 1, result);
	}
	return result;
}

bool RecordPeers::send(handle_t handle, const void* data, int32_t size, bool reliable)
{
	bool result = m_peers->send(handle, data, size, reliable);

	Writer w(m_stream);

	w << uint8_t(result ? 0x02 : 0x03);
	w << uint32_t(m_timer.getElapsedTime() * 1000.0f);

	w << handle;
	w << size;
	w.write(data, 1, size);

	return result;
}

	}
}
