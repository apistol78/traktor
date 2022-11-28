/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Physics/CompoundShapeDesc.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.CompoundShapeDesc", ShapeDesc::Version, CompoundShapeDesc, ShapeDesc)

void CompoundShapeDesc::serialize(ISerializer& s)
{
	ShapeDesc::serialize(s);
	s >> MemberRefArray< ShapeDesc >(L"shapes", m_shapes);
}

}
