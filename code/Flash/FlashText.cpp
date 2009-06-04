#include <sstream>
#include "Flash/FlashText.h"
#include "Flash/FlashTextInstance.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashText", FlashText, FlashCharacter)

FlashText::FlashText(uint16_t id, const SwfRect& textBounds, const Matrix33& textMatrix)
:	FlashCharacter(id)
,	m_textBounds(textBounds)
,	m_textMatrix(textMatrix)
{
}

bool FlashText::create(const AlignedVector< SwfTextRecord* >& textRecords)
{
	Character character = { 0, 0, 0, 0, { 255, 255, 255, 255 }, 0 };
	for (AlignedVector< SwfTextRecord* >::const_iterator i = textRecords.begin(); i != textRecords.end(); ++i)
	{
		const SwfTextRecord* textRecord = *i;
		if (textRecord->styleFlag)
		{
			if (textRecord->style.hasFont)
				character.fontId = textRecord->style.fontId;
			if (textRecord->style.hasColor)
				character.color = textRecord->style.color;
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

const AlignedVector< FlashText::Character >& FlashText::getCharacters() const
{
	return m_characters;
}

FlashCharacterInstance* FlashText::createInstance(ActionContext* context, FlashCharacterInstance* parent) const
{
	return gc_new< FlashTextInstance >(context, parent, this);
}

const SwfRect& FlashText::getTextBounds() const
{
	return m_textBounds;
}

const Matrix33& FlashText::getTextMatrix() const
{
	return m_textMatrix;
}

	}
}
