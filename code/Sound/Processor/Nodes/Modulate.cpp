/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Memory/Alloc.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Modulate.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_Modulate_i[] =
{
	{ L"Input1", NptSignal, false },
	{ L"Input2", NptSignal, false },
	{ 0 }
};

const ImmutableNode::OutputPinDesc c_Modulate_o[] =
{
	{ L"Output", NptSignal },
	{ 0 }
};

class ModulateCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	float* m_outputSamples[SbcMaxChannelCount];

	ModulateCursor()
	{
		m_outputSamples[0] = nullptr;
	}

	virtual ~ModulateCursor()
	{
		if (m_outputSamples[0])
			Alloc::freeAlign(m_outputSamples[0]);
	}

	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Modulate", 0, Modulate, ImmutableNode)

Modulate::Modulate()
:	ImmutableNode(c_Modulate_i, c_Modulate_o)
{
}

bool Modulate::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< ISoundBufferCursor > Modulate::createCursor() const
{
	Ref< ModulateCursor > modulateCursor = new ModulateCursor();

	const uint32_t outputSamplesCount = 1024/*hwFrameSamples*/;
	const uint32_t outputSamplesSize = SbcMaxChannelCount * outputSamplesCount * sizeof(float);

	modulateCursor->m_outputSamples[0] = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	if (!modulateCursor->m_outputSamples[0])
		return 0;

	for (uint32_t i = 1; i < SbcMaxChannelCount; ++i)
		modulateCursor->m_outputSamples[i] = modulateCursor->m_outputSamples[0] + outputSamplesCount * i;

	return modulateCursor;
}

bool Modulate::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	return false;
}

bool Modulate::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	ModulateCursor* modulateCursor = static_cast< ModulateCursor* >(cursor);

	SoundBlock soundBlock1 = { { 0 }, outBlock.samplesCount, 0, 0 };
	SoundBlock soundBlock2 = { { 0 }, outBlock.samplesCount, 0, 0 };

	if (!evaluator->evaluateBlock(getInputPin(0), mixer, soundBlock1))
		return false;
	if (!evaluator->evaluateBlock(getInputPin(1), mixer, soundBlock2))
		return false;

	outBlock.sampleRate = std::max(soundBlock1.sampleRate, soundBlock2.sampleRate);
	outBlock.samplesCount = std::max(soundBlock1.samplesCount, soundBlock2.samplesCount);
	outBlock.maxChannel = std::max(soundBlock1.maxChannel, soundBlock2.maxChannel);

	T_ASSERT(mixer);
	for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
	{
		if (soundBlock1.samples[i] && soundBlock2.samples[i])
		{
			outBlock.samples[i] = modulateCursor->m_outputSamples[i];
			// \TBD
		}
		else
			outBlock.samples[i] = 0;
	}

	return true;
}

	}
}
