/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/AnimatedMeshComponent.h"
#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/AnimationEntityFactory.h"
#include "Animation/JointBindingComponent.h"
#include "Animation/JointBindingComponentData.h"
#include "Animation/SkeletonComponent.h"
#include "Animation/SkeletonComponentData.h"
#include "Animation/Boids/BoidsComponent.h"
#include "Animation/Boids/BoidsComponentData.h"
#include "Animation/Cloth/ClothComponent.h"
#include "Animation/Cloth/ClothComponentData.h"
#include "Animation/IK/IKComponent.h"
#include "Animation/IK/IKComponentData.h"
#include "Animation/PathEntity/PathComponentData.h"
#include "Animation/Rotator/OrientateComponent.h"
#include "Animation/Rotator/OrientateComponentData.h"
#include "Animation/Rotator/PendulumComponentData.h"
#include "Animation/Rotator/RotatorComponentData.h"
#include "Animation/Rotator/WobbleComponentData.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationEntityFactory", AnimationEntityFactory, world::AbstractEntityFactory)

AnimationEntityFactory::AnimationEntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, physics::PhysicsManager* physicsManager)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_physicsManager(physicsManager)
{
}

const TypeInfoSet AnimationEntityFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< AnimatedMeshComponentData >();
	typeSet.insert< BoidsComponentData >();
	typeSet.insert< ClothComponentData >();
	typeSet.insert< IKComponentData >();
	typeSet.insert< JointBindingComponentData >();
	typeSet.insert< OrientateComponentData >();
	typeSet.insert< PathComponentData >();
	typeSet.insert< PendulumComponentData >();
	typeSet.insert< RotatorComponentData >();
	typeSet.insert< SkeletonComponentData >();
	typeSet.insert< WobbleComponentData >();
	return typeSet;
}

Ref< world::IEntityComponent > AnimationEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto animatedMeshComponentData = dynamic_type_cast< const AnimatedMeshComponentData* >(&entityComponentData))
		return animatedMeshComponentData->createComponent(m_resourceManager, m_renderSystem, m_physicsManager, builder);
	else if (auto boidsComponentData = dynamic_type_cast< const BoidsComponentData* >(&entityComponentData))
		return boidsComponentData->createComponent();
	else if (auto clothComponentData = dynamic_type_cast< const ClothComponentData* >(&entityComponentData))
		return clothComponentData->createComponent(m_resourceManager, m_renderSystem);
	else if (auto ikComponentData = dynamic_type_cast< const IKComponentData* >(&entityComponentData))
		return ikComponentData->createComponent();
	else if (auto jointBindingComponentData = dynamic_type_cast< const JointBindingComponentData* >(&entityComponentData))
		return jointBindingComponentData->createComponent(builder);
	else if (auto orientateComponentData = dynamic_type_cast< const OrientateComponentData* >(&entityComponentData))
		return orientateComponentData->createComponent();
	else if (auto pathComponentData = dynamic_type_cast< const PathComponentData* >(&entityComponentData))
		return pathComponentData->createComponent();
	else if (auto pendulumComponentData = dynamic_type_cast< const PendulumComponentData* >(&entityComponentData))
		return pendulumComponentData->createComponent();
	else if (auto rotatorComponentData = dynamic_type_cast< const RotatorComponentData* >(&entityComponentData))
		return rotatorComponentData->createComponent();
	else if (auto skeletonComponentData = dynamic_type_cast< const SkeletonComponentData* >(&entityComponentData))
		return skeletonComponentData->createComponent(m_resourceManager, m_physicsManager);
	else if (auto wobbleComponentData = dynamic_type_cast< const WobbleComponentData* >(&entityComponentData))
		return wobbleComponentData->createComponent();
	else
		return nullptr;
}

}
