#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/EntityData.h"
#include "World/Entity/FacadeComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.FacadeComponentData", 0, FacadeComponentData, GroupComponentData)

const std::wstring& FacadeComponentData::getShow() const
{
	return m_show;
}

void FacadeComponentData::serialize(ISerializer& s)
{
    GroupComponentData::serialize(s);
	s >> Member< std::wstring >(L"show", m_show);
}

	}
}
