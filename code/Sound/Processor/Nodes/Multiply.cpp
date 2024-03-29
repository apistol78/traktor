/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/IAudioMixer.h"
#include "Sound/IAudioBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Multiply.h"

namespace traktor::sound
{
	namespace
	{

const ImmutableNode::InputPinDesc c_Multiply_i[] =
{
	{ L"Input1", NodePinType::Scalar, false },
	{ L"Input2", NodePinType::Scalar, false },
	{ 0 }
};

const ImmutableNode::OutputPinDesc c_Multiply_o[] =
{
	{ L"Output", NodePinType::Signal },
	{ 0 }
};

class MultiplyCursor : public RefCountImpl< IAudioBufferCursor >
{
public:
	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Multiply", 0, Multiply, ImmutableNode)

Multiply::Multiply()
:	ImmutableNode(c_Multiply_i, c_Multiply_o)
{
}

bool Multiply::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< IAudioBufferCursor > Multiply::createCursor() const
{
	return new MultiplyCursor();
}

bool Multiply::getScalar(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	float lh, rh;
	if (!evaluator->evaluateScalar(getInputPin(0), lh))
		return false;
	if (!evaluator->evaluateScalar(getInputPin(1), rh))
		return false;

	outScalar = lh * rh;
	return true;
}

bool Multiply::getBlock(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	const InputPin* ip1 = getInputPin(0);
	const InputPin* ip2 = getInputPin(1);

	const NodePinType pt1 = evaluator->evaluatePinType(ip1);
	const NodePinType pt2 = evaluator->evaluatePinType(ip2);

	if (pt1 == NodePinType::Signal && pt2 == NodePinType::Signal)
	{
		AudioBlock soundBlock1 = { { 0 }, outBlock.samplesCount, 0, 0 };
		AudioBlock soundBlock2 = { { 0 }, outBlock.samplesCount, 0, 0 };

		bool anyBlocks = false;
		anyBlocks |= evaluator->evaluateBlock(ip1, mixer, soundBlock1);
		anyBlocks |= evaluator->evaluateBlock(ip2, mixer, soundBlock2);
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
				mixer->mul(
					outBlock.samples[i],
					soundBlock2.samples[i],
					soundBlock2.samplesCount
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
	}
	else if ((pt1 == NodePinType::Scalar && pt2 == NodePinType::Signal) || (pt1 == NodePinType::Signal && pt2 == NodePinType::Scalar))
	{
		if (pt1 == NodePinType::Signal)
			std::swap(ip1, ip2);

		float f;
		if (!evaluator->evaluateScalar(ip1, f))
			return false;

		if (!evaluator->evaluateBlock(ip2, mixer, outBlock))
			return false;

		for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
		{
			if (outBlock.samples[i])
				mixer->mulConst(outBlock.samples[i], outBlock.samplesCount, f);
		}
	}
	else
		return false;

	return true;
}

}
