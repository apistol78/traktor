/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_GcAchievements_H
#define traktor_online_GcAchievements_H

#include <list>
#include "Online/Provider/IAchievementsProvider.h"

namespace traktor
{
	namespace online
	{

class GcAchievementsImpl;

class GcAchievements : public IAchievementsProvider
{
	T_RTTI_CLASS;

public:
	GcAchievements(const std::list< std::wstring >& achievementIds);

	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements) T_OVERRIDE T_FINAL;

	virtual bool set(const std::wstring& achievementId, bool reward) T_OVERRIDE T_FINAL;

private:
	Ref< GcAchievementsImpl > m_impl;
};

	}
}

#endif	// traktor_online_GcAchievements_H
