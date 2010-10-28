#ifndef traktor_online_LocalAchievements_H
#define traktor_online_LocalAchievements_H

#include "Online/Provider/IAchievementsProvider.h"

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace online
	{

class LocalAchievements : public IAchievementsProvider
{
	T_RTTI_CLASS;

public:
	LocalAchievements(sql::IConnection* db);

	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements);

	virtual bool set(const std::wstring& achievementId, bool reward);

private:
	Ref< sql::IConnection > m_db;
};

	}
}

#endif	// traktor_online_LocalAchievements_H
