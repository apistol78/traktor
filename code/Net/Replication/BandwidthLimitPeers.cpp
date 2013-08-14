#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Net/Replication/BandwidthLimitPeers.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

#if 1
#	define T_LIMIT_DEBUG(x) traktor::log::info << x << traktor::Endl
#else
#	define T_LIMIT_DEBUG(x)
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.BandwidthLimitPeers", BandwidthLimitPeers, IReplicatorPeers)

BandwidthLimitPeers::BandwidthLimitPeers(IReplicatorPeers* peers, int32_t maxSendRate, bool silentDiscard, bool alwaysSendReliable)
:	m_peers(peers)
,	m_maxSendRate(maxSendRate)
,	m_silentDiscard(silentDiscard)
,	m_alwaysSendReliable(alwaysSendReliable)
,	m_lastTime(0.0)
,	m_sent(0)
,	m_currentRate(0)
{
	m_timer.start();
}

BandwidthLimitPeers::~BandwidthLimitPeers()
{
	destroy();
}

void BandwidthLimitPeers::destroy()
{
	safeDestroy(m_peers);
}

bool BandwidthLimitPeers::update()
{
	if (!m_peers->update())
		return false;

	bool sendDisabled = m_currentRate > m_maxSendRate;

	double currentTime = m_timer.getElapsedTime();
	if (m_lastTime > FUZZY_EPSILON)
	{
		double currentRate = m_sent / (currentTime - m_lastTime);
		m_currentRate = int32_t(currentRate * 0.1 + m_currentRate * 0.9);
	}

	if (!sendDisabled && m_currentRate > m_maxSendRate)
		T_LIMIT_DEBUG(L"WARNING: Exceeding bandwidth limit; " << m_currentRate / 1024 << L" KiB/s > limit " << m_maxSendRate / 1024 << L" KiB/s; sending disabled");
	else if (sendDisabled && m_currentRate <= m_maxSendRate)
		T_LIMIT_DEBUG(L"OK: Sending resumed");

	m_lastTime = currentTime;
	m_sent = 0;

	return true;
}

void BandwidthLimitPeers::setStatus(uint8_t status)
{
	m_peers->setStatus(status);
}

void BandwidthLimitPeers::setConnectionState(uint64_t connectionState)
{
	m_peers->setConnectionState(connectionState);
}

handle_t BandwidthLimitPeers::getHandle() const
{
	return m_peers->getHandle();
}

std::wstring BandwidthLimitPeers::getName() const
{
	return m_peers->getName();
}

handle_t BandwidthLimitPeers::getPrimaryPeerHandle() const
{
	return m_peers->getPrimaryPeerHandle();
}

bool BandwidthLimitPeers::setPrimaryPeerHandle(handle_t handle)
{
	return m_peers->setPrimaryPeerHandle(handle);
}

uint32_t BandwidthLimitPeers::getPeers(std::vector< PeerInfo >& outPeers) const
{
	return m_peers->getPeers(outPeers);
}

int32_t BandwidthLimitPeers::receive(void* data, int32_t size, handle_t& outFromHandle)
{
	return m_peers->receive(data, size, outFromHandle);
}

bool BandwidthLimitPeers::send(handle_t handle, const void* data, int32_t size, bool reliable)
{
	if (!m_alwaysSendReliable || !reliable)
	{
		if (m_currentRate > m_maxSendRate)
			return m_silentDiscard ? true : false;
	}

	if (!m_peers->send(handle, data, size, reliable))
		return false;

	m_sent += size;
	return true;
}

	}
}
