/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/Resound/IGrain.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*!
 * \ingroup Sound
 */
class T_DLLCLASS BlendGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	explicit BlendGrain(handle_t id, float response, IGrain* grain1, IGrain* grain2);

	virtual Ref< IAudioBufferCursor > createCursor() const override final;

	virtual void updateCursor(IAudioBufferCursor* cursor) const override final;

	virtual const IGrain* getCurrentGrain(const IAudioBufferCursor* cursor) const override final;

	virtual void getActiveGrains(const IAudioBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const override final;

	virtual bool getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const override final;

private:
	handle_t m_id;
	float m_response;
	Ref< IGrain > m_grains[2];
};

}
