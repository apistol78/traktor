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
	float m_parameter;
	Ref< ISoundBufferCursor > m_cursors[2];
	float* m_outputSamples[SbcMaxChannelCount];

	BlendGrainCursor()
	:	m_parameter(0.0f)
	{
	}

	virtual ~BlendGrainCursor()
	{
		Alloc::freeAlign(m_outputSamples[0]);
	}

	virtual void setParameter(float parameter)
	{
		m_parameter = parameter;
	}

	virtual void reset()
	{
		m_cursors[0]->reset();
		m_cursors[1]->reset();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BlendGrain", BlendGrain, IGrain)

BlendGrain::BlendGrain(IGrain* grain1, IGrain* grain2)
{
	m_grains[0] = grain1;
	m_grains[1] = grain2;
}

Ref< ISoundBufferCursor > BlendGrain::createCursor() const
{
	Ref< BlendGrainCursor > cursor = new BlendGrainCursor();

	cursor->m_cursors[0] = m_grains[0]->createCursor();
	cursor->m_cursors[1] = m_grains[1]->createCursor();

	const uint32_t outputSamplesCount = 1024/*hwFrameSamples*/ * c_outputSamplesBlockCount;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	cursor->m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	std::memset(cursor->m_outputSamples[0], 0, outputSamplesSize);

	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		cursor->m_outputSamples[i] = cursor->m_outputSamples[0] + outputSamplesCount * i;

	return cursor;
}

void BlendGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	BlendGrainCursor* blendCursor = checked_type_cast< BlendGrainCursor*, false >(cursor);
	m_grains[0]->updateCursor(blendCursor->m_cursors[0]);
	m_grains[1]->updateCursor(blendCursor->m_cursors[1]);
}

bool BlendGrain::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	BlendGrainCursor* blendCursor = checked_type_cast< BlendGrainCursor*, false >(cursor);

	SoundBlock soundBlock1 = { { 0, 0, 0, 0, 0, 0, 0, 0 }, outBlock.samplesCount, 0, 0 };
	SoundBlock soundBlock2 = { { 0, 0, 0, 0, 0, 0, 0, 0 }, outBlock.samplesCount, 0, 0 };

	bool anyBlock = false;
	anyBlock |= m_grains[0]->getBlock(blendCursor->m_cursors[0], mixer, soundBlock1);
	anyBlock |= m_grains[1]->getBlock(blendCursor->m_cursors[1], mixer, soundBlock1);
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
				1.0f - blendCursor->m_parameter
			);
			mixer->addMulConst(
				outBlock.samples[i],
				soundBlock2.samples[i],
				soundBlock2.samplesCount,
				blendCursor->m_parameter
			);
		}
		else if (soundBlock1.samples[i])
		{
			outBlock.samples[i] = blendCursor->m_outputSamples[i];
			mixer->mulConst(
				outBlock.samples[i],
				soundBlock1.samples[i],
				soundBlock1.samplesCount,
				blendCursor->m_parameter
			);
		}
		else if (soundBlock2.samples[i])
		{
			outBlock.samples[i] = blendCursor->m_outputSamples[i];
			mixer->mulConst(
				outBlock.samples[i],
				soundBlock2.samples[i],
				soundBlock2.samplesCount,
				1.0f - blendCursor->m_parameter
			);
		}
		else
			outBlock.samples[i] = 0;
	}

	return true;
}

	}
}
