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

class GroupComponent;

}

namespace traktor::theater
{

/*! Resolve entities from children of a group.
 * \ingroup Theater
 *
 * Direct children are searched before nested groups; since entity
 * identifiers are preserved when an entity is instantiated from an
 * external entity this keep a performance within its own instance.
 */
class T_DLLCLASS GroupEntityResolver : public IEntityResolver
{
public:
	explicit GroupEntityResolver(const world::GroupComponent* group);

	virtual world::Entity* findEntity(const Guid& id) const override final;

private:
	const world::GroupComponent* m_group;
};

}
