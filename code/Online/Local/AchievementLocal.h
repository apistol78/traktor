#ifndef traktor_online_AchievementLocal_H
#define traktor_online_AchievementLocal_H

#include "Online/IAchievement.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_LOCAL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace online
	{

class T_DLLCLASS AchievementLocal : public IAchievement
{
	T_RTTI_CLASS;

public:
	AchievementLocal(sql::IConnection* db, int32_t id);

	int32_t getId() const;

	virtual std::wstring getName() const;

	virtual uint32_t getScoreRating() const;

private:
	Ref< sql::IConnection > m_db;
	int32_t m_id;
};

	}
}

#endif	// traktor_online_AchievementLocal_H
