/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/OceanComponentData.h"
#include "Terrain/Terrain.h"

namespace traktor::terrain
{
	namespace
	{
	
const resource::Id< render::Shader > c_waveShader(Guid(L"{3ABA4673-C5A5-404E-8563-CE8A6E043AC4}"));
const resource::Id< render::Shader > c_defaultShader(Guid(L"{FB9B7138-B7B2-E341-82EB-453BE2B558A8}"));
	
	}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.OceanComponentData", 3, OceanComponentData, world::IEntityComponentData)

OceanComponentData::OceanComponentData()
:	m_shaderWave(c_waveShader)
,	m_shader(c_defaultShader)
,	m_shallowTint(1.0f, 1.0f, 1.0f, 1.0f)
,	m_deepColor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_elevation(0.0f)
,	m_opacity(0.04f)
{
}

int32_t OceanComponentData::getOrdinal() const
{
	return 0;
}

void OceanComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void OceanComponentData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 2)
		s >> resource::Member< render::Shader >(L"shaderWave", m_shaderWave);

	s >> resource::Member< render::Shader >(L"shader", m_shader);

	if (s.getVersion() >= 1)
		s >> resource::Member< render::ITexture >(L"reflectionTexture", m_reflectionTexture);

	s >> Member< Color4f >(L"shallowTint", m_shallowTint);

	if (s.getVersion() < 3)
	{
		s >> ObsoleteMember< Color4f >(L"reflectionTint");
		s >> ObsoleteMember< Color4f >(L"shadowTint");
	}

	s >> Member< Color4f >(L"deepColor", m_deepColor);
	s >> Member< float >(L"opacity", m_opacity);
	s >> Member< float >(L"elevation", m_elevation);

	if (s.getVersion() < 3)
	{
		Wave waves[4];
		s >> MemberStaticArray< Wave, 4, MemberComposite< Wave > >(L"waves", waves);
	}
}

void OceanComponentData::Wave::serialize(ISerializer& s)
{
	s >> Member< float >(L"direction", direction);
	s >> Member< float >(L"amplitude", amplitude);
	s >> Member< float >(L"frequency", frequency);
	s >> Member< float >(L"phase", phase);
	s >> Member< float >(L"pinch", pinch);
	s >> Member< float >(L"rate", rate);
}

}
