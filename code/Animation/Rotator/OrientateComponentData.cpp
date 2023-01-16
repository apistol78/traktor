/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Rotator/OrientateComponent.h"
#include "Animation/Rotator/OrientateComponentData.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.OrientateComponentData", 0, OrientateComponentData, world::IEntityComponentData)

Ref< OrientateComponent > OrientateComponentData::createComponent() const
{
	return new OrientateComponent();
}

int32_t OrientateComponentData::getOrdinal() const
{
	return -1000;
}

void OrientateComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void OrientateComponentData::serialize(ISerializer& s)
{
}

}
