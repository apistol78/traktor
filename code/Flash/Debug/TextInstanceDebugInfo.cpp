/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Text.h"
#include "Flash/TextInstance.h"
#include "Flash/Debug/TextInstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.TextInstanceDebugInfo", 0, TextInstanceDebugInfo, InstanceDebugInfo)

TextInstanceDebugInfo::TextInstanceDebugInfo()
{
}

TextInstanceDebugInfo::TextInstanceDebugInfo(const TextInstance* instance)
{
	m_name = instance->getName();
	m_bounds = instance->getText()->getTextBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
}

void TextInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);
}

	}
}
