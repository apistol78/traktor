#pragma once

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

