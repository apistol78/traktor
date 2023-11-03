/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Online/Provider/IAchievementsProvider.h"

namespace traktor::sql
{

class IConnection;

}

namespace traktor::online
{

class LocalAchievements : public IAchievementsProvider
{
	T_RTTI_CLASS;

public:
	explicit LocalAchievements(sql::IConnection* db);

	virtual bool enumerate(std::map< std::wstring, bool >& outAchievements) override final;

	virtual bool set(const std::wstring& achievementId, bool reward) override final;

private:
	Ref< sql::IConnection > m_db;
};

}
