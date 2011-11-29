#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Online/Impl/Achievements.h"
#include "Online/Impl/Leaderboards.h"
#include "Online/Impl/MatchMaking.h"
#include "Online/Impl/SaveData.h"
#include "Online/Impl/SessionManager.h"
#include "Online/Impl/Statistics.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/User.h"
#include "Online/Impl/Tasks/TaskUpdateSessionManager.h"
#include "Online/Provider/ISessionManagerProvider.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

template < typename ServiceType >
ServiceType waitUntilReady(const ServiceType& service)
{
	if (!service)
		return 0;

	if (!service->ready())
	{
		log::debug << L"Service \"" << type_name(service) << L"\t not ready; waiting..." << Endl;
		Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
		do 
		{
			currentThread->sleep(100);
		}
		while (!service->ready());
	}

	return service;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SessionManager", SessionManager, ISessionManager)

bool SessionManager::create(ISessionManagerProvider* provider)
{
	if (!(m_provider = provider))
		return false;

	m_taskQueues[0] = new TaskQueue();
	if (!m_taskQueues[0]->create(new TaskUpdateSessionManager(
		m_provider
	)))
		return false;

	m_taskQueues[1] = new TaskQueue();
	if (!m_taskQueues[1]->create(0))
		return false;

	IAchievementsProvider* achievementsProvider = m_provider->getAchievements();
	ILeaderboardsProvider* leaderboardsProvider = m_provider->getLeaderboards();
	IMatchMakingProvider* matchMakingProvider = m_provider->getMatchMaking();
	ISaveDataProvider* saveDataProvider = m_provider->getSaveData();
	IStatisticsProvider* statisticsProvider = m_provider->getStatistics();
	IUserProvider* userProvider = m_provider->getUser();

	if (saveDataProvider)
		m_saveData = new SaveData(saveDataProvider, m_taskQueues[1]);

	if (achievementsProvider)
		m_achievements = new Achievements(achievementsProvider, m_taskQueues[0]);

	if (leaderboardsProvider)
		m_leaderboards = new Leaderboards(leaderboardsProvider, m_taskQueues[0]);

	if (matchMakingProvider && userProvider)
		m_matchMaking = new MatchMaking(matchMakingProvider, userProvider, m_taskQueues[0]);

	if (statisticsProvider)
		m_statistics = new Statistics(statisticsProvider, m_taskQueues[0]);

	return true;
}

void SessionManager::destroy()
{
	m_statistics = 0;
	m_saveData = 0;
	m_matchMaking = 0;
	m_leaderboards = 0;
	m_achievements = 0;

	safeDestroy(m_taskQueues[1]);
	safeDestroy(m_taskQueues[0]);
	safeDestroy(m_provider);
}

bool SessionManager::update()
{
	if (!m_provider)
		return false;

	return true;
}

std::wstring SessionManager::getLanguageCode() const
{
	return m_provider ? m_provider->getLanguageCode() : L"";
}

bool SessionManager::isConnected() const
{
	return m_provider ? m_provider->isConnected() : false;
}

bool SessionManager::requireUserAttention() const
{
	return m_provider ? m_provider->requireUserAttention() : false;
}

bool SessionManager::haveP2PData() const
{
	return m_provider ? m_provider->haveP2PData() : false;
}

uint32_t SessionManager::receiveP2PData(void* data, uint32_t size, Ref< IUser >& outFromUser) const
{
	if (!m_provider)
		return 0;

	uint64_t fromUserHandle = 0;
	uint32_t received = m_provider->receiveP2PData(data, size, fromUserHandle);
	if (!received)
		return 0;

	SmallMap< uint64_t, Ref< IUser > >::const_iterator i = m_p2pUsers.find(fromUserHandle);
	if (i != m_p2pUsers.end())
		outFromUser = i->second;
	else
	{
		outFromUser = new User(m_provider->getUser(), fromUserHandle);
		m_p2pUsers.insert(fromUserHandle, outFromUser);
	}

	return received;
}

IAchievements* SessionManager::getAchievements() const
{
	return waitUntilReady(m_achievements);
}

ILeaderboards* SessionManager::getLeaderboards() const
{
	return waitUntilReady(m_leaderboards);
}

IMatchMaking* SessionManager::getMatchMaking() const
{
	return waitUntilReady(m_matchMaking);
}

ISaveData* SessionManager::getSaveData() const
{
	return waitUntilReady(m_saveData);
}

IStatistics* SessionManager::getStatistics() const
{
	return waitUntilReady(m_statistics);
}

	}
}
