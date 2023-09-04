/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/SkeletonUtils.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Animation/RagDoll/RagDollPoseControllerData.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberBitMask.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallSet.h"
#include "Physics/CollisionSpecification.h"
#include "Resource/Member.h"

namespace traktor::animation
{
	namespace
	{

const resource::Id< physics::CollisionSpecification > c_defaultCollision(Guid(L"{F9805131-50C2-504C-9421-13C99E44616C}"));
const resource::Id< physics::CollisionSpecification > c_interactableCollision(Guid(L"{09CB1141-1924-3349-934A-CEB9728D7A61}"));

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RagDollPoseControllerData", 6, RagDollPoseControllerData, IPoseControllerData)

RagDollPoseControllerData::RagDollPoseControllerData()
{
	m_collisionGroup.insert(c_defaultCollision);
	m_collisionMask.insert(c_defaultCollision);
	m_collisionMask.insert(c_interactableCollision);
}

RagDollPoseControllerData::RagDollPoseControllerData(
	const SmallSet< resource::Id< physics::CollisionSpecification > >& collisionGroup,
	const SmallSet< resource::Id< physics::CollisionSpecification > >& collisionMask
)
:	m_collisionGroup(collisionGroup)
,	m_collisionMask(collisionMask)
{
}

Ref< IPoseController > RagDollPoseControllerData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform
) const
{
	Ref< IPoseController > trackPoseController;

	AlignedVector< Transform > jointTransforms;
	calculateJointTransforms(skeleton, jointTransforms);

	if (m_trackPoseController)
	{
		trackPoseController = m_trackPoseController->createInstance(
			resourceManager,
			physicsManager,
			skeleton,
			worldTransform
		);
		if (!trackPoseController)
			return nullptr;
	}

	Ref< RagDollPoseController > poseController = new RagDollPoseController();
	if (!poseController->create(
		resourceManager,
		physicsManager,
		this,
		skeleton,
		worldTransform,
		jointTransforms,
		trackPoseController
	))
		return nullptr;

	return poseController;
}

void RagDollPoseControllerData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< RagDollPoseControllerData >() >= 5);

	s >> MemberSmallSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionGroup", m_collisionGroup);
	s >> MemberSmallSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionMask", m_collisionMask);
	s >> Member< bool >(L"autoDeactivate", m_autoDeactivate);
	s >> Member< bool >(L"enabled", m_enabled);

	if (s.getVersion< RagDollPoseControllerData >() < 6)
	{
		bool fixateJoints;
		s >> Member< bool >(L"fixateJoints", fixateJoints);
	}

	s >> Member< float >(L"limbMass", m_limbMass, AttributeUnit(UnitType::Kilograms) | AttributeRange(0.0f));
	s >> Member< float >(L"linearDamping", m_linearDamping, AttributeUnit(UnitType::Percent) | AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"angularDamping", m_angularDamping, AttributeUnit(UnitType::Percent) | AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"linearThreshold", m_linearThreshold, AttributeUnit(UnitType::Metres, true) | AttributeRange(0.0f));
	s >> Member< float >(L"angularThreshold", m_angularThreshold, AttributeUnit(UnitType::Metres, true) | AttributeRange(0.0f));

	if (s.getVersion< RagDollPoseControllerData >() >= 6)
	{
		const MemberBitMask::Bit c_constraintBits[] =
		{
			{ L"X", 1 },
			{ L"Y", 2 },
			{ L"Z", 4 },
			{ 0 }
		};

		s >> MemberBitMask(L"constraintAxises", m_constraintAxises, c_constraintBits);
	}

	s >> MemberRef< const IPoseControllerData >(L"trackPoseController", m_trackPoseController);
	s >> Member< float >(L"trackLinearTension", m_trackLinearTension, AttributeUnit(UnitType::Percent) | AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"trackAngularTension", m_trackAngularTension, AttributeUnit(UnitType::Percent) | AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"trackDuration", m_trackDuration, AttributeUnit(UnitType::Seconds) | AttributeRange(0.0f));
}

}
