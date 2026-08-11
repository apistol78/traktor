/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Theater/IEntityResolver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class World;

}

namespace traktor::theater
{

/*! Resolve entities from all entities of a world.
 * \ingroup Theater
 */
class T_DLLCLASS WorldEntityResolver : public IEntityResolver
{
public:
	explicit WorldEntityResolver(const world::World* world);

	virtual world::Entity* findEntity(const Guid& id) const override final;

private:
	const world::World* m_world;
};

}
