/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/FlashButtonInstance.h"
#include "Flash/Debug/ButtonInstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.ButtonInstanceDebugInfo", 0, ButtonInstanceDebugInfo, InstanceDebugInfo)

ButtonInstanceDebugInfo::ButtonInstanceDebugInfo()
{
}

ButtonInstanceDebugInfo::ButtonInstanceDebugInfo(const FlashButtonInstance* instance)
{
	m_name = instance->getName();
	m_bounds = instance->getLocalBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
}

void ButtonInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);
}

	}
}
