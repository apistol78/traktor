#include "Core/Math/Const.h"
#include "Sound/IFilter.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/ISoundMixer.h"
#include "Sound/ISoundResource.h"
#include "Sound/Sound.h"
#include "Sound/Resound/PlayGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct PlayGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	Ref< ISoundBuffer > m_soundBuffer;
	Ref< ISoundBufferCursor > m_soundCursor;
	RefArray< IFilterInstance > m_filterInstances;
	bool m_repeat;
	float m_gain;
	float m_pitch;

	virtual void setParameter(handle_t id, float parameter)
	{
		if (m_soundCursor)
			m_soundCursor->setParameter(id, parameter);
	}

	virtual void disableRepeat()
	{
		m_repeat = false;
	}

	virtual void reset()
	{
		if (m_soundCursor)
			m_soundCursor->reset();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.PlayGrain", PlayGrain, IGrain)

PlayGrain::PlayGrain(
	const resource::Proxy< Sound >& sound,
	const RefArray< IFilter >& filters,
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

Ref< ISoundBufferCursor > PlayGrain::createCursor() const
{
	Ref< ISoundBuffer > soundBuffer = m_sound->getBuffer();
	if (!soundBuffer)
		return 0;

	Ref< ISoundBufferCursor > soundCursor = soundBuffer->createCursor();
	if (!soundCursor)
		return 0;

	Ref< PlayGrainCursor > playCursor = new PlayGrainCursor();
	playCursor->m_soundBuffer = soundBuffer;
	playCursor->m_soundCursor = soundCursor;
	playCursor->m_repeat = m_repeat;
	playCursor->m_gain = m_sound->getVolume() + clamp(m_gain.random(m_random), -1.0f, 1.0f);
	playCursor->m_pitch = clamp(m_pitch.random(m_random), 0.5f, 1.5f);

	for (RefArray< IFilter >::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
		playCursor->m_filterInstances.push_back((*i) ? (*i)->createInstance() : 0);

	m_sound.consume();

	return playCursor;
}

void PlayGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	PlayGrainCursor* playCursor = static_cast< PlayGrainCursor* >(cursor);
	
	playCursor->m_filterInstances.resize(0);
	for (RefArray< IFilter >::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
		playCursor->m_filterInstances.push_back((*i) ? (*i)->createInstance() : 0);
}

const IGrain* PlayGrain::getCurrentGrain(const ISoundBufferCursor* cursor) const
{
	return this;
}

void PlayGrain::getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	outActiveGrains.push_back(this);
}

bool PlayGrain::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	PlayGrainCursor* playCursor = static_cast< PlayGrainCursor* >(cursor);
	if (!playCursor)
		return false;

	// Have sound been externally changed then we need to re-create buffer and cursor.
	if (m_sound.changed())
	{
		if (!m_sound)
			return false;

		Ref< ISoundBuffer > soundBuffer = m_sound->getBuffer();
		if (!soundBuffer)
			return false;

		Ref< ISoundBufferCursor > soundCursor = soundBuffer->createCursor();
		if (!soundCursor)
			return false;

		playCursor->m_soundBuffer = soundBuffer;
		playCursor->m_soundCursor = soundCursor;
		playCursor->m_gain = m_sound->getVolume() + clamp(m_gain.random(m_random), -1.0f, 1.0f);

		m_sound.consume();
	}

	T_ASSERT (playCursor->m_soundBuffer);
	T_ASSERT (playCursor->m_soundCursor);

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
	uint32_t nfilters = uint32_t(m_filters.size());
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
		Scalar gain(playCursor->m_gain);
		for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
		{
			if (!outBlock.samples[i])
				continue;

			float* sb = outBlock.samples[i];
			int32_t s = 0;

			for (; s < int32_t(outBlock.samplesCount) - 7 * 4; s += 7 * 4)
			{
				Vector4 s4_0 = Vector4::loadAligned(&sb[s]);
				Vector4 s4_1 = Vector4::loadAligned(&sb[s + 4]);
				Vector4 s4_2 = Vector4::loadAligned(&sb[s + 8]);
				Vector4 s4_3 = Vector4::loadAligned(&sb[s + 12]);
				Vector4 s4_4 = Vector4::loadAligned(&sb[s + 16]);
				Vector4 s4_5 = Vector4::loadAligned(&sb[s + 20]);
				Vector4 s4_6 = Vector4::loadAligned(&sb[s + 24]);

				(s4_0 * gain).storeAligned(&sb[s]);
				(s4_1 * gain).storeAligned(&sb[s + 4]);
				(s4_2 * gain).storeAligned(&sb[s + 8]);
				(s4_3 * gain).storeAligned(&sb[s + 12]);
				(s4_4 * gain).storeAligned(&sb[s + 16]);
				(s4_5 * gain).storeAligned(&sb[s + 20]);
				(s4_6 * gain).storeAligned(&sb[s + 24]);
			}

			for (; s < int32_t(outBlock.samplesCount); s += 4)
			{
				Vector4 s4 = Vector4::loadAligned(&sb[s]);
				(s4 * gain).storeAligned(&sb[s]);
			}
		}
	}

	// Modify sample rate by random pitch; sound channel will normalize sample rate thus alter pitch.
	if (abs(1.0f - playCursor->m_pitch) > FUZZY_EPSILON)
		outBlock.sampleRate = uint32_t(outBlock.sampleRate * playCursor->m_pitch);

	return true;
}

	}
}
