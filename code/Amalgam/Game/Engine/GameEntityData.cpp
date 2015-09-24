#include "Amalgam/Game/Engine/GameEntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/IEntityComponentData.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.GameEntityData", 0, GameEntityData, world::EntityData)

void GameEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);
	s >> MemberRefArray< world::IEntityComponentData >(L"components", m_components);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
}

	}
}
