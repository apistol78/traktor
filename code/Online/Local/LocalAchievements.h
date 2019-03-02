#pragma once

#include "Core/Ref.h"
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

	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements) override final;

	virtual bool set(const std::wstring& achievementId, bool reward) override final;

private:
	Ref< sql::IConnection > m_db;
};

	}
}

