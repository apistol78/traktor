/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb2.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class Bitmap;
class Character;
class Font;
class Sound;
class Sprite;
class SpriteInstance;
class ICharacterFactory;
class IMovieLoader;

/*! Movie
 * \ingroup Spark
 */
class T_DLLCLASS Movie : public ISerializable
{
	T_RTTI_CLASS;

public:
	Movie() = default;

	explicit Movie(const Aabb2& frameBounds, Sprite* movieClip);

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

	virtual void serialize(ISerializer& s) override final;

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

