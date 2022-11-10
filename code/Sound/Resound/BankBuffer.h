/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Sound/ISoundBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class IGrain;

class T_DLLCLASS BankBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	BankBuffer(const RefArray< IGrain >& grains);

	void updateCursor(ISoundBufferCursor* cursor) const;

	const IGrain* getCurrentGrain(const ISoundBufferCursor* cursor) const;

	void getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const;

	virtual Ref< ISoundBufferCursor > createCursor() const override final;

	virtual bool getBlock(ISoundBufferCursor* cursor, const IAudioMixer* mixer, SoundBlock& outBlock) const override final;

private:
	RefArray< IGrain > m_grains;
	mutable Semaphore m_lock;
};

	}
}

