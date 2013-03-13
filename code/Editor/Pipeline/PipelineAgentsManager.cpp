#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Acquire.h"
#include "Database/ConnectionString.h"
#include "Database/Remote/Server/ConnectionManager.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketSet.h"
#include "Net/TcpSocket.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"
#include "Net/Stream/StreamServer.h"
#include "Editor/Pipeline/PipelineAgent.h"
#include "Editor/Pipeline/PipelineAgentsManager.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineAgentsManager", PipelineAgentsManager, Object)

PipelineAgentsManager::PipelineAgentsManager(
	net::StreamServer* streamServer,
	db::ConnectionManager* dbConnectionManager
)
:	m_streamServer(streamServer)
,	m_dbConnectionManager(dbConnectionManager)
,	m_sessionId(Guid::create())
,	m_threadUpdate(0)
{
}

bool PipelineAgentsManager::create(
	const PropertyGroup* settings,
	const db::ConnectionString& sourceDatabaseCs,
	const db::ConnectionString& outputDatabaseCs
)
{
	m_settings = settings;

	m_discoveryManager = new net::DiscoveryManager();
	if (!m_discoveryManager->create(false))
		return false;

	m_dbConnectionManager->setConnectionString(m_sessionId.format() + L"|Source", sourceDatabaseCs.format());
	m_dbConnectionManager->setConnectionString(m_sessionId.format() + L"|Output", outputDatabaseCs.format());

	m_threadUpdate = ThreadManager::getInstance().create(makeFunctor(this, &PipelineAgentsManager::threadUpdate), L"Pipeline agents manager");
	m_threadUpdate->start();

	log::info << L"Pipeline agent manager created" << Endl;
	return true;
}

void PipelineAgentsManager::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_threadUpdate)
	{
		m_threadUpdate->stop();
		ThreadManager::getInstance().destroy(m_threadUpdate);
		m_threadUpdate = 0;
	}

	m_agents.clear();

	safeDestroy(m_discoveryManager);

	if (m_dbConnectionManager)
	{
		m_dbConnectionManager->removeConnectionString(m_sessionId.format() + L"|Source");
		m_dbConnectionManager->removeConnectionString(m_sessionId.format() + L"|Output");
		m_dbConnectionManager = 0;
	}

	m_streamServer = 0;
}

PipelineAgent* PipelineAgentsManager::getIdleAgent()
{
	for (;;)
	{
		// Find first idle agent.
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			for (std::map< std::wstring, Ref< PipelineAgent > >::const_iterator i = m_agents.begin(); i != m_agents.end(); ++i)
			{
				PipelineAgent* agent = i->second;
				T_ASSERT (agent);

				if (agent->isIdle())
					return agent;
			}
		}

		// No idle agent found; wait until agents has been updated.
		m_eventAgentsUpdated.wait(1000);
	}
	return 0;
}

int32_t PipelineAgentsManager::getAgentIndex(const PipelineAgent* agent) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	int32_t index = 0;
	for (std::map< std::wstring, Ref< PipelineAgent > >::const_iterator i = m_agents.begin(); i != m_agents.end(); ++i)
	{
		if (i->second == agent)
			return index;
		++index;
	}
	return -1;
}

void PipelineAgentsManager::waitUntilAllIdle()
{
	for (;;)
	{
		bool allIdle = true;
		
		// Check if all agents are idle.
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			for (std::map< std::wstring, Ref< PipelineAgent > >::const_iterator i = m_agents.begin(); i != m_agents.end(); ++i)
			{
				PipelineAgent* agent = i->second;
				T_ASSERT (agent);

				if (!agent->isIdle())
				{
					allIdle = false;
					break;
				}
			}
		}

		// Working agent found; wait until agents has been updated.
		if (!allIdle)
			m_eventAgentsUpdated.wait(1000);
		else
			break;
	}
}

void PipelineAgentsManager::threadUpdate()
{
	RefArray< net::NetworkService > services;
	while (!m_threadUpdate->stopped())
	{
		// Discover agent services.
		m_discoveryManager->beginFindServices< net::NetworkService >();
		
		// Wait for agent activity.
		net::SocketSet agentSocketSet;
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			for (std::map< std::wstring, Ref< PipelineAgent > >::iterator i = m_agents.begin(); i != m_agents.end(); ++i)
			{
				if (i->second->getTransport()->connected())
					agentSocketSet.add(i->second->getTransport()->getSocket());
			}
		}

		net::SocketSet agentResultSet;
		if (agentSocketSet.select(true, false, false, 1000, agentResultSet) > 0)
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			for (std::map< std::wstring, Ref< PipelineAgent > >::iterator i = m_agents.begin(); i != m_agents.end(); ++i)
			{
				if (i->second->getTransport()->connected())
					i->second->update();
			}
			m_eventAgentsUpdated.broadcast();
		}

		// Get found agents; establish local agent proxies for each new agent found.
		m_discoveryManager->endFindServices< net::NetworkService >(services);
		for (RefArray< net::NetworkService >::const_iterator i = services.begin(); i != services.end(); ++i)
		{
			if ((*i)->getType() != L"Pipeline/Agent")
				continue;

			std::wstring description = (*i)->getProperties()->getProperty< PropertyString >(L"Description", L"");
			std::wstring host = (*i)->getProperties()->getProperty< PropertyString >(L"Host", L"");
			int32_t port = (*i)->getProperties()->getProperty< PropertyInteger >(L"Port", 0);
			int32_t agents = (*i)->getProperties()->getProperty< PropertyInteger >(L"Agents", 0);

			for (int32_t j = 0; j < agents; ++j)
			{
				std::wstring hostAndAgent = host + L":" + toString(j);

				{
					T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
					std::map< std::wstring, Ref< PipelineAgent > >::const_iterator j = m_agents.find(hostAndAgent);
					if (j != m_agents.end() && j->second->getTransport()->connected())
						continue;
				}

				Ref< PipelineAgent > agent = PipelineAgent::create(
					m_streamServer,
					m_sessionId,
					m_settings,
					description + L":" + toString(j),
					host,
					port,
					m_dbConnectionManager->getListenPort()
				);
				if (!agent)
					continue;

				{
					T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
					m_agents[hostAndAgent] = agent;
				}
			}
		}
	}
}

	}
}
