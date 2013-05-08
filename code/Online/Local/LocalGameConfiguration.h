#ifndef traktor_online_LocalGameConfiguration_H
#define traktor_online_LocalGameConfiguration_H

#include <list>
#include "Online/IGameConfiguration.h"

namespace traktor
{
	namespace online
	{

class LocalGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

private:
	friend class LocalSessionManager;

	std::wstring m_dbName;
	std::list< std::wstring > m_achievementIds;
	std::list< std::wstring > m_leaderboardIds;
	std::list< std::wstring > m_statsIds;
};

	}
}

#endif	// traktor_online_LocalGameConfiguration_H
