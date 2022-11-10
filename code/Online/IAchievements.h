/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>
#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Result;

	namespace online
	{

class T_DLLCLASS IAchievements : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const = 0;

	virtual bool enumerate(std::set< std::wstring >& outAchievementIds) const = 0;

	virtual bool have(const std::wstring& achievementId) const = 0;

	virtual Ref< Result > set(const std::wstring& achievementId, bool reward) = 0;
};

	}
}

