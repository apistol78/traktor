/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/IAudioBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Output.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_Output_i[] = { { L"Input", NodePinType::Signal, false }, { 0 } };

class OutputCursor : public RefCountImpl< IAudioBufferCursor >
{
public:
	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Output", 0, Output, ImmutableNode)

Output::Output()
:	ImmutableNode(c_Output_i, nullptr)
{
}

bool Output::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< IAudioBufferCursor > Output::createCursor() const
{
	Ref< OutputCursor > outputCursor = new OutputCursor();
	return outputCursor;
}

bool Output::getScalar(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	return false;
}

bool Output::getBlock(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	return evaluator->evaluateBlock(getInputPin(0), mixer, outBlock);
}

	}
}
