/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Sound.h"
#include "Sound/Player/SoundPlayer.h"
#include "Spark/Sound/SoundBuffer.h"
#include "Spark/Sound/SoundRenderer.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SoundRenderer", SoundRenderer, ISoundRenderer)

bool SoundRenderer::create(sound::SoundPlayer* soundPlayer)
{
	m_soundPlayer = soundPlayer;
	return true;
}

void SoundRenderer::destroy()
{
	m_soundPlayer = nullptr;
}

void SoundRenderer::play(const Sound* sound)
{
	if (m_soundPlayer)
	{
		Ref< sound::Sound > snd = new sound::Sound(new SoundBuffer(sound), 0, 1.0f, 0.0f);
		m_soundPlayer->play(snd, 0);
	}
}

}
