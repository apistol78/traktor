#include "Core/Misc/SafeDestroy.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Online/Lan/LanAchievements.h"
#include "Online/Lan/LanLeaderboards.h"
#include "Online/Lan/LanMatchMaking.h"
#include "Online/Lan/LanSaveData.h"
#include "Online/Lan/LanSessionManager.h"
#include "Online/Lan/LanStatistics.h"
#include "Online/Lan/LanUser.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.LanSessionManager", 0, LanSessionManager, ISessionManagerProvider)

LanSessionManager::LanSessionManager()
:	m_userHandle(0)
{
}

bool LanSessionManager::create(const IGameConfiguration* configuration)
{
	m_discoveryManager = new net::DiscoveryManager();
	if (!m_discoveryManager->create(net::MdFindServices | net::MdPublishServices | net::MdVerbose))
		return false;

	Guid unique = Guid::create();
	const uint8_t* p = unique;
	m_userHandle = *(const uint64_t*)&p[0] ^ *(const uint64_t*)&p[8];

	m_achievements = new LanAchievements();
	m_leaderboards = new LanLeaderboards();
	m_matchMaking = new LanMatchMaking(m_discoveryManager, m_userHandle);
	m_saveData = new LanSaveData();
	m_statistics = new LanStatistics();
	m_user = new LanUser();

	return true;
}

void LanSessionManager::destroy()
{
	safeDestroy(m_discoveryManager);
}

bool LanSessionManager::update()
{
	if (m_matchMaking)
		m_matchMaking->update();

	return true;
}

std::wstring LanSessionManager::getLanguageCode() const
{
	return L"en";
}

bool LanSessionManager::isConnected() const
{
	return true;
}

bool LanSessionManager::requireUserAttention() const
{
	return false;
}

bool LanSessionManager::haveDLC(const std::wstring& id) const
{
	return true;
}

bool LanSessionManager::buyDLC(const std::wstring& id) const
{
	return true;
}

bool LanSessionManager::navigateUrl(const net::Url& url) const
{
	return false;
}

uint64_t LanSessionManager::getCurrentUserHandle() const
{
	return m_userHandle;
}

bool LanSessionManager::getFriends(std::vector< uint64_t >& outFriends, bool onlineOnly) const
{
	return true;
}

bool LanSessionManager::findFriend(const std::wstring& name, uint64_t& outFriendUserHandle) const
{
	return false;
}

bool LanSessionManager::haveP2PData() const
{
	return false;
}

uint32_t LanSessionManager::receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const
{
	return 0;
}

uint32_t LanSessionManager::getCurrentGameCount() const
{
	return 0;
}

IAchievementsProvider* LanSessionManager::getAchievements() const
{
	return m_achievements;
}

ILeaderboardsProvider* LanSessionManager::getLeaderboards() const
{
	return m_leaderboards;
}

IMatchMakingProvider* LanSessionManager::getMatchMaking() const
{
	return m_matchMaking;
}

ISaveDataProvider* LanSessionManager::getSaveData() const
{
	return m_saveData;
}

IStatisticsProvider* LanSessionManager::getStatistics() const
{
	return m_statistics;
}

IUserProvider* LanSessionManager::getUser() const
{
	return m_user;
}

IVideoSharingProvider* LanSessionManager::getVideoSharing() const
{
	return 0;
}

IVoiceChatProvider* LanSessionManager::getVoiceChat() const
{
	return 0;
}

	}
}
