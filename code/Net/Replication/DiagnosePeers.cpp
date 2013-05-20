#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Net/Replication/DiagnosePeers.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.DiagnosePeers", DiagnosePeers, IReplicatorPeers)

DiagnosePeers::DiagnosePeers(IReplicatorPeers* peers)
:	m_peers(peers)
,	m_lastT(-1e8)
,	m_sent(0)
,	m_received(0)
{
	m_timer.start();
}

DiagnosePeers::~DiagnosePeers()
{
	destroy();
}

void DiagnosePeers::destroy()
{
	safeDestroy(m_peers);
}

int32_t DiagnosePeers::update()
{
	int32_t result = m_peers->update();

	double T = m_timer.getElapsedTime();
	if (T >= m_lastT + 10.0)
	{
		if (m_lastT > 0.0)
		{
			std::vector< PeerInfo > info;
			m_peers->getPeers(info);

			double dT = T - m_lastT;
			double up = m_sent * 8.0 / (dT * 1024.0);
			double down = m_received * 8.0 / (dT * 1024.0);

			log::info << L"Network traffic (" << int32_t(info.size()) << L" peer(s)) :" << int32_t(T) << Endl;
			log::info << L"  U " << up << L" kbps" << Endl;
			log::info << L"  D " << down << L" kbps" << Endl;
		}
		m_sent = 0;
		m_received = 0;
		m_lastT = T;
	}

	return result;
}

void DiagnosePeers::setStatus(uint8_t status)
{
	m_peers->setStatus(status);
}

handle_t DiagnosePeers::getHandle() const
{
	return m_peers->getHandle();
}

std::wstring DiagnosePeers::getName() const
{
	return m_peers->getName();
}

handle_t DiagnosePeers::getPrimaryPeerHandle() const
{
	return m_peers->getPrimaryPeerHandle();
}

bool DiagnosePeers::setPrimaryPeerHandle(handle_t handle)
{
	return m_peers->setPrimaryPeerHandle(handle);
}

uint32_t DiagnosePeers::getPeers(std::vector< PeerInfo >& outPeers) const
{
	return m_peers->getPeers(outPeers);
}

int32_t DiagnosePeers::receive(void* data, int32_t size, handle_t& outFromHandle)
{
	int32_t nrecv = m_peers->receive(data, size, outFromHandle);
	if (nrecv <= 0)
		return nrecv;

	m_received += nrecv;
	return nrecv;
}

bool DiagnosePeers::send(handle_t handle, const void* data, int32_t size, bool reliable)
{
	if (!m_peers->send(handle, data, size, reliable))
		return false;

	m_sent += size;
	return true;
}

	}
}
