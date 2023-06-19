/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/StringSplit.h"
#include "Spark/Context.h"
#include "Spark/Key.h"
#include "Spark/Mouse.h"
#include "Spark/Sound.h"
#include "Spark/Stage.h"
#include "Spark/SpriteInstance.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Context", Context, Object)

Context::Context(const Movie* movie, const ICharacterFactory* characterFactory, const IMovieLoader* movieLoader)
:	m_movie(movie)
,	m_characterFactory(characterFactory)
,	m_movieLoader(movieLoader)
{
	m_key = new Key();
	m_mouse = new Mouse();
	m_sound = new Sound();
	m_stage = new Stage(this);
}

void Context::setMovieClip(SpriteInstance* movieClip)
{
	m_movieClip = movieClip;
}

void Context::setFocus(CharacterInstance* focus)
{
	if (m_focus)
		m_focus->eventKillFocus();

	m_focus = focus;

	if (m_focus)
		m_focus->eventSetFocus();
}

void Context::setPressed(CharacterInstance* pressed)
{
	m_pressed = pressed;
}

void Context::setRolledOver(CharacterInstance* rolledOver)
{
	m_rolledOver = rolledOver;
}

}
