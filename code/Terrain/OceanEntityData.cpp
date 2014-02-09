#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/OceanEntityData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.OceanEntityData", 6, OceanEntityData, world::EntityData)

OceanEntityData::OceanEntityData()
:	m_shallowTint(1.0f, 1.0f, 1.0f, 1.0f)
,	m_reflectionTint(1.0f, 1.0f, 1.0f, 1.0f)
,	m_deepColor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_opacity(0.04f)
,	m_allowSSReflections(true)
{
}

void OceanEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);

	if (s.getVersion() >= 1)
	{
		s >> resource::Member< render::Shader >(L"shader", m_shader);

		if (s.getVersion() >= 2)
		{
			s >> resource::Member< render::ITexture >(L"reflectionMap", m_reflectionMap);
			s >> Member< Color4f >(L"shallowTint", m_shallowTint);
			s >> Member< Color4f >(L"reflectionTint", m_reflectionTint);
			s >> Member< Color4f >(L"deepColor", m_deepColor);
		}

		if (s.getVersion() >= 3)
			s >> Member< float >(L"opacity", m_opacity);

		if (s.getVersion() >= 4)
			s >> Member< bool >(L"allowSSReflections", m_allowSSReflections);

		s >> MemberStaticArray< Wave, 4, MemberComposite< Wave > >(L"waves", m_waves);
	}
	else
		s >> resource::Member< render::Shader >(L"shaderComposite", m_shader);
}

OceanEntityData::Wave::Wave()
:	center(0.0f, 0.0f)
,	amplitude(0.0f)
,	frequency(0.25f)
,	phase(0.0f)
,	pinch(3.0f)
,	rate(4.0f)
{
}

void OceanEntityData::Wave::serialize(ISerializer& s)
{
	s >> Member< Vector2 >(L"center", center);
	s >> Member< float >(L"amplitude", amplitude);
	s >> Member< float >(L"frequency", frequency);
	s >> Member< float >(L"phase", phase);

	if (s.getVersion() >= 5)
		s >> Member< float >(L"pinch", pinch);

	if (s.getVersion() >= 6)
		s >> Member< float >(L"rate", rate);
}

	}
}
