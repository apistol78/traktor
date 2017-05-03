/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Debug/SpriteInstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.SpriteInstanceDebugInfo", 0, SpriteInstanceDebugInfo, InstanceDebugInfo)

SpriteInstanceDebugInfo::SpriteInstanceDebugInfo()
{
}

SpriteInstanceDebugInfo::SpriteInstanceDebugInfo(const FlashSpriteInstance* instance)
{
	m_name = instance->getName();
	m_bounds = instance->getLocalBounds();
	m_transform = instance->getFullTransform();
}

void SpriteInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);
}

	}
}
