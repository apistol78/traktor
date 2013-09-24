#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "World/Entity/VolumeEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.VolumeEntityData", 0, VolumeEntityData, EntityData)

void VolumeEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);
	s >> MemberAlignedVector< Aabb3, MemberAabb3 >(L"volumes", m_volumes);
}

	}
}
