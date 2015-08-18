#ifndef traktor_flash_FlashMovie_H
#define traktor_flash_FlashMovie_H

#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"
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
class FlashSound;
class FlashSprite;
class FlashSpriteInstance;
class IFlashMovieLoader;

/*! \brief Flash movie.
 * \ingroup Flash
 */
class T_DLLCLASS FlashMovie : public ISerializable
{
	T_RTTI_CLASS;

public:
	FlashMovie();

	FlashMovie(const Aabb2& frameBounds, FlashSprite* movieClip);

	void defineFont(uint16_t fontId, FlashFont* font);

	void defineBitmap(uint16_t bitmapId, FlashBitmap* bitmap);

	void defineSound(uint16_t soundId, FlashSound* sound);

	void defineCharacter(uint16_t characterId, FlashCharacter* character);

	void setExport(const std::string& name, uint16_t exportId);

	Ref< FlashSpriteInstance > createMovieClipInstance(const IFlashMovieLoader* movieLoader) const;

	Ref< FlashSpriteInstance > createExternalMovieClipInstance(FlashSpriteInstance* containerInstance) const;

	const Aabb2& getFrameBounds() const { return m_frameBounds; }

	const FlashSprite* getMovieClip() const { return m_movieClip; }

	const SmallMap< uint16_t, Ref< FlashFont > >& getFonts() const { return m_fonts; }

	const SmallMap< uint16_t, Ref< FlashBitmap > >& getBitmaps() const { return m_bitmaps; }

	const SmallMap< uint16_t, Ref< FlashSound > >& getSounds() const { return m_sounds; }

	const SmallMap< uint16_t, Ref< FlashCharacter > >& getCharacters() const { return m_characters; }

	const SmallMap< std::string, uint16_t >& getExports() const { return m_exports; }

	virtual void serialize(ISerializer& s);

private:
	Aabb2 m_frameBounds;
	Ref< FlashSprite > m_movieClip;
	SmallMap< uint16_t, Ref< FlashFont > > m_fonts;
	SmallMap< uint16_t, Ref< FlashBitmap > > m_bitmaps;
	SmallMap< uint16_t, Ref< FlashSound > > m_sounds;
	SmallMap< uint16_t, Ref< FlashCharacter > > m_characters;
	SmallMap< std::string, uint16_t > m_exports;
};

	}
}

#endif	// traktor_flash_FlashMovie_H
