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
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Core/Serialization/MemberStl.h"
#include "Spark/Frame.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.Frame", 0, Frame, ISerializable)

Frame::Frame()
:	m_backgroundColorChange(false)
,	m_backgroundColor(1.0f, 1.0f, 1.0f, 1.0f)
{
}

void Frame::setLabel(const std::string& label)
{
	m_label = label;
}

const std::string& Frame::getLabel() const
{
	return m_label;
}

void Frame::changeBackgroundColor(const Color4f& backgroundColor)
{
	m_backgroundColor = backgroundColor;
	m_backgroundColorChange = true;
}

bool Frame::hasBackgroundColorChanged() const
{
	return m_backgroundColorChange;
}

const Color4f& Frame::getBackgroundColor() const
{
	return m_backgroundColor;
}

uint16_t Frame::nextUnusedDepth() const
{
	uint16_t depth = 0;
	for (auto it : m_placeObjects)
		depth = max(depth, it.first);
	return depth + 1;
}

void Frame::placeObject(const PlaceObject& placeObject)
{
	T_ASSERT(placeObject.depth > 0);
	m_placeObjects[placeObject.depth] = placeObject;
}

void Frame::removeObject(const RemoveObject& removeObject)
{
	T_ASSERT(removeObject.depth > 0);
	m_removeObjects[removeObject.depth] = removeObject;
}

void Frame::startSound(uint16_t soundId)
{
	m_startSounds.push_back(soundId);
}

const SmallMap< uint16_t, Frame::PlaceObject >& Frame::getPlaceObjects() const
{
	return m_placeObjects;
}

const SmallMap< uint16_t, Frame::RemoveObject >& Frame::getRemoveObjects() const
{
	return m_removeObjects;
}

const AlignedVector< uint16_t >& Frame::getStartSounds() const
{
	return m_startSounds;
}

void Frame::serialize(ISerializer& s)
{
	s >> Member< std::string >(L"label", m_label);
	s >> Member< bool >(L"backgroundColorChange", m_backgroundColorChange);
	s >> Member< Color4f >(L"backgroundColor", m_backgroundColor);
	s >> MemberSmallMap< uint16_t, PlaceObject, Member< uint16_t >, MemberComposite< PlaceObject > >(L"placeObjects", m_placeObjects);
	s >> MemberSmallMap< uint16_t, RemoveObject, Member< uint16_t >, MemberComposite< RemoveObject > >(L"removeObjects", m_removeObjects);
	s >> MemberAlignedVector< uint16_t >(L"startSounds", m_startSounds);
}

void Frame::PlaceObject::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"hasFlags", hasFlags);
	s >> Member< uint16_t >(L"depth", depth);

	if (hasFlags & PfHasBitmapCaching)
		s >> Member< uint8_t >(L"bitmapCaching", bitmapCaching);

	if (hasFlags & PfHasBlendMode)
		s >> Member< uint8_t >(L"blendMode", blendMode);

	if (hasFlags & PfHasFilters)
	{
		s >> Member< uint8_t >(L"filter", filter);
		s >> Member< Color4f >(L"filterColor", filterColor);
	}

	if (hasFlags & PfHasClipDepth)
		s >> Member< uint16_t >(L"clipDepth", clipDepth);

	if (hasFlags & PfHasName)
		s >> Member< std::string >(L"name", name);

	if (hasFlags & PfHasRatio)
		s >> Member< uint16_t >(L"ratio", ratio);

	if (hasFlags & PfHasCxTransform)
		s >> MemberComposite< ColorTransform >(L"cxTransform", cxTransform);

	if (hasFlags & PfHasMatrix)
		s >> Member< Matrix33 >(L"matrix", matrix);

	if (hasFlags & PfHasCharacterId)
		s >> Member< uint16_t >(L"characterId", characterId);
}

void Frame::RemoveObject::serialize(ISerializer& s)
{
	s >> Member< bool >(L"hasCharacterId", hasCharacterId);
	s >> Member< uint16_t >(L"depth", depth);

	if (hasCharacterId)
		s >> Member< uint16_t >(L"characterId", characterId);
}

	}
}
