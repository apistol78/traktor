/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_Movie_H
#define traktor_flash_Movie_H

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

class Bitmap;
class Character;
class Font;
class Sound;
class Sprite;
class SpriteInstance;
class ICharacterFactory;
class IMovieLoader;

/*! \brief Flash movie.
 * \ingroup Flash
 */
class T_DLLCLASS Movie : public ISerializable
{
	T_RTTI_CLASS;

public:
	Movie();

	Movie(const Aabb2& frameBounds, Sprite* movieClip);

	void defineFont(uint16_t fontId, Font* font);

	void defineBitmap(uint16_t bitmapId, Bitmap* bitmap);

	void defineSound(uint16_t soundId, Sound* sound);

	void defineCharacter(uint16_t characterId, Character* character);

	void setExport(const std::string& name, uint16_t exportId);

	Ref< SpriteInstance > createMovieClipInstance(const ICharacterFactory* characterFactory, const IMovieLoader* movieLoader) const;

	Ref< SpriteInstance > createExternalMovieClipInstance(SpriteInstance* containerInstance, int32_t depth) const;

	Ref< SpriteInstance > createExternalSpriteInstance(SpriteInstance* containerInstance, const std::string& characterName, int32_t depth) const;

	const Aabb2& getFrameBounds() const { return m_frameBounds; }

	const Sprite* getMovieClip() const { return m_movieClip; }

	const SmallMap< uint16_t, Ref< Font > >& getFonts() const { return m_fonts; }

	const SmallMap< uint16_t, Ref< Bitmap > >& getBitmaps() const { return m_bitmaps; }

	const SmallMap< uint16_t, Ref< Sound > >& getSounds() const { return m_sounds; }

	const SmallMap< uint16_t, Ref< Character > >& getCharacters() const { return m_characters; }

	const SmallMap< std::string, uint16_t >& getExports() const { return m_exports; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Aabb2 m_frameBounds;
	Ref< Sprite > m_movieClip;
	SmallMap< uint16_t, Ref< Font > > m_fonts;
	SmallMap< uint16_t, Ref< Bitmap > > m_bitmaps;
	SmallMap< uint16_t, Ref< Sound > > m_sounds;
	SmallMap< uint16_t, Ref< Character > > m_characters;
	SmallMap< std::string, uint16_t > m_exports;
};

	}
}

#endif	// traktor_flash_Movie_H
