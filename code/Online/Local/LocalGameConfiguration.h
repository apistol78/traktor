/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LocalGameConfiguration_H
#define traktor_online_LocalGameConfiguration_H

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
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

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

#endif	// traktor_online_LocalGameConfiguration_H
