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
:	m_local(false)
,	m_volume(Vector4::zero(), Vector4::zero())
{
}

void ProbeComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::ITexture >(L"diffuseTexture", m_diffuseTexture);
	s >> resource::Member< render::ITexture >(L"specularTexture", m_specularTexture);
	s >> Member< bool >(L"local", m_local);
	s >> MemberAabb3(L"volume", m_volume);
}

	}
}
