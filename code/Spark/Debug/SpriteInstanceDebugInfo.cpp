/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Spark/Frame.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Debug/SpriteInstanceDebugInfo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SpriteInstanceDebugInfo", 0, SpriteInstanceDebugInfo, InstanceDebugInfo)

SpriteInstanceDebugInfo::SpriteInstanceDebugInfo(const SpriteInstance* instance, const std::string& className, bool mask, bool clipped, const RefArray< InstanceDebugInfo >& childrenDebugInfo)
{
	m_name = instance->getName();
	m_bounds = instance->getVisibleLocalBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
	m_className = className;
	m_mask = mask;
	m_clipped = clipped;
	m_frames = instance->getSprite()->getFrameCount();
	m_currentFrame = instance->getCurrentFrame();
	m_playing = instance->getPlaying();
	m_scalingGrid = instance->getSprite()->getScalingGrid();

	for (uint32_t i = 0; i < instance->getSprite()->getFrameCount(); ++i)
	{
		const Frame* frame = instance->getSprite()->getFrame(i);
		if (!frame->getLabel().empty())
			m_labels.push_back(std::make_pair(i, frame->getLabel()));
	}

	m_childrenDebugInfo = childrenDebugInfo;
}

void SpriteInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);

	s >> Member< std::string >(L"className", m_className);
	s >> Member< bool >(L"mask", m_mask);
	s >> Member< bool >(L"clipped", m_clipped);
	s >> Member< uint16_t >(L"frames", m_frames);
	s >> Member< uint16_t >(L"currentFrame", m_currentFrame);
	s >> Member< bool >(L"playing", m_playing);
	s >> MemberAabb2(L"scalingGrid", m_scalingGrid);
	s >> MemberAlignedVector< std::pair< uint32_t, std::string >, MemberStlPair< uint32_t, std::string > >(L"labels", m_labels);
	s >> MemberRefArray< InstanceDebugInfo >(L"childrenDebugInfo", m_childrenDebugInfo);
}

	}
}
