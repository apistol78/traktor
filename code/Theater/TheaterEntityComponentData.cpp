/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/TheaterEntityComponentData.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Theater/ActData.h"
#include "Theater/Performance.h"
#include "Theater/TheaterEntityComponent.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.theater.TheaterEntityComponentData", 0, TheaterEntityComponentData, world::IEntityComponentData)

Ref< TheaterEntityComponent > TheaterEntityComponentData::createComponent(const world::IEntityBuilder* entityBuilder) const
{
	Ref< Performance > performance = Performance::create(m_acts, false, entityBuilder);
	if (!performance)
		return nullptr;

	return new TheaterEntityComponent(performance);
}

int32_t TheaterEntityComponentData::getOrdinal() const
{
	// Ensure the owner's group, and thus all animated entities, has been
	// created before this component.
	return 1000;
}

void TheaterEntityComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
	// Nothing to do; tracks are relative the owner's transform so the
	// performance follow the owner as it is moved.
}

void TheaterEntityComponentData::serialize(ISerializer& s)
{
	s >> MemberRefArray< ActData >(L"acts", m_acts);
}

}
