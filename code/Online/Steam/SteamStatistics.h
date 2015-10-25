#ifndef traktor_online_SteamStatistics_H
#define traktor_online_SteamStatistics_H

#include <list>
#include <steam/steam_api.h>
#include "Online/Provider/IStatisticsProvider.h"
#include "Online/Steam/SteamTypes.h"

namespace traktor
{
	namespace online
	{

class SteamSessionManager;

class SteamStatistics : public IStatisticsProvider
{
	T_RTTI_CLASS;

public:
	SteamStatistics(SteamSessionManager* sessionManager, const std::list< std::wstring >& statIds);

	virtual bool enumerate(std::map< std::wstring, int32_t >& outStats) T_OVERRIDE T_FINAL;

	virtual bool set(const std::wstring& statId, int32_t value) T_OVERRIDE T_FINAL;

private:
	SteamSessionManager* m_sessionManager;
	std::set< std::wstring > m_statIds;

	STEAM_CALLBACK(SteamStatistics, OnUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
};

	}
}

#endif	// traktor_online_SteamStatistics_H
