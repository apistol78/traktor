/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Memory/Alloc.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Sine.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_Sin_i[] =
{
	{ L"Frequency", NptScalar, false },
	{ L"Amplitude", NptScalar, false },
	{ 0 }
};

const ImmutableNode::OutputPinDesc c_Sine_o[] =
{
	{ L"Output", NptSignal },
	{ 0 }
};

class SineCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	float* m_outputSamples;
	float m_time;

	SineCursor()
	{
		m_outputSamples = nullptr;
	}

	virtual ~SineCursor()
	{
		if (m_outputSamples)
			Alloc::freeAlign(m_outputSamples);
	}

	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Sine", 0, Sine, ImmutableNode)

Sine::Sine()
:	ImmutableNode(c_Sin_i, c_Sine_o)
{
}

bool Sine::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< ISoundBufferCursor > Sine::createCursor() const
{
	Ref< SineCursor > sineCursor = new SineCursor();

	const uint32_t outputSamplesCount = 1024/*hwFrameSamples*/;
	const uint32_t outputSamplesSize = outputSamplesCount * sizeof(float);

	sineCursor->m_outputSamples = static_cast< float* >(Alloc::acquireAlign(outputSamplesSize, 16, T_FILE_LINE));
	sineCursor->m_time = 0.0f;

	return sineCursor;
}

bool Sine::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	SineCursor* sineCursor = static_cast< SineCursor* >(cursor);

	// Evaluate scalars.
	float frequency, amplitude;
	if (!evaluator->evaluateScalar(getInputPin(0), frequency))
		return false;
	if (!evaluator->evaluateScalar(getInputPin(1), amplitude))
		return false;
	if (frequency <= 0.0f || amplitude <= 0.0f)
		return false;

	float t = evaluator->getTime();
	float k = TWO_PI * frequency;
	outScalar = float(std::sin(t * k) * amplitude);

	return true;
}

bool Sine::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	SineCursor* sineCursor = static_cast< SineCursor* >(cursor);

	// Evaluate scalars.
	float frequency, amplitude;
	if (!evaluator->evaluateScalar(getInputPin(0), frequency))
		return false;
	if (!evaluator->evaluateScalar(getInputPin(1), amplitude))
		return false;
	if (frequency <= 0.0f || amplitude <= 0.0f)
		return false;

	// Prepare output block.
	outBlock.samples[0] = sineCursor->m_outputSamples;
	outBlock.samplesCount = 1024/*hwFrameSamples*/;
	outBlock.sampleRate = 44100;
	outBlock.maxChannel = 1;

	// Generate signal.
	float t = sineCursor->m_time;
	float dt = 1.0f / outBlock.sampleRate;
	float k = TWO_PI * frequency;
	for (int32_t i = 0; i < 1024; ++i)
	{
		outBlock.samples[0][i] = float(std::sin(t * k) * amplitude);
		t += dt;
	}

	sineCursor->m_time = t;
	return true;
}

	}
}
