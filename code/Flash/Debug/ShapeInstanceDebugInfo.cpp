/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashShapeInstance.h"
#include "Flash/Debug/ShapeInstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.ShapeInstanceDebugInfo", 0, ShapeInstanceDebugInfo, InstanceDebugInfo)

ShapeInstanceDebugInfo::ShapeInstanceDebugInfo()
{
}

ShapeInstanceDebugInfo::ShapeInstanceDebugInfo(const FlashShapeInstance* instance)
{
	m_name = instance->getName();
	m_bounds = instance->getBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_shape = instance->getShape();
}

void ShapeInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);

	s >> MemberRef< const FlashShape >(L"shape", m_shape);
}

	}
}
