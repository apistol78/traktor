/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Sound/IAudioBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class IGrain;

class T_DLLCLASS BankBuffer : public IAudioBuffer
{
	T_RTTI_CLASS;

public:
	explicit BankBuffer(const RefArray< IGrain >& grains);

	void updateCursor(IAudioBufferCursor* cursor) const;

	const IGrain* getCurrentGrain(const IAudioBufferCursor* cursor) const;

	void getActiveGrains(const IAudioBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const;

	virtual Ref< IAudioBufferCursor > createCursor() const override final;

	virtual bool getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const override final;

private:
	RefArray< IGrain > m_grains;
};

}
