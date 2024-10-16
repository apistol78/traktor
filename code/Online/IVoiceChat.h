/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class SoundPlayer;

}

namespace traktor::online
{

class IUser;

class T_DLLCLASS IVoiceChat : public Object
{
	T_RTTI_CLASS;

public:
	virtual void setSoundPlayer(sound::SoundPlayer* soundPlayer) = 0;

	virtual void setAudience(const RefArray< IUser >& audience) = 0;

	virtual void beginTransmission() = 0;

	virtual void endTransmission() = 0;

	virtual void setMute(bool mute) = 0;

	virtual bool isTransmitting(IUser* user) = 0;
};

}
