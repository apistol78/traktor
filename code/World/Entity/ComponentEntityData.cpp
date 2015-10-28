#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/IEntityComponentData.h"
#include "World/Entity/ComponentEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ComponentEntityData", 0, ComponentEntityData, EntityData)

void ComponentEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);
	s >> MemberRefArray< IEntityComponentData >(L"components", m_components);
}

	}
}
