/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Spark/Dictionary.h"

namespace traktor
{
	namespace spark
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Dictionary", Dictionary, Object)

uint16_t Dictionary::addFont(Font* font)
{
	uint16_t fontId = getMaximumKey(m_fonts) + 1;
	m_fonts[fontId] = font;
	return fontId;
}

uint16_t Dictionary::addBitmap(Bitmap* bitmap)
{
	uint16_t bitmapId = getMaximumKey(m_bitmaps) + 1;
	m_bitmaps[bitmapId] = bitmap;
	return bitmapId;
}

uint16_t Dictionary::addSound(Sound* sound)
{
	uint16_t soundId = getMaximumKey(m_sounds) + 1;
	m_sounds[soundId] = sound;
	return soundId;
}

uint16_t Dictionary::addCharacter(Character* character)
{
	uint16_t characterId = getMaximumKey(m_characters) + 1;
	m_characters[characterId] = character;
	return characterId;
}

const Font* Dictionary::getFont(uint16_t fontId) const
{
	auto it = m_fonts.find(fontId);
	return it != m_fonts.end() ? it->second.ptr() : nullptr;
}

const Bitmap* Dictionary::getBitmap(uint16_t bitmapId) const
{
	auto it = m_bitmaps.find(bitmapId);
	return it != m_bitmaps.end() ? it->second.ptr() : nullptr;
}

const Sound* Dictionary::getSound(uint16_t soundId) const
{
	auto it = m_sounds.find(soundId);
	return it != m_sounds.end() ? it->second.ptr() : nullptr;
}

const Character* Dictionary::getCharacter(uint16_t characterId) const
{
	auto it = m_characters.find(characterId);
	return it != m_characters.end() ? it->second.ptr() : nullptr;
}

bool Dictionary::getExportId(const std::string& name, uint16_t& outExportId) const
{
	auto it = m_exports.find(name);
	if (it != m_exports.end())
	{
		outExportId = it->second;
		return true;
	}
	return false;
}

bool Dictionary::getExportName(uint16_t exportId, std::string& outName) const
{
	for (const auto& exp : m_exports)
	{
		if (exp.second == exportId)
		{
			outName = exp.first;
			return true;
		}
	}
	return false;
}

	}
}
