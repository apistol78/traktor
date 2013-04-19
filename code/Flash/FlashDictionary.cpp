#include "Flash/FlashDictionary.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

template < typename ContainerType >
typename ContainerType::pair_t::first_type getMaximumKey(const ContainerType& container)
{
	typename ContainerType::pair_t::first_type mx = 0;
	for (typename ContainerType::const_iterator i = container.begin(); i != container.end(); ++i)
		mx = std::max(mx, i->first);
	return mx;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashDictionary", FlashDictionary, Object)

uint16_t FlashDictionary::addFont(FlashFont* font)
{
	uint16_t fontId = getMaximumKey(m_fonts) + 1;
	m_fonts[fontId] = font;
	return fontId;
}

uint16_t FlashDictionary::addBitmap(FlashBitmap* bitmap)
{
	uint16_t bitmapId = getMaximumKey(m_bitmaps) + 1;
	m_bitmaps[bitmapId] = bitmap;
	return bitmapId;
}

uint16_t FlashDictionary::addSound(FlashSound* sound)
{
	uint16_t soundId = getMaximumKey(m_sounds) + 1;
	m_sounds[soundId] = sound;
	return soundId;
}

uint16_t FlashDictionary::addCharacter(FlashCharacter* character)
{
	uint16_t characterId = getMaximumKey(m_characters) + 1;
	m_characters[characterId] = character;
	return characterId;
}

const FlashFont* FlashDictionary::getFont(uint16_t fontId) const
{
	SmallMap< uint16_t, Ref< FlashFont > >::const_iterator i = m_fonts.find(fontId);
	return i != m_fonts.end() ? i->second.ptr() : 0;
}

const FlashBitmap* FlashDictionary::getBitmap(uint16_t bitmapId) const
{
	SmallMap< uint16_t, Ref< FlashBitmap > >::const_iterator i = m_bitmaps.find(bitmapId);
	return i != m_bitmaps.end() ? i->second.ptr() : 0;
}

const FlashSound* FlashDictionary::getSound(uint16_t soundId) const
{
	SmallMap< uint16_t, Ref< FlashSound > >::const_iterator i = m_sounds.find(soundId);
	return i != m_sounds.end() ? i->second.ptr() : 0;
}

const FlashCharacter* FlashDictionary::getCharacter(uint16_t characterId) const
{
	SmallMap< uint16_t, Ref< FlashCharacter > >::const_iterator i = m_characters.find(characterId);
	return i != m_characters.end() ? i->second.ptr() : 0;
}

bool FlashDictionary::getExportId(const std::string& name, uint16_t& outExportId) const
{
	SmallMap< std::string, uint16_t >::const_iterator i = m_exports.find(name);
	if (i != m_exports.end())
	{
		outExportId = i->second;
		return true;
	}
	return false;
}

bool FlashDictionary::getExportName(uint16_t exportId, std::string& outName) const
{
	for (SmallMap< std::string, uint16_t >::const_iterator i = m_exports.begin(); i != m_exports.end(); ++i)
	{
		if (i->second == exportId)
		{
			outName = i->first;
			return true;
		}
	}
	return false;
}

	}
}
