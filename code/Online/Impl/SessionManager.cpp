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

	Ref< ISaveDataProvider > saveDataProvider = m_provider->getSaveData();
	if (saveDataProvider)
		m_saveData = new SaveData(saveDataProvider, m_taskQueues[1]);

	Ref< IAchievementsProvider > achievementsProvider = m_provider->getAchievements();
	if (achievementsProvider)
		m_achievements = new Achievements(achievementsProvider, m_taskQueues[0]);

	Ref< ILeaderboardsProvider > leaderboardsProvider = m_provider->getLeaderboards();
	if (leaderboardsProvider)
		m_leaderboards = new Leaderboards(leaderboardsProvider, m_taskQueues[0]);

	Ref< IMatchMakingProvider > matchMakingProvider = m_provider->getMatchMaking();
	if (matchMakingProvider)
		m_matchMaking = new MatchMaking(matchMakingProvider, m_taskQueues[0]);

	Ref< IStatisticsProvider > statisticsProvider = m_provider->getStatistics();
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

Ref< IAchievements > SessionManager::getAchievements() const
{
	return waitUntilReady(m_achievements);
}

Ref< ILeaderboards > SessionManager::getLeaderboards() const
{
	return waitUntilReady(m_leaderboards);
}

Ref< IMatchMaking > SessionManager::getMatchMaking() const
{
	return waitUntilReady(m_matchMaking);
}

Ref< ISaveData > SessionManager::getSaveData() const
{
	return waitUntilReady(m_saveData);
}

Ref< IStatistics > SessionManager::getStatistics() const
{
	return waitUntilReady(m_statistics);
}

	}
}
