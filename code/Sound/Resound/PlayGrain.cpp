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
	float m_gain;
	float m_pitch;

	virtual void setParameter(handle_t id, float parameter)
	{
		if (m_soundCursor)
			m_soundCursor->setParameter(id, parameter);
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
	playCursor->m_gain = clamp(m_gain.random(m_random), -1.0f, 1.0f);
	playCursor->m_pitch = clamp(m_pitch.random(m_random), 0.5f, 1.5f);

	for (RefArray< IFilter >::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
		playCursor->m_filterInstances.push_back((*i) ? (*i)->createInstance() : 0);

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

bool PlayGrain::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	PlayGrainCursor* playCursor = static_cast< PlayGrainCursor* >(cursor);
	if (!playCursor)
		return false;

	T_ASSERT (playCursor->m_soundBuffer);
	T_ASSERT (playCursor->m_soundCursor);

	// Get sound block from buffer; rewind if repeat flag is set.
	if (!playCursor->m_soundBuffer->getBlock(
		playCursor->m_soundCursor,
		mixer,
		outBlock
	))
	{
		if (m_repeat)
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
	uint32_t nfilters = m_filters.size();
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

	// Apply random gain.
	if (abs(playCursor->m_gain) > FUZZY_EPSILON)
	{
		Scalar gain(playCursor->m_gain + 1.0f);
		for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
		{
			if (!outBlock.samples[i])
				continue;

			for (uint32_t j = 0; j < outBlock.samplesCount; j += 4)
			{
				Vector4 s4 = Vector4::loadAligned(&outBlock.samples[i][j]);
				s4 *= gain;
				s4.storeAligned(&outBlock.samples[i][j]);
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
