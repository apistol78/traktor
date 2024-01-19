/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Float.h"
#include "Core/Memory/Alloc.h"
#include "Sound/IAudioMixer.h"
#include "Sound/IAudioBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Blend.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_Blend_i[] =
{
	{ L"Input1", NodePinType::Signal, false },
	{ L"Input2", NodePinType::Signal, false },
	{ L"Weight", NodePinType::Scalar, false },
	{ 0 }
};

const ImmutableNode::OutputPinDesc c_Blend_o[] =
{
	{ L"Output", NodePinType::Signal },
	{ 0 }
};

class BlendCursor : public RefCountImpl< IAudioBufferCursor >
{
public:
	float* m_outputSamples[SbcMaxChannelCount];

	BlendCursor()
	{
		m_outputSamples[0] = nullptr;
	}

	virtual ~BlendCursor()
	{
		if (m_outputSamples[0])
			Alloc::freeAlign(m_outputSamples[0]);
	}

	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Blend", 0, Blend, ImmutableNode)

Blend::Blend()
:	ImmutableNode(c_Blend_i, c_Blend_o)
{
}

bool Blend::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< IAudioBufferCursor > Blend::createCursor() const
{
	Ref< BlendCursor > blendCursor = new BlendCursor();

	const uint32_t outputSamplesCount = 4096;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	blendCursor->m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	if (!blendCursor->m_outputSamples[0])
		return 0;

	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		blendCursor->m_outputSamples[i] = blendCursor->m_outputSamples[0] + outputSamplesCount * i;

	return blendCursor;
}

bool Blend::getScalar(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	float scalar1, scalar2, weight;

	if (!evaluator->evaluateScalar(getInputPin(0), scalar1))
		return false;
	if (!evaluator->evaluateScalar(getInputPin(1), scalar2))
		return false;
	if (!evaluator->evaluateScalar(getInputPin(2), weight))
		return false;

	outScalar = lerp(scalar1, scalar2, weight);
	return true;
}

bool Blend::getBlock(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	BlendCursor* blendCursor = static_cast< BlendCursor* >(cursor);

	float weight;
	if (!evaluator->evaluateScalar(getInputPin(2), weight))
		return false;

	AudioBlock soundBlock1 = { { 0 }, outBlock.samplesCount, 0, 0 };
	AudioBlock soundBlock2 = { { 0 }, outBlock.samplesCount, 0, 0 };

	bool anyBlocks = false;
	anyBlocks |= evaluator->evaluateBlock(getInputPin(0), mixer, soundBlock1);
	anyBlocks |= evaluator->evaluateBlock(getInputPin(1), mixer, soundBlock2);
	if (!anyBlocks)
		return false;

	outBlock.sampleRate = std::max(soundBlock1.sampleRate, soundBlock2.sampleRate);
	outBlock.samplesCount = std::max(soundBlock1.samplesCount, soundBlock2.samplesCount);
	outBlock.maxChannel = std::max(soundBlock1.maxChannel, soundBlock2.maxChannel);

	T_ASSERT(mixer);
	for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
	{
		if (soundBlock1.samples[i] && soundBlock2.samples[i])
		{
			outBlock.samples[i] = blendCursor->m_outputSamples[i];
			mixer->mulConst(
				outBlock.samples[i],
				soundBlock1.samples[i],
				soundBlock1.samplesCount,
				1.0f - weight
			);
			mixer->addMulConst(
				outBlock.samples[i],
				soundBlock2.samples[i],
				soundBlock2.samplesCount,
				weight
			);
		}
		else if (soundBlock1.samples[i])
		{
			outBlock.samples[i] = blendCursor->m_outputSamples[i];
			mixer->mulConst(
				outBlock.samples[i],
				soundBlock1.samples[i],
				soundBlock1.samplesCount,
				1.0f - weight
			);
		}
		else if (soundBlock2.samples[i])
		{
			outBlock.samples[i] = blendCursor->m_outputSamples[i];
			mixer->mulConst(
				outBlock.samples[i],
				soundBlock2.samples[i],
				soundBlock2.samplesCount,
				weight
			);
		}
		else
			outBlock.samples[i] = nullptr;
	}

	return true;
}

	}
}
