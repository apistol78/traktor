#ifndef traktor_online_SteamStatistics_H
#define traktor_online_SteamStatistics_H

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
	SteamStatistics(SteamSessionManager* sessionManager, const wchar_t** statIds);

	virtual bool enumerate(std::map< std::wstring, float >& outStats);

	virtual bool set(const std::wstring& statId, float value);

private:
	SteamSessionManager* m_sessionManager;
	std::set< std::wstring > m_statIds;
	bool m_storeStats;

	STEAM_CALLBACK(SteamStatistics, OnUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
};

	}
}

#endif	// traktor_online_SteamStatistics_H
