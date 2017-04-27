/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements) T_OVERRIDE T_FINAL;

	virtual bool set(const std::wstring& achievementId, bool reward) T_OVERRIDE T_FINAL;

private:
	Ref< sql::IConnection > m_db;
};

	}
}

#endif	// traktor_online_LocalAchievements_H
