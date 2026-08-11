/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

namespace traktor
{

class Guid;

}

namespace traktor::world
{

class Entity;

}

namespace traktor::theater
{

/*! Entity resolver interface.
 * \ingroup Theater
 *
 * Tracks reference entities through identifiers; how such an identifier
 * map to an entity instance depend on the context in which a performance
 * is played, thus is resolved through this interface.
 */
class IEntityResolver
{
public:
	virtual ~IEntityResolver() {}

	virtual world::Entity* findEntity(const Guid& id) const = 0;
};

}
