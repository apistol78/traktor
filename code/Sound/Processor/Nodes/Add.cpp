/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/IAudioMixer.h"
#include "Sound/IAudioBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Add.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_Add_i[] =
{
	{ L"Input1", NodePinType::Scalar, false },
	{ L"Input2", NodePinType::Scalar, false },
	{ 0 }
};

const ImmutableNode::OutputPinDesc c_Add_o[] =
{
	{ L"Output", NodePinType::Signal },
	{ 0 }
};

class AddCursor : public RefCountImpl< IAudioBufferCursor >
{
public:
	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Add", 0, Add, ImmutableNode)

Add::Add()
:	ImmutableNode(c_Add_i, c_Add_o)
{
}

bool Add::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< IAudioBufferCursor > Add::createCursor() const
{
	return new AddCursor();
}

bool Add::getScalar(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	float lh, rh;
	if (!evaluator->evaluateScalar(getInputPin(0), lh))
		return false;
	if (!evaluator->evaluateScalar(getInputPin(1), rh))
		return false;

	outScalar = lh + rh;
	return true;
}

bool Add::getBlock(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
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

	for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
	{
		if (soundBlock1.samples[i] && soundBlock2.samples[i])
		{
			outBlock.samples[i] = soundBlock1.samples[i];
			mixer->addMulConst(
				outBlock.samples[i],
				soundBlock2.samples[i],
				soundBlock2.samplesCount,
				1.0f
			);
		}
		else if (soundBlock1.samples[i])
		{
			outBlock.samples[i] = soundBlock1.samples[i];
		}
		else if (soundBlock2.samples[i])
		{
			outBlock.samples[i] = soundBlock2.samples[i];
		}
		else
			outBlock.samples[i] = nullptr;
	}

	return true;
}

	}
}
