/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Online/IGameConfiguration.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_GC_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class T_DLLCLASS GcGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

private:
	friend class GcSessionManager;
	friend class GcVideoSharingEveryplay;

	std::list< std::wstring > m_achievementIds;
	std::list< std::wstring > m_leaderboardIds;
	std::list< std::wstring > m_statsIds;
};

	}
}

