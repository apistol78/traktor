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
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class IAudioMixer;
class IAudioBufferCursor;

/*!
 * \ingroup Sound
 */
class T_DLLCLASS IGrain : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< IAudioBufferCursor > createCursor() const = 0;

	virtual void updateCursor(IAudioBufferCursor* cursor) const = 0;

	virtual const IGrain* getCurrentGrain(const IAudioBufferCursor* cursor) const = 0;

	virtual void getActiveGrains(const IAudioBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const = 0;

	virtual bool getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const = 0;
};

}
