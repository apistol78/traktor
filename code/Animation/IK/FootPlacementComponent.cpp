/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/IK/FootPlacementComponent.h"

#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Physics/PhysicsManager.h"
#include "World/Entity.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.FootPlacementComponent", FootPlacementComponent, world::IEntityComponent)

FootPlacementComponent::FootPlacementComponent(
	physics::PhysicsManager* physicsManager,
	const AlignedVector< render::handle_t >& footJoints,
	uint32_t traceInclude,
	uint32_t traceIgnore,
	float offset,
	float range
)
	: m_physicsManager(physicsManager)
	, m_footJoints(footJoints)
	, m_traceInclude(traceInclude)
	, m_traceIgnore(traceIgnore)
	, m_offset(offset)
	, m_range(range)
{
}

void FootPlacementComponent::destroy()
{
	m_physicsManager = nullptr;
}

void FootPlacementComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void FootPlacementComponent::setTransform(const Transform& transform)
{
}

Aabb3 FootPlacementComponent::getBoundingBox() const
{
	return Aabb3();
}

void FootPlacementComponent::update(const world::UpdateParams& update)
{
	const Transform ownerTransform = m_owner->getTransform();
	const Transform ownerTransformInv = ownerTransform.inverse();
	Transform footTransform;

	auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
	if (!skeletonComponent)
		return;

	for (const auto footJoint : m_footJoints)
	{
		if (!skeletonComponent->getPoseTransform(footJoint, footTransform))
			continue;

		const Vector4 footPosition = ownerTransform * footTransform.translation().xyz1();

		physics::QueryResult result;
		if (m_physicsManager->queryRay(
			footPosition + Vector4(0.0f, m_range - m_offset, 0.0f),
			Vector4(0.0f, -1.0f, 0.0f),
			2.0f * m_range,
			physics::QueryFilter(m_traceInclude, m_traceIgnore),
			false,
			result
		))
		{
			const Vector4 hit = (result.position + Vector4(0.0f, m_offset, 0.0f)).xyz1();
			skeletonComponent->setPoseTransform(
				footJoint,
				Transform(
					ownerTransformInv * hit,
					footTransform.rotation()
				),
				true
			);
		}
	}
}

}
