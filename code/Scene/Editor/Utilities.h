/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace traktor::world
{

class EntityData;

}

namespace traktor::scene
{

/*! Recursively generate entity IDs.
 * \ingroup Scene
 *
 * Using traversal to ensure all referenced entities
 * get ID properly generated.
 */
void generateEntityIds(world::EntityData* entityData);

}
