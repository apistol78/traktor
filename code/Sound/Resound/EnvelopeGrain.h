/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Envelope.h"
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
class T_DLLCLASS EnvelopeGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	struct Grain
	{
		Ref< IGrain > grain;
		float in;
		float out;
		float easeIn;
		float easeOut;
	};

	explicit EnvelopeGrain(handle_t id, const AlignedVector< Grain >& grains, const float levels[3], float mid, float response);

	virtual Ref< IAudioBufferCursor > createCursor() const override final;

	virtual void updateCursor(IAudioBufferCursor* cursor) const override final;

	virtual const IGrain* getCurrentGrain(const IAudioBufferCursor* cursor) const override final;

	virtual void getActiveGrains(const IAudioBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const override final;

	virtual bool getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const override final;

private:
	handle_t m_id;
	AlignedVector< Grain > m_grains;
	float m_response;
	Envelope< float, HermiteEvaluator< float > > m_envelope;
};

}
