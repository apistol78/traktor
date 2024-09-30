/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeHdr.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "World/Entity/FogComponent.h"
#include "World/Entity/FogComponentData.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::Shader > c_defaultShader(Guid(L"{FEDA90CE-25C6-BC4D-9767-EA4B45F4A043}"));

	}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.FogComponentData", 0, FogComponentData, IEntityComponentData)

Ref< FogComponent > FogComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (m_volumetricFogEnable)
	{
		if (!resourceManager->bind(c_defaultShader, shader))
			return nullptr;
	}

	Ref< FogComponent > component = new FogComponent(this, shader);
	if (component->create(renderSystem))
		return component;
	else
		return nullptr;
}

const resource::Id< render::Shader >& FogComponentData::getShader() const
{
	return c_defaultShader;
}

int32_t FogComponentData::getOrdinal() const
{
	return 0;
}

void FogComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void FogComponentData::serialize(ISerializer& s)
{
	s >> Member< float >(L"fogDistance", m_fogDistance, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"fogDensity", m_fogDensity, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	s >> Member< float >(L"fogDensityMax", m_fogDensityMax, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	s >> Member< Color4f >(L"fogColor", m_fogColor, AttributeHdr());

	s >> Member< bool >(L"volumetricFogEnable", m_volumetricFogEnable);
	s >> Member< float >(L"maxDistance", m_maxDistance, AttributeRange(0.0f));
	s >> Member< float >(L"maxScattering", m_maxScattering, AttributeRange(0.0f));
	s >> Member< int32_t >(L"sliceCount", m_sliceCount, AttributeRange(1));
	s >> Member< Color4f >(L"mediumColor", m_mediumColor);
	s >> Member< float >(L"mediumDensity", m_mediumDensity, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
}

}
