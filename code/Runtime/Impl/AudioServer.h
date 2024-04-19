/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Runtime/IAudioServer.h"
#include "Core/Platform.h"
#include "Core/Ref.h"

namespace traktor
{

class PropertyGroup;

}

namespace traktor::sound
{

class SoundPlayer;

}

namespace traktor::runtime
{

class IEnvironment;

/*!
 * \ingroup Runtime
 */
class AudioServer : public IAudioServer
{
	T_RTTI_CLASS;

public:
	bool create(const PropertyGroup* settings, const SystemApplication& sysapp);

	void destroy();

	int32_t reconfigure(const PropertyGroup* settings);

	void update(float dT, bool renderViewActive);

	uint32_t getActiveSoundChannels() const;

	virtual sound::AudioSystem* getAudioSystem() override final;

	virtual sound::ISoundPlayer* getSoundPlayer() override final;

	virtual sound::SurroundEnvironment* getSurroundEnvironment() override final;

private:
	Ref< sound::AudioSystem > m_audioSystem;
	Ref< sound::SoundPlayer > m_soundPlayer;
	Ref< sound::SurroundEnvironment > m_surroundEnvironment;
	std::wstring m_audioType;
	bool m_autoMute = true;
	bool m_soundMuted = false;
	float m_soundMutedVolume = 1.0f;
};

}
