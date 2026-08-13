/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/Utilities.h"

#include "World/Editor/Traverser.h"
#include "World/EntityData.h"

namespace traktor::scene
{

void generateEntityIds(world::EntityData* entityData)
{
	entityData->setId(Guid::create());
	world::Traverser::visit(entityData, [](world::EntityData* ed) -> world::Traverser::Result {
		ed->setId(Guid::create());
		return world::Traverser::Result::Continue;
	});
}

}
