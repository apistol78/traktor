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

	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements);

	virtual bool set(const std::wstring& achievementId, bool reward);

private:
	Ref< GcAchievementsImpl > m_impl;
};

	}
}

#endif	// traktor_online_GcAchievements_H
