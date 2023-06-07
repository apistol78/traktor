/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Cloth/ClothComponent.h"
#include "Animation/Cloth/ClothComponentData.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.ClothComponentData", 1, ClothComponentData, world::IEntityComponentData)

Ref< ClothComponent > ClothComponentData::createComponent(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem
) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

	Ref< ClothComponent > clothEntity = new ClothComponent();
	if (!clothEntity->create(
		renderSystem,
		shader,
		m_resolutionX,
		m_resolutionY,
		m_scale,
		m_jointRadius,
		m_damping,
		m_solverIterations
	))
		return nullptr;

	// Fixate nodes by setting infinite mass.
	for (const auto& anchor : m_anchors)
	{
		clothEntity->setNodeAnchor(
			render::getParameterHandle(anchor.jointName),
			anchor.jointOffset,
			anchor.x,
			anchor.y
		);
	}

	return clothEntity;
}

int32_t ClothComponentData::getOrdinal() const
{
	return 0;
}

void ClothComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void ClothComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< uint32_t >(L"resolutionX", m_resolutionX);
	s >> Member< uint32_t >(L"resolutionY", m_resolutionY);
	s >> Member< float >(L"scale", m_scale, AttributeRange(0.0f));
	if (s.getVersion< ClothComponentData >() >= 1)
		s >> Member< float >(L"jointRadius", m_jointRadius, AttributeRange(0.0f));
	s >> MemberAlignedVector< Anchor, MemberComposite< Anchor > >(L"anchors", m_anchors);
	s >> Member< uint32_t >(L"solverIterations", m_solverIterations, AttributeRange(0));
	s >> Member< float >(L"damping", m_damping, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
}

void ClothComponentData::Anchor::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"jointName", jointName);
	s >> Member< Vector4 >(L"jointOffset", jointOffset);
	s >> Member< uint32_t >(L"x", x);
	s >> Member< uint32_t >(L"y", y);
}

}
