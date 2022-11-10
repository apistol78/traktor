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
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.ClothComponentData", 0, ClothComponentData, world::IEntityComponentData)

ClothComponentData::ClothComponentData()
:	m_resolutionX(10)
,	m_resolutionY(10)
,	m_scale(1.0f)
,	m_solverIterations(4)
,	m_damping(0.01f)
{
}

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
		m_damping,
		m_solverIterations
	))
		return nullptr;

	// Fixate nodes by setting infinite mass.
	for (std::vector< Anchor >::const_iterator i = m_anchors.begin(); i != m_anchors.end(); ++i)
		clothEntity->setNodeInvMass(i->x, i->y, 0.0f);

	return clothEntity;
}

void ClothComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void ClothComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< uint32_t >(L"resolutionX", m_resolutionX);
	s >> Member< uint32_t >(L"resolutionY", m_resolutionY);
	s >> Member< float >(L"scale", m_scale);
	s >> MemberStlVector< Anchor, MemberComposite< Anchor > >(L"anchors", m_anchors);
	s >> Member< uint32_t >(L"solverIterations", m_solverIterations);
	s >> Member< float >(L"damping", m_damping);
}

void ClothComponentData::Anchor::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"x", x);
	s >> Member< uint32_t >(L"y", y);
}

	}
}
