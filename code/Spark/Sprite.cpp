/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Spark/Context.h"
#include "Spark/Dictionary.h"
#include "Spark/Frame.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.Sprite", 1, Sprite, Character)

Sprite::Sprite(uint16_t frameRate)
:	m_frameRate(frameRate)
{
}

uint16_t Sprite::getFrameRate() const
{
	return m_frameRate;
}

void Sprite::addFrame(Frame* frame)
{
	m_frames.push_back(frame);
}

uint32_t Sprite::getFrameCount() const
{
	return uint32_t(m_frames.size());
}

Frame* Sprite::getFrame(uint32_t frameId) const
{
	return frameId < m_frames.size() ? m_frames[frameId] : nullptr;
}

int Sprite::findFrame(const std::string& frameLabel) const
{
	for (RefArray< Frame >::const_iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		if (compareIgnoreCase((*i)->getLabel(), frameLabel) == 0)
			return int(std::distance(m_frames.begin(), i));
	}
	return -1;
}

void Sprite::setScalingGrid(const Aabb2& scalingGrid)
{
	m_scalingGrid = scalingGrid;
}

const Aabb2& Sprite::getScalingGrid() const
{
	return m_scalingGrid;
}

Ref< CharacterInstance > Sprite::createInstance(
	Context* context,
	Dictionary* dictionary,
	CharacterInstance* parent,
	const std::string& name,
	const Matrix33& transform
) const
{
	Ref< SpriteInstance > spriteInstance = new SpriteInstance(context, dictionary, parent, this);

	if (!name.empty())
		spriteInstance->setName(name);

	spriteInstance->setTransform(transform);
	spriteInstance->updateDisplayList();

	return spriteInstance;
}

void Sprite::serialize(ISerializer& s)
{
	Character::serialize(s);

	s >> Member< uint16_t >(L"frameRate", m_frameRate);
	s >> MemberRefArray< Frame >(L"frames", m_frames);
	s >> MemberAabb2(L"scalingGrid", m_scalingGrid);
}

	}
}
