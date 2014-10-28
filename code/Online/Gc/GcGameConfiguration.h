#ifndef traktor_online_GcGameConfiguration_H
#define traktor_online_GcGameConfiguration_H

#include <list>
#include "Online/IGameConfiguration.h"

namespace traktor
{
	namespace online
	{

class GcGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

private:
	friend class GcSessionManager;
	friend class GcVideoSharingKamcord;

	std::list< std::wstring > m_achievementIds;
	std::list< std::wstring > m_leaderboardIds;
	std::list< std::wstring > m_statsIds;

	std::wstring m_kamcordDeveloperKey;
	std::wstring m_kamcordDeveloperSecret;
	std::wstring m_kamcordAppName;
};

	}
}

#endif	// traktor_online_GcGameConfiguration_H
