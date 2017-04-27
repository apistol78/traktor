/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class GcSessionManager;
	friend class GcVideoSharingEveryplay;

	std::list< std::wstring > m_achievementIds;
	std::list< std::wstring > m_leaderboardIds;
	std::list< std::wstring > m_statsIds;
	std::wstring m_sharingClientId;
	std::wstring m_sharingClientSecret;
	std::wstring m_sharingRedirectURI;
};

	}
}

#endif	// traktor_online_GcGameConfiguration_H
