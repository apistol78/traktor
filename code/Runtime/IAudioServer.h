/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IServer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class AudioSystem;
class ISoundPlayer;
class SurroundEnvironment;

	}

	namespace runtime
	{

/*! Audio server.
 * \ingroup Runtime
 *
 * "Audio.Type"					- Sound system type.
 * "Audio.MasterVolume"			- Master volume.
 * "Audio.Surround/MaxDistance"	- Surround sound max speaker distance.
 * "Audio.Surround/InnerRadius"	- Surround inner listener radius.
 * "Audio.Volumes/<Category>"   - Per category volume.
 */
class T_DLLCLASS IAudioServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual sound::AudioSystem* getAudioSystem() = 0;

	virtual sound::ISoundPlayer* getSoundPlayer() = 0;

	virtual sound::SurroundEnvironment* getSurroundEnvironment() = 0;
};

	}
}

