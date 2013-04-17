#include <cstring>
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/ISoundMixer.h"
#include "Sound/Resound/EnvelopeGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const uint32_t c_outputSamplesBlockCount = 8;

struct EnvelopeGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	handle_t m_id;
	float m_parameter;
	RefArray< ISoundBufferCursor > m_cursors;
	float* m_outputSamples[SbcMaxChannelCount];

	virtual ~EnvelopeGrainCursor()
	{
		Alloc::freeAlign(m_outputSamples[0]);
	}

	virtual void setParameter(handle_t id, float parameter)
	{
		if (m_id == id)
			m_parameter = parameter;
	}

	virtual void disableRepeat()
	{
		for (RefArray< ISoundBufferCursor >::iterator i = m_cursors.begin(); i != m_cursors.end(); ++i)
			(*i)->disableRepeat();
	}

	virtual void reset()
	{
		for (RefArray< ISoundBufferCursor >::iterator i = m_cursors.begin(); i != m_cursors.end(); ++i)
			(*i)->reset();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.EnvelopeGrain", EnvelopeGrain, IGrain)

EnvelopeGrain::EnvelopeGrain(handle_t id, const std::vector< Grain >& grains)
:	m_id(id)
,	m_grains(grains)
{
}

Ref< ISoundBufferCursor > EnvelopeGrain::createCursor() const
{
	if (m_grains.empty())
		return 0;

	Ref< EnvelopeGrainCursor > cursor = new EnvelopeGrainCursor();
	cursor->m_id = m_id;
	cursor->m_parameter = 0.0f;

	cursor->m_cursors.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		cursor->m_cursors[i] = m_grains[i].grain->createCursor();
		if (!cursor->m_cursors[i])
			return 0;
	}

	const uint32_t outputSamplesCount = 1024/*hwFrameSamples*/ * c_outputSamplesBlockCount;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	cursor->m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	std::memset(cursor->m_outputSamples[0], 0, outputSamplesSize);

	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		cursor->m_outputSamples[i] = cursor->m_outputSamples[0] + outputSamplesCount * i;

	return cursor;
}

void EnvelopeGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	EnvelopeGrainCursor* envelopeCursor = static_cast< EnvelopeGrainCursor* >(cursor);
	for (uint32_t i = 0; i < m_grains.size(); ++i)
		m_grains[i].grain->updateCursor(envelopeCursor->m_cursors[i]);
}

const IGrain* EnvelopeGrain::getCurrentGrain(const ISoundBufferCursor* cursor) const
{
	return this;
}

bool EnvelopeGrain::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	EnvelopeGrainCursor* envelopeCursor = static_cast< EnvelopeGrainCursor* >(cursor);
	T_ASSERT (envelopeCursor);

	float p = envelopeCursor->m_parameter;
	bool result = false;

	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		float I = m_grains[i].in;
		float O = m_grains[i].out;
		float Ie = m_grains[i].easeIn;
		float Oe = m_grains[i].easeOut;

		if (p < I - Ie)
			continue;
		if (p > O + Oe)
			continue;

		float v = 0.0f;

		if (p < I)
		{
			float f = (p - I + Ie) / Ie;
			v = std::sin(f * PI / 2.0f);
		}
		else if (p > O)
		{
			float f = 1.0f - (p - O) / Oe;
			v = std::sin(f * PI / 2.0f);
		}
		else
			v = 1.0f;

		if (v < FUZZY_EPSILON)
			continue;

		SoundBlock soundBlock = { { 0, 0, 0, 0, 0, 0, 0, 0 }, outBlock.samplesCount, 0, 0 };
		if (m_grains[i].grain->getBlock(
			envelopeCursor->m_cursors[i],
			mixer,
			soundBlock
		))
		{
			outBlock.sampleRate = soundBlock.sampleRate;
			outBlock.samplesCount = max(outBlock.samplesCount, soundBlock.samplesCount);
			outBlock.maxChannel = max(outBlock.maxChannel, soundBlock.maxChannel);

			for (uint32_t j = 0; j < soundBlock.maxChannel; ++j)
			{
				if (!soundBlock.samples[j])
					continue;

				if (!outBlock.samples[j])
				{
					outBlock.samples[j] = envelopeCursor->m_outputSamples[j];
					mixer->mulConst(
						outBlock.samples[j],
						soundBlock.samples[j],
						soundBlock.samplesCount,
						v
					);
				}
				else
				{
					mixer->addMulConst(
						outBlock.samples[j],
						soundBlock.samples[j],
						soundBlock.samplesCount,
						v
					);
				}
			}

			result = true;
		}
	}

	return result;
}

	}
}
