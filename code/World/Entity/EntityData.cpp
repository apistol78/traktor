#include "World/Entity/EntityData.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityData", EntityData, Serializable)

void EntityData::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& EntityData::getName() const
{
	return m_name;
}

bool EntityData::serialize(Serializer& s)
{
	return s >> Member< std::wstring >(L"name", m_name);
}

	}
}
