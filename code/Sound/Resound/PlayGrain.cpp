/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector4.h"
#include "Sound/IAudioBuffer.h"
#include "Sound/IAudioFilter.h"
#include "Sound/IAudioMixer.h"
#include "Sound/IAudioResource.h"
#include "Sound/Sound.h"
#include "Sound/Resound/PlayGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct PlayGrainCursor : public RefCountImpl< IAudioBufferCursor >
{
	Ref< IAudioBuffer > m_soundBuffer;
	Ref< IAudioBufferCursor > m_soundCursor;
	RefArray< IAudioFilterInstance > m_filterInstances;
	bool m_repeat;
	float m_gain;
	float m_pitch;

	virtual void setParameter(handle_t id, float parameter) override final
	{
		if (m_soundCursor)
			m_soundCursor->setParameter(id, parameter);
	}

	virtual void disableRepeat() override final
	{
		m_repeat = false;
	}

	virtual void reset() override final
	{
		if (m_soundCursor)
			m_soundCursor->reset();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.PlayGrain", PlayGrain, IGrain)

PlayGrain::PlayGrain(
	const resource::Proxy< Sound >& sound,
	const RefArray< IAudioFilter >& filters,
	const Range< float >& gain,
	const Range< float >& pitch,
	bool repeat
)
:	m_sound(sound)
,	m_filters(filters)
,	m_gain(gain)
,	m_pitch(pitch)
,	m_repeat(repeat)
{
}

Ref< IAudioBufferCursor > PlayGrain::createCursor() const
{
	if (!m_sound)
		return 0;

	Ref< IAudioBuffer > soundBuffer = m_sound->getBuffer();
	if (!soundBuffer)
		return 0;

	Ref< IAudioBufferCursor > soundCursor = soundBuffer->createCursor();
	if (!soundCursor)
		return 0;

	Ref< PlayGrainCursor > playCursor = new PlayGrainCursor();
	playCursor->m_soundBuffer = soundBuffer;
	playCursor->m_soundCursor = soundCursor;
	playCursor->m_repeat = m_repeat;
	playCursor->m_gain = decibelToLinear(m_sound->getGain() + m_gain.random(m_random));
	playCursor->m_pitch = clamp(m_pitch.random(m_random), 0.5f, 1.5f);

	for (auto filter : m_filters)
		playCursor->m_filterInstances.push_back(filter ? filter->createInstance() : nullptr);

	m_sound.consume();

	return playCursor;
}

void PlayGrain::updateCursor(IAudioBufferCursor* cursor) const
{
	PlayGrainCursor* playCursor = static_cast< PlayGrainCursor* >(cursor);
	playCursor->m_filterInstances.resize(0);
	for (auto filter : m_filters)
		playCursor->m_filterInstances.push_back(filter ? filter->createInstance() : nullptr);
}

const IGrain* PlayGrain::getCurrentGrain(const IAudioBufferCursor* cursor) const
{
	return this;
}

void PlayGrain::getActiveGrains(const IAudioBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	outActiveGrains.push_back(this);
}

bool PlayGrain::getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	PlayGrainCursor* playCursor = static_cast< PlayGrainCursor* >(cursor);
	if (!playCursor)
		return false;

	// Have sound been externally changed then we need to re-create buffer and cursor.
	if (m_sound.changed())
	{
		if (!m_sound)
			return false;

		Ref< IAudioBuffer > soundBuffer = m_sound->getBuffer();
		if (!soundBuffer)
			return false;

		Ref< IAudioBufferCursor > soundCursor = soundBuffer->createCursor();
		if (!soundCursor)
			return false;

		playCursor->m_soundBuffer = soundBuffer;
		playCursor->m_soundCursor = soundCursor;
		playCursor->m_gain = decibelToLinear(m_sound->getGain() + m_gain.random(m_random));

		m_sound.consume();
	}

	T_ASSERT(playCursor->m_soundBuffer);
	T_ASSERT(playCursor->m_soundCursor);

	// Get sound block from buffer; rewind if repeat flag is set.
	if (!playCursor->m_soundBuffer->getBlock(
		playCursor->m_soundCursor,
		mixer,
		outBlock
	))
	{
		if (playCursor->m_repeat)
		{
			playCursor->m_soundCursor->reset();
			if (!playCursor->m_soundBuffer->getBlock(
				playCursor->m_soundCursor,
				mixer,
				outBlock
			))
				return false;
		}
		else
			return false;
	}

	// Apply filter chain.
	const uint32_t nfilters = (uint32_t)m_filters.size();
	for (uint32_t i = 0; i < nfilters; ++i)
	{
		if (m_filters[i])
		{
			m_filters[i]->apply(
				playCursor->m_filterInstances[i],
				outBlock
			);
		}
	}

	// Apply gain.
	if (abs(1.0f - playCursor->m_gain) > FUZZY_EPSILON)
	{
		for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
		{
			if (outBlock.samples[i])
				mixer->mulConst(outBlock.samples[i], outBlock.samplesCount, playCursor->m_gain);
		}
	}

	// Modify sample rate by random pitch; sound channel will normalize sample rate thus alter pitch.
	if (abs(1.0f - playCursor->m_pitch) > FUZZY_EPSILON)
		outBlock.sampleRate = (uint32_t)(outBlock.sampleRate * playCursor->m_pitch);

	return true;
}

	}
}
