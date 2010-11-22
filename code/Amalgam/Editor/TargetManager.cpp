#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetManager.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Net/SocketAddressIPv4.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetManager", TargetManager, Object)

bool TargetManager::create(uint16_t port, int32_t timeout)
{
	m_listenSocket = new net::TcpSocket();
	if (!m_listenSocket->bind(net::SocketAddressIPv4(port)))
		return false;

	if (!m_listenSocket->listen())
		return false;

	m_timeout = timeout;
	return true;
}

void TargetManager::destroy()
{
	if (m_listenSocket)
	{
		m_listenSocket->close();
		m_listenSocket = 0;
	}
}

bool TargetManager::accept(TargetInstance* targetInstance)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_acceptLock);

	for (RefArray< TargetConnection >::iterator i = m_connections.begin(); i != m_connections.end(); )
	{
		if ((*i)->getTargetInstance() == targetInstance)
			i = m_connections.erase(i);
		else
			++i;
	}

	if (!m_listenSocket->select(true, false, false, m_timeout))
	{
		log::warning << L"Target connection timeout" << Endl;
		return false;
	}

	Ref< net::TcpSocket > targetSocket = m_listenSocket->accept();
	if (!targetSocket)
		return false;

	m_targetSockets.add(targetSocket);
	m_connections.push_back(new TargetConnection(targetInstance, targetSocket));
	
	log::info << L"Target connected successfully" << Endl;
	return true;
}

void TargetManager::update()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_acceptLock);

	if (m_connections.empty())
	{
		ThreadManager::getInstance().getCurrentThread()->sleep(100);
		return;
	}

	net::SocketSet rs;
	if (m_targetSockets.select(true, false, false, 100, rs) <= 0)
		return;

	RefArray< TargetConnection >::iterator i = m_connections.begin();
	while (i != m_connections.end())
	{
		if ((*i)->update())
			++i;
		else
		{
			m_targetSockets.remove((*i)->getSocket());
			i = m_connections.erase(i);
			log::info << L"Target connection terminated" << Endl;
		}
	}
}

	}
}
