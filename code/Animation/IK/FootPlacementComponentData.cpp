/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/IK/FootPlacementComponentData.h"

#include "Animation/IK/FootPlacementComponent.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberSmallSet.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.FootPlacementComponentData", 0, FootPlacementComponentData, world::IEntityComponentData)

Ref< FootPlacementComponent > FootPlacementComponentData::createComponent(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager
) const
{
	uint32_t traceInclude = 0;
	uint32_t traceIgnore = 0;

	for (const auto& traceIncludeIt : m_traceInclude)
	{
		resource::Proxy< physics::CollisionSpecification > traceGroup;
		if (!resourceManager->bind(traceIncludeIt, traceGroup))
			return nullptr;
		traceInclude |= traceGroup->getBitMask();
	}
	for (const auto& traceIgnoreId : m_traceIgnore)
	{
		resource::Proxy< physics::CollisionSpecification > traceGroup;
		if (!resourceManager->bind(traceIgnoreId, traceGroup))
			return nullptr;
		traceIgnore |= traceGroup->getBitMask();
	}

	AlignedVector< render::handle_t > footJoints;
	footJoints.reserve(m_footJoints.size());
	for (const auto& footJoint : m_footJoints)
		footJoints.push_back(render::getParameterHandle(footJoint));

	return new FootPlacementComponent(
		physicsManager,
		footJoints,
		traceInclude,
		traceIgnore,
		m_offset,
		m_range
	);
}

int32_t FootPlacementComponentData::getOrdinal() const
{
	return -60;
}

void FootPlacementComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void FootPlacementComponentData::serialize(ISerializer& s)
{
	s >> MemberSmallSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"traceInclude", m_traceInclude);
	s >> MemberSmallSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"traceIgnore", m_traceIgnore);
	s >> MemberAlignedVector< std::wstring >(L"footJoints", m_footJoints);
	s >> Member< float >(L"offset", m_offset, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"range", m_range, AttributeUnit(UnitType::Metres) | AttributeRange(0.0f));
}

}
