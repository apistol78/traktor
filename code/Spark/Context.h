/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"

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

class CharacterInstance;
class Key;
class Mouse;
class Movie;
class Sound;
class SpriteInstance;
class Stage;
class ICharacterFactory;
class IMovieLoader;

/*! Runtime context.
 * \ingroup Spark
 */
class T_DLLCLASS Context : public Object
{
	T_RTTI_CLASS;

public:
	explicit Context(const Movie* movie, const ICharacterFactory* characterFactory, const IMovieLoader* movieLoader);

	void setMovieClip(SpriteInstance* movieClip);

	void setFocus(CharacterInstance* focus);

	void setPressed(CharacterInstance* pressed);

	void setRolledOver(CharacterInstance* rolledOver);

	const Movie* getMovie() const { return m_movie; }

	const ICharacterFactory* getCharacterFactory() const { return m_characterFactory; }

	const IMovieLoader* getMovieLoader() const { return m_movieLoader; }

	Key* getKey() const { return m_key; }

	Mouse* getMouse() const { return m_mouse; }

	Sound* getSound() const { return m_sound; }

	Stage* getStage() const { return m_stage; }

	SpriteInstance* getMovieClip() const { return m_movieClip; }

	CharacterInstance* getFocus() const { return m_focus; }

	CharacterInstance* getPressed() const { return m_pressed; }

	CharacterInstance* getRolledOver() const { return m_rolledOver; }

private:
	const Movie* m_movie = nullptr;
	Ref< const ICharacterFactory > m_characterFactory;
	Ref< const IMovieLoader > m_movieLoader;
	Ref< Key > m_key;
	Ref< Mouse > m_mouse;
	Ref< Sound > m_sound;
	Ref< Stage > m_stage;
	SpriteInstance* m_movieClip = nullptr;
	CharacterInstance* m_focus = nullptr;
	CharacterInstance* m_pressed = nullptr;
	CharacterInstance* m_rolledOver = nullptr;
};

	}
}

