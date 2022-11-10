/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Multiply.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_Multiply_i[] =
{
	{ L"Input1", NptScalar, false },
	{ L"Input2", NptScalar, false },
	{ 0 }
};

const ImmutableNode::OutputPinDesc c_Multiply_o[] =
{
	{ L"Output", NptSignal },
	{ 0 }
};

class MultiplyCursor : public RefCountImpl< ISoundBufferCursor >
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

Ref< ISoundBufferCursor > Multiply::createCursor() const
{
	return new MultiplyCursor();
}

bool Multiply::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	float lh, rh;
	if (!evaluator->evaluateScalar(getInputPin(0), lh))
		return false;
	if (!evaluator->evaluateScalar(getInputPin(1), rh))
		return false;

	outScalar = lh * rh;
	return true;
}

bool Multiply::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	return false;
}

	}
}
