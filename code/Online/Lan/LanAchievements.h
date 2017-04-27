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
	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements) T_OVERRIDE T_FINAL;

	virtual bool set(const std::wstring& achievementId, bool reward) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_online_LanAchievements_H
