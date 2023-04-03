/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "World/Entity/VolumetricFogComponent.h"
#include "World/Entity/VolumetricFogComponentData.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::Shader > c_defaultShader(Guid(L"{FEDA90CE-25C6-BC4D-9767-EA4B45F4A043}"));

	}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.VolumetricFogComponentData", 0, VolumetricFogComponentData, IEntityComponentData)

VolumetricFogComponentData::VolumetricFogComponentData()
:	m_shader(c_defaultShader)
{
}

Ref< VolumetricFogComponent > VolumetricFogComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

	Ref< VolumetricFogComponent > component = new VolumetricFogComponent(shader, m_maxDistance, m_sliceCount, m_mediumColor);
	if (component->create(renderSystem))
		return component;
	else
		return nullptr;
}

int32_t VolumetricFogComponentData::getOrdinal() const
{
	return 0;
}

void VolumetricFogComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void VolumetricFogComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< float >(L"maxDistance", m_maxDistance);
	s >> Member< int32_t >(L"sliceCount", m_sliceCount);
	s >> Member< Color4f >(L"mediumColor", m_mediumColor);
}

}
