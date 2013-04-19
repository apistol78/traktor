#ifndef traktor_flash_FlashDictionary_H
#define traktor_flash_FlashDictionary_H

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"

namespace traktor
{
	namespace flash
	{

class FlashBitmap;
class FlashCharacter;
class FlashFont;
class FlashSound;

/*! \brief Flash character dictionary.
 * \ingroup Flash
 */
class FlashDictionary : public Object
{
	T_RTTI_CLASS;

public:
	uint16_t addFont(FlashFont* font);

	uint16_t addBitmap(FlashBitmap* bitmap);

	uint16_t addSound(FlashSound* sound);

	uint16_t addCharacter(FlashCharacter* character);

	const FlashFont* getFont(uint16_t fontId) const;

	const FlashBitmap* getBitmap(uint16_t bitmapId) const;

	const FlashSound* getSound(uint16_t soundId) const;

	const FlashCharacter* getCharacter(uint16_t characterId) const;

	bool getExportId(const std::string& name, uint16_t& outExportId) const;

	bool getExportName(uint16_t exportId, std::string& outName) const;

private:
	friend class FlashMovie;

	SmallMap< uint16_t, Ref< FlashFont > > m_fonts;
	SmallMap< uint16_t, Ref< FlashBitmap > > m_bitmaps;
	SmallMap< uint16_t, Ref< FlashSound > > m_sounds;
	SmallMap< uint16_t, Ref< FlashCharacter > > m_characters;
	SmallMap< std::string, uint16_t > m_exports;
};

	}
}

#endif	// traktor_flash_FlashDictionary_H
