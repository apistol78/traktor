/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Flash/Debug/InstanceDebugInfo.h"
#include "Flash/Debug/PostFrameDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.PostFrameDebugInfo", 0, PostFrameDebugInfo, ISerializable)

PostFrameDebugInfo::PostFrameDebugInfo()
{
}

PostFrameDebugInfo::PostFrameDebugInfo(const Aabb2& frameBounds, const RefArray< InstanceDebugInfo >& instances)
:	m_frameBounds(frameBounds)
,	m_instances(instances)
{
}

void PostFrameDebugInfo::serialize(ISerializer& s)
{
	s >> MemberAabb2(L"frameBounds", m_frameBounds);
	s >> MemberRefArray< InstanceDebugInfo >(L"instances", m_instances);
}

	}
}
