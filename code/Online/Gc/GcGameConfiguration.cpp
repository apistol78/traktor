/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Online/Gc/GcGameConfiguration.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.online.GcGameConfiguration", 3, GcGameConfiguration, IGameConfiguration)

void GcGameConfiguration::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 3);

	s >> MemberStlList< std::wstring >(L"achievementIds", m_achievementIds);
	s >> MemberStlList< std::wstring >(L"leaderboardIds", m_leaderboardIds);
	s >> MemberStlList< std::wstring >(L"statsIds", m_statsIds);
}

}
