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
#include "Online/Impl/UserCache.h"
#include "Online/Impl/VideoSharing.h"
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
		T_DEBUG(L"Service \"" << type_name(service) << L"\t not ready; waiting...");
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

SessionManager::SessionManager()
:	m_downloadableContent(false)
,	m_connected(false)
{
}

bool SessionManager::create(ISessionManagerProvider* provider, const IGameConfiguration* configuration, bool downloadableContent)
{
	if (!(m_provider = provider))
		return false;

	if (!m_provider->create(configuration))
		return false;

	m_taskQueues[0] = new TaskQueue();
	if (!m_taskQueues[0]->create())
		return false;

	m_taskQueues[1] = new TaskQueue();
	if (!m_taskQueues[1]->create())
		return false;

	IAchievementsProvider* achievementsProvider = m_provider->getAchievements();
	ILeaderboardsProvider* leaderboardsProvider = m_provider->getLeaderboards();
	IMatchMakingProvider* matchMakingProvider = m_provider->getMatchMaking();
	ISaveDataProvider* saveDataProvider = m_provider->getSaveData();
	IStatisticsProvider* statisticsProvider = m_provider->getStatistics();
	IUserProvider* userProvider = m_provider->getUser();
	IVideoSharingProvider* videoSharingProvider = m_provider->getVideoSharing();

	if (userProvider)
	{
		m_userCache = new UserCache(userProvider);
		m_user = m_userCache->get(provider->getCurrentUserHandle());
	}

	if (saveDataProvider)
		m_saveData = new SaveData(saveDataProvider, m_taskQueues[1]);

	if (achievementsProvider)
		m_achievements = new Achievements(achievementsProvider, m_taskQueues[0]);

	if (leaderboardsProvider)
		m_leaderboards = new Leaderboards(leaderboardsProvider, m_userCache, m_taskQueues[0]);

	if (matchMakingProvider && m_userCache)
		m_matchMaking = new MatchMaking(matchMakingProvider, m_userCache, m_taskQueues[0]);

	if (statisticsProvider)
		m_statistics = new Statistics(statisticsProvider, m_taskQueues[0]);

	if (videoSharingProvider)
		m_videoSharing = new VideoSharing(videoSharingProvider);

	m_downloadableContent = downloadableContent;
	m_connected = m_provider->isConnected();

	m_updateTask = new TaskUpdateSessionManager(m_provider);
	m_taskQueues[0]->add(m_updateTask);

	// Perform an initial enumeration; do this even if the provider
	// is disconnected as we need to have systems partially up and running.
	if (m_saveData)
		m_saveData->enqueueEnumeration();
	if (m_achievements)
		m_achievements->enqueueEnumeration();
	if (m_leaderboards)
		m_leaderboards->enqueueEnumeration();
	if (m_statistics)
		m_statistics->enqueueEnumeration();

	return true;
}

void SessionManager::destroy()
{
	m_userCache = 0;
	m_videoSharing = 0;
	m_user = 0;
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

	// Enqueue a new update of back-end if previous update has been finished;
	// as back-end updates might be really expensive we cannot afford to block
	// this update, thus need to run on multiple frames.
	if (m_updateTask->completed())
	{
		m_updateTask->reset();
		m_taskQueues[0]->add(m_updateTask);
	}

	// If provider has become connected then we need to re-enumerate systems.
	bool connected = m_provider->isConnected();
	if (connected && !m_connected)
	{
		if (m_saveData)
			m_saveData->enqueueEnumeration();
		if (m_achievements)
			m_achievements->enqueueEnumeration();
		if (m_leaderboards)
			m_leaderboards->enqueueEnumeration();
		if (m_statistics)
			m_statistics->enqueueEnumeration();
		m_connected = true;		
	}

	return true;
}

std::wstring SessionManager::getLanguageCode() const
{
	return m_provider ? m_provider->getLanguageCode() : L"";
}

bool SessionManager::isConnected() const
{
	return m_connected;
}

bool SessionManager::requireUserAttention() const
{
	return m_provider ? m_provider->requireUserAttention() : false;
}

bool SessionManager::haveDLC(const std::wstring& id) const
{
	if (m_downloadableContent)
		return m_provider ? m_provider->haveDLC(id) : false;
	else
		return false;
}

bool SessionManager::buyDLC(const std::wstring& id) const
{
	if (m_downloadableContent)
		return m_provider ? m_provider->buyDLC(id) : false;
	else
		return false;
}

bool SessionManager::navigateUrl(const net::Url& url) const
{
	return m_provider ? m_provider->navigateUrl(url) : false;
}

bool SessionManager::getFriends(RefArray< IUser >& outFriends, bool onlineOnly) const
{
	if (!m_provider)
		return false;

	std::vector< uint64_t > friendHandles;
	if (!m_provider->getFriends(friendHandles, onlineOnly))
		return false;

	outFriends.resize(friendHandles.size());
	for (size_t i = 0; i < friendHandles.size(); ++i)
		outFriends[i] = m_userCache->get(friendHandles[i]);

	return true;
}

bool SessionManager::findFriend(const std::wstring& name, Ref< IUser >& outFriend) const
{
	if (!m_provider)
		return false;

	uint64_t friendHandle;
	if (!m_provider->findFriend(name, friendHandle))
		return false;

	outFriend = m_userCache->get(friendHandle);
	return true;
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

	outFromUser = m_userCache->get(fromUserHandle);
	return received;
}

uint32_t SessionManager::getCurrentGameCount() const
{
	return m_provider->getCurrentGameCount();
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

IUser* SessionManager::getUser() const
{
	return m_user;
}

IVideoSharing* SessionManager::getVideoSharing() const
{
	return m_videoSharing;
}

	}
}
