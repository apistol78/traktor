/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/TrailComponentData.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Spray/Trail.h"
#include "Spray/TrailComponent.h"
#include "Spray/TrailInstance.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.TrailComponentData", 0, TrailComponentData, world::IEntityComponentData)

Ref< TrailComponent > TrailComponentData::createComponent(resource::IResourceManager* resourceManager) const
{
	Ref< Trail > trail = m_trail.createTrail(resourceManager);
	if (!trail)
		return nullptr;

	Ref< TrailInstance > trailInstance = trail->createInstance();
	if (!trailInstance)
		return nullptr;

	return new TrailComponent(trailInstance);
}

int32_t TrailComponentData::getOrdinal() const
{
	return 0;
}

void TrailComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void TrailComponentData::serialize(ISerializer& s)
{
	s >> MemberComposite< TrailData >(L"trail", m_trail);
}

}
