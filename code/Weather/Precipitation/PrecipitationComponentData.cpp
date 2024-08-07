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
#include "Mesh/Static/StaticMesh.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "Weather/Precipitation/PrecipitationComponentData.h"

namespace traktor::weather
{
	namespace
	{
		
const resource::Id< mesh::StaticMesh > c_defaultMesh(Guid(L"{C07C9EBA-22DF-2148-B045-34BD4F84AC09}"));
		
	}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.PrecipitationComponentData", 0, PrecipitationComponentData, world::IEntityComponentData)

PrecipitationComponentData::PrecipitationComponentData()
:	m_mesh(c_defaultMesh)
{
}

Ref< PrecipitationComponent > PrecipitationComponentData::createComponent(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< mesh::StaticMesh > mesh;
	if (resourceManager->bind(m_mesh, mesh))
		return new PrecipitationComponent(mesh, m_tiltRate, m_parallaxDistance, m_depthDistance, m_opacity);
	else
		return nullptr;
}

int32_t PrecipitationComponentData::getOrdinal() const
{
	return 0;
}

void PrecipitationComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void PrecipitationComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< mesh::StaticMesh >(L"mesh", m_mesh);
	s >> Member< float >(L"tiltRate", m_tiltRate, AttributeRange(0.0f));
	s >> Member< float >(L"parallaxDistance", m_parallaxDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"depthDistance", m_depthDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"opacity", m_opacity, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));
}

}
