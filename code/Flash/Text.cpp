/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sstream>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Flash/Text.h"
#include "Flash/TextInstance.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.Text", 0, Text, Character)

Text::Text()
:	m_textMatrix(Matrix33::identity())
{
}

Text::Text(uint16_t id, const Aabb2& textBounds, const Matrix33& textMatrix)
:	Character(id)
,	m_textBounds(textBounds)
,	m_textMatrix(textMatrix)
{
}

bool Text::create(const AlignedVector< SwfTextRecord* >& textRecords)
{
	Char character = { 0, 0, 0, 0, Color4f(1.0f, 1.0f, 1.0f, 1.0f), { 0 } };
	for (AlignedVector< SwfTextRecord* >::const_iterator i = textRecords.begin(); i != textRecords.end(); ++i)
	{
		const SwfTextRecord* textRecord = *i;
		if (textRecord->styleFlag)
		{
			if (textRecord->style.hasFont)
				character.fontId = textRecord->style.fontId;
			if (textRecord->style.hasColor)
				character.color = Color4f::loadUnaligned(textRecord->style.color);
			if (textRecord->style.hasXOffset)
				character.offsetX = textRecord->style.XOffset;
			if (textRecord->style.hasYOffset)
				character.offsetY = textRecord->style.YOffset;
			if (textRecord->style.hasFont)
				character.height = textRecord->style.height;
		}
		else
		{
			for (uint8_t j = 0; j < textRecord->glyph.glyphCount; ++j)
			{
				character.glyphIndex = textRecord->glyph.glyphEntries[j]->glyphIndex;
				m_characters.push_back(character);
				character.offsetX += textRecord->glyph.glyphEntries[j]->glyphAdvance;
			}
		}
	}
	return true;
}

const AlignedVector< Text::Char >& Text::getCharacters() const
{
	return m_characters;
}

Ref< CharacterInstance > Text::createInstance(
	ActionContext* context,
	Dictionary* dictionary,
	CharacterInstance* parent,
	const std::string& name,
	const Matrix33& transform,
	const ActionObject* initObject,
	const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
) const
{
	return new TextInstance(context, dictionary, parent, this);
}

const Aabb2& Text::getTextBounds() const
{
	return m_textBounds;
}

const Matrix33& Text::getTextMatrix() const
{
	return m_textMatrix;
}

void Text::serialize(ISerializer& s)
{
	Character::serialize(s);

	s >> MemberAabb2(L"textBounds", m_textBounds);
	s >> Member< Matrix33 >(L"textMatrix", m_textMatrix);
	s >> MemberAlignedVector< Char, MemberComposite< Char > >(L"characters", m_characters);
}

void Text::Char::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"fontId", fontId);
	s >> Member< int16_t >(L"offsetX", offsetX);
	s >> Member< int16_t >(L"offsetY", offsetY);
	s >> Member< int16_t >(L"height", height);
	s >> Member< Color4f >(L"color", color);
	s >> Member< uint32_t >(L"glyphIndexOrCode", glyphIndex);
}

	}
}
