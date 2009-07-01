#include "Terrain/OceanEntityData.h"
#include "Terrain/Heightfield.h"
#include "Terrain/HeightfieldResource.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.terrain.OceanEntityData", OceanEntityData, world::EntityData)

OceanEntityData::OceanEntityData()
:	m_altitude(0.0f)
{
}

void OceanEntityData::setWave(int index, const Wave& wave)
{
	T_ASSERT (index >= 0 && index < MaxWaves);
	m_waves[index] = wave;
}

const OceanEntityData::Wave& OceanEntityData::getWave(int index) const
{
	T_ASSERT (index >= 0 && index < MaxWaves);
	return m_waves[index];
}

bool OceanEntityData::serialize(Serializer& s)
{
	s >> resource::Member< Heightfield, HeightfieldResource >(L"heightfield", m_heightfield);
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> Member< float >(L"altitude", m_altitude);
	s >> MemberStaticArray< Wave, MaxWaves, MemberComposite< Wave > >(L"waves", m_waves);
	return true;
}

bool OceanEntityData::Wave::serialize(Serializer& s)
{
	s >> Member< Vector2 >(L"direction", direction);
	s >> Member< float >(L"amplitude", amplitude);
	s >> Member< float >(L"phase", phase);
	return true;
}

	}
}
