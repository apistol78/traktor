#ifndef traktor_flash_FlashMovie_H
#define traktor_flash_FlashMovie_H

#include <map>
#include "Core/Object.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class FlashBitmap;
class FlashCharacter;
class FlashFont;
class FlashSprite;
class FlashSpriteInstance;
class IActionVM;

/*! \brief Flash movie.
 * \ingroup Flash
 */
class T_DLLCLASS FlashMovie : public Object
{
	T_RTTI_CLASS;

public:
	FlashMovie(const SwfRect& frameBounds, FlashSprite* movieClip);

	void defineFont(uint16_t fontId, FlashFont* font);

	void defineBitmap(uint16_t bitmapId, FlashBitmap* bitmap);

	void defineCharacter(uint16_t characterId, FlashCharacter* character);

	void removeCharacter(FlashCharacter* character);

	const FlashFont* getFont(uint16_t fontId) const;

	const FlashBitmap* getBitmap(uint16_t bitmapId) const;

	const FlashCharacter* getCharacter(uint16_t characterId) const;

	void setExport(const std::wstring& name, uint16_t exportId);

	bool getExportId(const std::wstring& name, uint16_t& outExportId) const;

	bool getExportName(uint16_t exportId, std::wstring& outName) const;

	const SwfRect& getFrameBounds() const;

	const FlashSprite* getMovieClip() const;

	Ref< FlashSpriteInstance > createMovieClipInstance(const IActionVM* vm) const;

	Ref< FlashSpriteInstance > createExternalMovieClipInstance(FlashSpriteInstance* containerInstance) const;

	const std::map< uint16_t, Ref< FlashFont > >& getFonts() const { return m_fonts; }

	const std::map< uint16_t, Ref< FlashBitmap > >& getBitmaps() const { return m_bitmaps; }

	const std::map< uint16_t, Ref< FlashCharacter > >& getCharacters() const { return m_characters; }

	const std::map< std::wstring, uint16_t >& getExports() const { return m_exports; }

private:
	SwfRect m_frameBounds;
	Ref< FlashSprite > m_movieClip;
	std::map< uint16_t, Ref< FlashFont > > m_fonts;
	std::map< uint16_t, Ref< FlashBitmap > > m_bitmaps;
	std::map< uint16_t, Ref< FlashCharacter > > m_characters;
	std::map< std::wstring, uint16_t > m_exports;
};

	}
}

#endif	// traktor_flash_FlashMovie_H
