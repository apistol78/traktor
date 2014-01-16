#include "Amalgam/Engine/GameEntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "World/EntityEventSetData.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.GameEntityData", 0, GameEntityData, world::EntityData)

void GameEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);

	s >> Member< std::wstring >(L"tag", m_tag);
	s >> MemberRef< ISerializable >(L"object", m_object);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
	s >> MemberRef< world::EntityEventSetData >(L"eventSetData", m_eventSetData);
}

	}
}
