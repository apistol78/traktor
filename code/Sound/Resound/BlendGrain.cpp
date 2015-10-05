#include <cstring>
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/Alloc.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/ISoundMixer.h"
#include "Sound/Resound/BlendGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const uint32_t c_outputSamplesBlockCount = 8;

struct BlendGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	handle_t m_id;
	float m_parameter;
	float m_lastP;
	Ref< ISoundBufferCursor > m_cursors[2];
	float* m_outputSamples[SbcMaxChannelCount];

	BlendGrainCursor()
	:	m_id(0)
	,	m_parameter(0.0f)
	,	m_lastP(0.0f)
	{
		for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
			m_outputSamples[i] = 0;
	}

	virtual ~BlendGrainCursor()
	{
		if (m_outputSamples[0])
			Alloc::freeAlign(m_outputSamples[0]);
	}

	virtual void setParameter(handle_t id, float parameter) T_OVERRIDE T_FINAL
	{
		if (id == m_id)
			m_parameter = parameter;

		m_cursors[0]->setParameter(id, parameter);
		m_cursors[1]->setParameter(id, parameter);
	}

	virtual void disableRepeat() T_OVERRIDE T_FINAL
	{
		m_cursors[0]->disableRepeat();
		m_cursors[1]->disableRepeat();
	}

	virtual void reset() T_OVERRIDE T_FINAL
	{
		m_cursors[0]->reset();
		m_cursors[1]->reset();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BlendGrain", BlendGrain, IGrain)

BlendGrain::BlendGrain(handle_t id, float response, IGrain* grain1, IGrain* grain2)
:	m_id(id)
,	m_response(response)
{
	m_grains[0] = grain1;
	m_grains[1] = grain2;
}

Ref< ISoundBufferCursor > BlendGrain::createCursor() const
{
	Ref< BlendGrainCursor > cursor = new BlendGrainCursor();

	cursor->m_id = m_id;
	cursor->m_cursors[0] = m_grains[0]->createCursor();
	cursor->m_cursors[1] = m_grains[1]->createCursor();

	if (!cursor->m_cursors[0] || !cursor->m_cursors[1])
		return 0;

	const uint32_t outputSamplesCount = 1024/*hwFrameSamples*/ * c_outputSamplesBlockCount;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	cursor->m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	if (!cursor->m_outputSamples[0])
		return 0;

	std::memset(cursor->m_outputSamples[0], 0, outputSamplesSize);

	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		cursor->m_outputSamples[i] = cursor->m_outputSamples[0] + outputSamplesCount * i;

	return cursor;
}

void BlendGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	BlendGrainCursor* blendCursor = static_cast< BlendGrainCursor* >(cursor);
	if (m_grains[0])
		m_grains[0]->updateCursor(blendCursor->m_cursors[0]);
	if (m_grains[1])
		m_grains[1]->updateCursor(blendCursor->m_cursors[1]);
}

const IGrain* BlendGrain::getCurrentGrain(const ISoundBufferCursor* cursor) const
{
	return this;
}

void BlendGrain::getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const
{
	const BlendGrainCursor* blendCursor = static_cast< const BlendGrainCursor* >(cursor);

	outActiveGrains.push_back(this);

	if (m_grains[0] && blendCursor->m_parameter < 1.0f - FUZZY_EPSILON)
		m_grains[0]->getActiveGrains(blendCursor->m_cursors[0], outActiveGrains);
	if (m_grains[1] && blendCursor->m_parameter > FUZZY_EPSILON)
		m_grains[1]->getActiveGrains(blendCursor->m_cursors[1], outActiveGrains);
}

bool BlendGrain::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	BlendGrainCursor* blendCursor = static_cast< BlendGrainCursor* >(cursor);

	float p0 = clamp(blendCursor->m_parameter, 0.0f, 1.0f);

	// Filter parameter to reduce intense changes causing clipping noises.
	float k = clamp(m_response * float(outBlock.samplesCount) / 44050.0f, 0.0f, 1.0f);
	float p = blendCursor->m_lastP * (1.0f - k) + p0 * k;
	blendCursor->m_lastP = p;

	SoundBlock soundBlock1 = { { 0 }, outBlock.samplesCount, 0, 0 };
	SoundBlock soundBlock2 = { { 0 }, outBlock.samplesCount, 0, 0 };

	bool anyBlock = false;
	if (m_grains[0])
		anyBlock |= m_grains[0]->getBlock(blendCursor->m_cursors[0], mixer, soundBlock1);
	if (m_grains[1])
		anyBlock |= m_grains[1]->getBlock(blendCursor->m_cursors[1], mixer, soundBlock2);
	if (!anyBlock)
		return false;

	outBlock.sampleRate = max(soundBlock1.sampleRate, soundBlock2.sampleRate);
	outBlock.samplesCount = max(soundBlock1.samplesCount, soundBlock2.samplesCount);
	outBlock.maxChannel = max(soundBlock1.maxChannel, soundBlock2.maxChannel);

	T_ASSERT (mixer);
	for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
	{
		if (soundBlock1.samples[i] && soundBlock2.samples[i])
		{
			outBlock.samples[i] = blendCursor->m_outputSamples[i];
			mixer->mulConst(
				outBlock.samples[i],
				soundBlock1.samples[i],
				soundBlock1.samplesCount,
				1.0f - p
			);
			mixer->addMulConst(
				outBlock.samples[i],
				soundBlock2.samples[i],
				soundBlock2.samplesCount,
				p
			);
		}
		else if (soundBlock1.samples[i])
		{
			outBlock.samples[i] = blendCursor->m_outputSamples[i];
			mixer->mulConst(
				outBlock.samples[i],
				soundBlock1.samples[i],
				soundBlock1.samplesCount,
				1.0f - p
			);
		}
		else if (soundBlock2.samples[i])
		{
			outBlock.samples[i] = blendCursor->m_outputSamples[i];
			mixer->mulConst(
				outBlock.samples[i],
				soundBlock2.samples[i],
				soundBlock2.samplesCount,
				p
			);
		}
		else
			outBlock.samples[i] = 0;
	}

	return true;
}

	}
}
