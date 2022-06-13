#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/OceanComponentData.h"
#include "Terrain/Terrain.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{
		
const resource::Id< render::Shader > c_defaultShader(Guid(L"{FB9B7138-B7B2-E341-82EB-453BE2B558A8}"));
		
		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.OceanComponentData", 1, OceanComponentData, world::IEntityComponentData)

OceanComponentData::OceanComponentData()
:	m_shader(c_defaultShader)
,	m_shallowTint(1.0f, 1.0f, 1.0f, 1.0f)
,	m_reflectionTint(1.0f, 1.0f, 1.0f, 1.0f)
,	m_shadowTint(1.0f, 1.0f, 1.0f, 1.0f)
,	m_deepColor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_elevation(0.0f)
,	m_opacity(0.04f)
{
}

void OceanComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void OceanComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);

	if (s.getVersion() >= 1)
		s >> resource::Member< render::ITexture >(L"reflectionTexture", m_reflectionTexture);

	s >> Member< Color4f >(L"shallowTint", m_shallowTint);
	s >> Member< Color4f >(L"reflectionTint", m_reflectionTint);
	s >> Member< Color4f >(L"shadowTint", m_shadowTint);
	s >> Member< Color4f >(L"deepColor", m_deepColor);
	s >> Member< float >(L"opacity", m_opacity);
	s >> Member< float >(L"elevation", m_elevation);
	s >> MemberStaticArray< Wave, 4, MemberComposite< Wave > >(L"waves", m_waves);
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
}
