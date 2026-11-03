/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Stack/RtStackData.h"

#include "Animation/IPoseControllerData.h"
#include "Animation/ITransformTimeData.h"
#include "Animation/Stack/RtStack.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStackData", 0, RtStackData, ISerializable)

Ref< RtStack > RtStackData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform,
	bool editor) const
{
	AlignedVector< RtStack::Layer > layers;

	for (const auto& layerData : m_layers)
	{
		auto& layer = layers.push_back();

		layer.name = render::getParameterHandle(layerData.name);

		if (layerData.animation)
		{
			if (!resourceManager->bind(layerData.animation, layer.animation))
				return nullptr;
		}
		else if (layerData.poseController)
		{
			layer.poseController = layerData.poseController->createInstance(resourceManager, physicsManager, skeleton, worldTransform, editor);
			if (!layer.poseController)
				return nullptr;
		}

		layer.weight = 0.0f;
	}

	return new RtStack(layers);
}

void RtStackData::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Layer, MemberComposite< Layer > >(L"layers", m_layers);
}

void RtStackData::Layer::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> resource::Member< Animation >(L"animation", animation);
	s >> MemberRef< const IPoseControllerData >(L"poseController", poseController);
}

}
