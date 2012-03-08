#include "Core/Misc/TString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Online/Steam/SteamStatistics.h"
#include "Online/Steam/SteamSessionManager.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamStatistics", SteamStatistics, IStatisticsProvider)

SteamStatistics::SteamStatistics(SteamSessionManager* sessionManager, const std::list< std::wstring >& statIds)
:	m_sessionManager(sessionManager)
,	m_callbackUserStatsStored(this, &SteamStatistics::OnUserStatsStored)
{
	m_statIds.insert(statIds.begin(), statIds.end());
}

bool SteamStatistics::enumerate(std::map< std::wstring, float >& outStats)
{
	bool haveStats = m_sessionManager->waitForStats();
	for (std::set< std::wstring >::const_iterator i = m_statIds.begin(); i != m_statIds.end(); ++i)
	{
		float value = 0.0f;
		if (haveStats)
		{
			if (!SteamUserStats()->GetStat(wstombs(*i).c_str(), &value))
				return false;
		}
		outStats.insert(std::make_pair(
			*i,
			value
		));
	}
	return true;
}

bool SteamStatistics::set(const std::wstring& statId, float value)
{
	if (!m_sessionManager->waitForStats())
		return false;

	if (!SteamUserStats()->SetStat(wstombs(statId).c_str(), value))
		return false;

	m_sessionManager->storeStats();
	return true;
}

void SteamStatistics::OnUserStatsStored(UserStatsStored_t* pCallback)
{
}

	}
}
