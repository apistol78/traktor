/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/StateGraph/RtStateNodeStackData.h"

#include "Animation/Animation.h"
#include "Animation/ITransformTimeData.h"
#include "Animation/StateGraph/RtStateNodeStack.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Render/Types.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStateNodeStackData", 0, RtStateNodeStackData, RtStateNodeData)

Ref< RtStateNode > RtStateNodeStackData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform,
	bool editor) const
{
	AlignedVector< RtStateNodeStack::Layer > layers;
	for (const auto& layerData : m_layers)
	{
		if (!layerData.animation)
			return nullptr;

		auto& layer = layers.push_back();
		layer.condition = layerData.condition;

		if (!resourceManager->bind(layerData.animation, layer.animation))
			return nullptr;

		if (layerData.transformTime)
			layer.transformTime = layerData.transformTime->createInstance();
	}
	return  new RtStateNodeStack(layers);
}

void RtStateNodeStackData::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Layer, MemberComposite< Layer > >(L"layers", m_layers);
}

void RtStateNodeStackData::Layer::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"condition", condition);
	s >> resource::Member< Animation >(L"animation", animation);
	s >> MemberRef< const ITransformTimeData >(L"transformTime", transformTime);
}

}
