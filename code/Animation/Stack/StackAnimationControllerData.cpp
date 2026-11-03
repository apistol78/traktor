/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Stack/StackAnimationControllerData.h"

#include "Animation/Stack/RtStackData.h"
#include "Animation/Stack/StackAnimationController.h"
#include "Core/Serialization/ISerializer.h"
#include "Render/Types.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StackAnimationControllerData", 0, StackAnimationControllerData, IPoseControllerData)

StackAnimationControllerData::StackAnimationControllerData(const resource::Id< RtStackData >& stack)
	: m_stack(stack)
{
}

Ref< IPoseController > StackAnimationControllerData::createInstance(resource::IResourceManager* resourceManager, physics::PhysicsManager* physicsManager, const Skeleton* skeleton, const Transform& worldTransform, bool editor) const
{
	// Load the (shared, immutable) compiled state graph data through the resource manager.
	resource::Proxy< RtStackData > stackData;
	if (!resourceManager->bind(m_stack, stackData))
		return nullptr;

	Ref< RtStack > stack = stackData->createInstance(resourceManager, physicsManager, skeleton, worldTransform, editor);
	if (!stack)
		return nullptr;

	return new StackAnimationController(stack);
}

void StackAnimationControllerData::serialize(ISerializer& s)
{
	s >> resource::Member< RtStackData >(L"stack", m_stack);
}

}
