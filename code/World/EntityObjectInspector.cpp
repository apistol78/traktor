/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/EntityObjectInspector.h"

#include "World/Entity.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.EntityObjectInspector", 0, EntityObjectInspector, IObjectInspector)

bool EntityObjectInspector::supportType(const TypeInfo& objectType) const
{
	return is_type_of< Entity >(objectType);
}

std::wstring EntityObjectInspector::toString(const ITypedObject* object) const
{
	return mandatory_non_null_type_cast< const Entity* >(object)->getName();
}

}
