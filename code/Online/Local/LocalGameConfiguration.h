#pragma once

#include <list>
#include <string>
#include "Online/IGameConfiguration.h"

namespace traktor
{
	namespace online
	{

class LocalGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

private:
	friend class LocalSessionManager;

	std::wstring m_dbName;
	std::list< std::wstring > m_achievementIds;
	std::list< std::wstring > m_leaderboardIds;
	std::list< std::wstring > m_statsIds;
	std::list< std::wstring > m_dlcIds;
};

	}
}

