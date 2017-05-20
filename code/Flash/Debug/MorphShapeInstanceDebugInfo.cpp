/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/FlashMorphShapeInstance.h"
#include "Flash/Debug/MorphShapeInstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.MorphShapeInstanceDebugInfo", 0, MorphShapeInstanceDebugInfo, InstanceDebugInfo)

MorphShapeInstanceDebugInfo::MorphShapeInstanceDebugInfo()
{
}

MorphShapeInstanceDebugInfo::MorphShapeInstanceDebugInfo(const FlashMorphShapeInstance* instance)
{
	m_name = instance->getName();
	m_bounds = instance->getBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
}

void MorphShapeInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);
}

	}
}
