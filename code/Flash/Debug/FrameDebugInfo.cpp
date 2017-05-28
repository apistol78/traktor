/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Flash/Debug/FrameDebugInfo.h"
#include "Flash/Debug/InstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FrameDebugInfo", 0, FrameDebugInfo, ISerializable)

FrameDebugInfo::FrameDebugInfo()
{
}

FrameDebugInfo::FrameDebugInfo(
	const Aabb2& frameBounds,
	const Vector4& stageTransform,
	int32_t viewWidth,
	int32_t viewHeight,
	const RefArray< InstanceDebugInfo >& instances
)
:	m_frameBounds(frameBounds)
,	m_stageTransform(stageTransform)
,	m_viewWidth(viewWidth)
,	m_viewHeight(viewHeight)
,	m_instances(instances)
{
}

void FrameDebugInfo::serialize(ISerializer& s)
{
	s >> MemberAabb2(L"frameBounds", m_frameBounds);
	s >> Member< Vector4 >(L"stageTransform", m_stageTransform);
	s >> Member< int32_t >(L"viewWidth", m_viewWidth);
	s >> Member< int32_t >(L"viewHeight", m_viewHeight);
	s >> MemberRefArray< InstanceDebugInfo >(L"instances", m_instances);
}

	}
}
