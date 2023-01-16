/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Weather/Clouds/CloudComponent.h"
#include "Weather/Clouds/CloudComponentData.h"
#include "Weather/Clouds/CloudMask.h"
#include "Weather/Clouds/CloudMaskResource.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.CloudComponentData", 0, CloudComponentData, world::IEntityComponentData)

CloudComponentData::CloudComponentData()
:	m_impostorTargetResolution(256)
,	m_impostorSliceCount(1)
,	m_updateFrequency(10)
,	m_updatePositionThreshold(0.1f)
,	m_updateDirectionThreshold(0.1f)
{
}

Ref< CloudComponent > CloudComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > particleShader;
	resource::Proxy< render::ITexture > particleTexture;
	resource::Proxy< render::Shader > impostorShader;
	resource::Proxy< CloudMask > mask;

	if (!resourceManager->bind(m_particleShader, particleShader))
		return nullptr;
	if (!resourceManager->bind(m_particleTexture, particleTexture))
		return nullptr;
	if (!resourceManager->bind(m_impostorShader, impostorShader))
		return nullptr;

	if (m_mask)
	{
		if (!resourceManager->bind(m_mask, mask))
			return nullptr;
	}

	Ref< CloudComponent > cloudComponent = new CloudComponent();
	if (!cloudComponent->create(
		renderSystem,
		particleShader,
		particleTexture,
		impostorShader,
		mask,
		m_impostorTargetResolution,
		m_impostorSliceCount,
		m_updateFrequency,
		m_updatePositionThreshold,
		m_updateDirectionThreshold,
		m_particleData
	))
		return nullptr;

	return cloudComponent;
}

int32_t CloudComponentData::getOrdinal() const
{
	return 0;
}

void CloudComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void CloudComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"particleShader", m_particleShader);
	s >> resource::Member< render::ITexture >(L"particleTexture", m_particleTexture);
	s >> resource::Member< render::Shader >(L"impostorShader", m_impostorShader);
	s >> resource::Member< CloudMask >(L"mask", m_mask);
	s >> Member< uint32_t >(L"impostorTargetResolution", m_impostorTargetResolution, AttributeUnit(UnitType::Pixels));
	s >> Member< uint32_t >(L"impostorSliceCount", m_impostorSliceCount);
	s >> Member< uint32_t >(L"updateFrequency", m_updateFrequency, AttributeUnit(UnitType::Hertz));
	s >> Member< float >(L"updatePositionThreshold", m_updatePositionThreshold, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"updateDirectionThreshold", m_updateDirectionThreshold, AttributeRange(0.0f, 180.0f) | AttributeUnit(UnitType::Metres));
	s >> MemberComposite< CloudParticleData >(L"particleData", m_particleData);
}

	}
}
