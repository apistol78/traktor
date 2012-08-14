#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/Terrain.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.OceanEntityData", 0, OceanEntityData, world::EntityData)

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

bool OceanEntityData::serialize(ISerializer& s)
{
	if (!world::EntityData::serialize(s))
		return false;

	s >> resource::Member< Terrain >(L"terrain", m_terrain);
	s >> resource::Member< render::Shader >(L"shaderWaves", m_shaderWaves);
	s >> resource::Member< render::Shader >(L"shaderComposite", m_shaderComposite);
	s >> MemberStaticArray< Wave, MaxWaves, MemberComposite< Wave > >(L"waves", m_waves);

	return true;
}

bool OceanEntityData::Wave::serialize(ISerializer& s)
{
	s >> Member< Vector2 >(L"direction", direction);
	s >> Member< float >(L"amplitude", amplitude);
	s >> Member< float >(L"phase", phase);
	return true;
}

	}
}
