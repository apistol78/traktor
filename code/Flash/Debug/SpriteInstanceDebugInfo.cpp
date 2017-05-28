/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/Debug/SpriteInstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.SpriteInstanceDebugInfo", 0, SpriteInstanceDebugInfo, InstanceDebugInfo)

SpriteInstanceDebugInfo::SpriteInstanceDebugInfo()
{
}

SpriteInstanceDebugInfo::SpriteInstanceDebugInfo(const FlashSpriteInstance* instance, const std::string& className, const RefArray< InstanceDebugInfo >& childrenDebugInfo)
{
	m_name = instance->getName();
	m_bounds = instance->getVisibleLocalBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
	m_className = className;
	m_frames = instance->getSprite()->getFrameCount();
	m_currentFrame = instance->getCurrentFrame();
	m_playing = instance->getPlaying();
	m_childrenDebugInfo = childrenDebugInfo;
}

void SpriteInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);

	s >> Member< std::string >(L"className", m_className);
	s >> Member< uint16_t >(L"frames", m_frames);
	s >> Member< uint16_t >(L"currentFrame", m_currentFrame);
	s >> Member< bool >(L"playing", m_playing);
	s >> MemberRefArray< InstanceDebugInfo >(L"childrenDebugInfo", m_childrenDebugInfo);
}

	}
}
