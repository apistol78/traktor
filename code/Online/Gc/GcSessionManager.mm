#import <GameKit/GameKit.h>
#include "Core/Log/Log.h"
#include "Online/Gc/GcAchievements.h"
#include "Online/Gc/GcLeaderboards.h"
#include "Online/Gc/GcMatchMaking.h"
#include "Online/Gc/GcSaveData.h"
#include "Online/Gc/GcSessionManager.h"
#include "Online/Gc/GcStatistics.h"
#include "Online/Gc/GcUser.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcSessionManager", GcSessionManager, ISessionManagerProvider)

bool GcSessionManager::create()
{
	// Ensure presence of game center API.
	Class gcClass = (NSClassFromString(@"GKLocalPlayer"));
	if (!gcClass)
	{
		log::error << L"GameCenter API missing; unable to create GameCenter session manager" << Endl;
		return false;
	}
	
	// Ensure iOS 4.1 or later.
	NSString* requiredVersion = @"4.1";
	NSString* currentVersion = [[UIDevice currentDevice] systemVersion];
	BOOL versionSupported = ([currentVersion compare: requiredVersion options:NSNumericSearch] != NSOrderedAscending);
	if (!versionSupported)	
	{
		log::error << L"Old iOS version; at least 4.1 is required" << Endl;
		return false;
	}

	// Authenticate user.
	if ([GKLocalPlayer localPlayer].authenticated == NO)
	{
	}
	
	// Create provider wrappers.
	m_achievements = new GcAchievements();
	m_leaderboards = new GcLeaderboards();
	m_matchMaking = new GcMatchMaking();
	m_saveData = new GcSaveData();
	m_statistics = new GcStatistics();
	m_user = new GcUser();

	return true;
}

void GcSessionManager::destroy()
{
	m_user = 0;
	m_statistics = 0;
	m_saveData = 0;
	m_matchMaking = 0;
	m_leaderboards = 0;
	m_achievements = 0;
}

bool GcSessionManager::update()
{
	return true;
}

std::wstring GcSessionManager::getLanguageCode() const
{
	return L"en";
}

bool GcSessionManager::isConnected() const
{
	return true;
}

bool GcSessionManager::requireUserAttention() const
{
	return false;
}

uint64_t GcSessionManager::getCurrentUserHandle() const
{
	return 0;
}

bool GcSessionManager::haveP2PData() const
{
	return false;
}

uint32_t GcSessionManager::receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const
{
	return 0;
}

IAchievementsProvider* GcSessionManager::getAchievements() const
{
	return m_achievements;
}

ILeaderboardsProvider* GcSessionManager::getLeaderboards() const
{
	return m_leaderboards;
}

IMatchMakingProvider* GcSessionManager::getMatchMaking() const
{
	return m_matchMaking;
}

ISaveDataProvider* GcSessionManager::getSaveData() const
{
	return m_saveData;
}

IStatisticsProvider* GcSessionManager::getStatistics() const
{
	return m_statistics;
}

IUserProvider* GcSessionManager::getUser() const
{
	return m_user;
}

	}
}
