/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "World/Entity/VolumeComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.VolumeComponentData", 0, VolumeComponentData, IEntityComponentData)

void VolumeComponentData::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Aabb3, MemberAabb3 >(L"volumes", m_volumes);
}

	}
}
