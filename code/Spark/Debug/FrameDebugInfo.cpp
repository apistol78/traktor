/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Spark/Debug/FrameDebugInfo.h"
#include "Spark/Debug/InstanceDebugInfo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.FrameDebugInfo", 0, FrameDebugInfo, ISerializable)

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
