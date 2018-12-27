/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LanAchievements_H
#define traktor_online_LanAchievements_H

#include "Online/Provider/IAchievementsProvider.h"

namespace traktor
{
	namespace online
	{

class LanAchievements : public IAchievementsProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements) override final;

	virtual bool set(const std::wstring& achievementId, bool reward) override final;
};

	}
}

#endif	// traktor_online_LanAchievements_H
