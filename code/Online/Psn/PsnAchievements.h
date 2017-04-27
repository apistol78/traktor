/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_PsnAchievements_H
#define traktor_online_PsnAchievements_H

#include "Online/Provider/IAchievementsProvider.h"
#include "Online/Psn/PsnTypes.h"

namespace traktor
{
	namespace online
	{

class PsnAchievements : public IAchievementsProvider
{
	T_RTTI_CLASS;

public:
	PsnAchievements(const PsnAchievementDesc* achievements, SceNpTrophyContext trophyContext, SceNpTrophyHandle trophyHandle);

	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements);

	virtual bool set(const std::wstring& achievementId, bool reward);

private:
	static int callbackTrophyStatus(SceNpTrophyContext context, SceNpTrophyStatus status, int completed, int total, void *arg);

	const PsnAchievementDesc* m_achievements;
	SceNpTrophyContext m_trophyContext;
	SceNpTrophyHandle m_trophyHandle;
};

	}
}

#endif	// traktor_online_PsnAchievements_H
