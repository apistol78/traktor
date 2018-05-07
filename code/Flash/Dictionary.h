/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_Dictionary_H
#define traktor_flash_Dictionary_H

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"

namespace traktor
{
	namespace flash
	{

class Bitmap;
class Character;
class Font;
class Sound;

/*! \brief Flash character dictionary.
 * \ingroup Flash
 */
class Dictionary : public Object
{
	T_RTTI_CLASS;

public:
	uint16_t addFont(Font* font);

	uint16_t addBitmap(Bitmap* bitmap);

	uint16_t addSound(Sound* sound);

	uint16_t addCharacter(Character* character);

	const Font* getFont(uint16_t fontId) const;

	const Bitmap* getBitmap(uint16_t bitmapId) const;

	const Sound* getSound(uint16_t soundId) const;

	const Character* getCharacter(uint16_t characterId) const;

	bool getExportId(const std::string& name, uint16_t& outExportId) const;

	bool getExportName(uint16_t exportId, std::string& outName) const;

	const SmallMap< uint16_t, Ref< Font > >& getFonts() const { return m_fonts; }

	const SmallMap< uint16_t, Ref< Bitmap > >& getBitmaps() const { return m_bitmaps; }

private:
	friend class Movie;

	SmallMap< uint16_t, Ref< Font > > m_fonts;
	SmallMap< uint16_t, Ref< Bitmap > > m_bitmaps;
	SmallMap< uint16_t, Ref< Sound > > m_sounds;
	SmallMap< uint16_t, Ref< Character > > m_characters;
	SmallMap< std::string, uint16_t > m_exports;
};

	}
}

#endif	// traktor_flash_Dictionary_H
