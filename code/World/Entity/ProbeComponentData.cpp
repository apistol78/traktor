#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"
#include "World/Entity/ProbeComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ProbeComponentData", 0, ProbeComponentData, IEntityComponentData)

ProbeComponentData::ProbeComponentData()
{
}

void ProbeComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::ITexture >(L"probeDiffuseTexture", m_probeDiffuseTexture);
	s >> resource::Member< render::ITexture >(L"probeSpecularTexture", m_probeSpecularTexture);
	s >> MemberAabb3(L"volume", m_volume);
}

	}
}
