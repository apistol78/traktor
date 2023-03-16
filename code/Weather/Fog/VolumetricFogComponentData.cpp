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
#include "Weather/Fog/VolumetricFogComponent.h"
#include "Weather/Fog/VolumetricFogComponentData.h"

namespace traktor::weather
{
	namespace
	{

const resource::Id< render::Shader > c_defaultShader(Guid(L"{4CF929EB-3A8B-C340-AA0A-0C5C80625BF1}"));

	}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.VolumetricFogComponentData", 1, VolumetricFogComponentData, world::IEntityComponentData)

VolumetricFogComponentData::VolumetricFogComponentData()
:	m_shader(c_defaultShader)
{
}

Ref< VolumetricFogComponent > VolumetricFogComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

	Ref< VolumetricFogComponent > component = new VolumetricFogComponent(shader, m_maxDistance, m_sliceCount);
	if (component->create(renderSystem))
		return component;
	else
		return nullptr;
}

int32_t VolumetricFogComponentData::getOrdinal() const
{
	return 0;
}

void VolumetricFogComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void VolumetricFogComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);

	if (s.getVersion< VolumetricFogComponentData >() >= 1)
	{
		s >> Member< float >(L"maxDistance", m_maxDistance);
		s >> Member< int32_t >(L"sliceCount", m_sliceCount);
	}
}

}
