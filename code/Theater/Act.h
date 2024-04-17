/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor::world
{

class World;

}

namespace traktor::theater
{

class Track;

/*! Act
 * \ingroup Theater
 */
class Act : public Object
{
	T_RTTI_CLASS;

public:
	explicit Act(const std::wstring& name, float start, float end, const RefArray< const Track >& tracks);

	bool update(world::World* world, float time, float deltaTime) const;

	const std::wstring& getName() const { return m_name; }

	float getStart() const { return m_start; }

	float getEnd() const { return m_end; }

private:
	std::wstring m_name;
	float m_start;
	float m_end;
	RefArray< const Track > m_tracks;
};

}
