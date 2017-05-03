/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Flash/FlashCharacterInstance.h"
#include "Flash/Debug/InstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.InstanceDebugInfo", InstanceDebugInfo, ISerializable)

InstanceDebugInfo::InstanceDebugInfo()
{
}

void InstanceDebugInfo::serialize(ISerializer& s)
{
	s >> Member< std::string >(L"name", m_name);
	s >> MemberAabb2(L"bounds", m_bounds);
	s >> Member< Matrix33 >(L"transform", m_transform);
}

	}
}
