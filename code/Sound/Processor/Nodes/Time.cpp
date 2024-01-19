/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/IAudioBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Time.h"

namespace traktor::sound
{
	namespace
	{

const ImmutableNode::OutputPinDesc c_Time_o[] =
{
	{ L"Output", NodePinType::Scalar },
	{ 0 }
};

class TimeCursor : public RefCountImpl< IAudioBufferCursor >
{
public:
	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Time", 0, Time, ImmutableNode)

Time::Time()
:	ImmutableNode(nullptr, c_Time_o)
{
}

bool Time::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< IAudioBufferCursor > Time::createCursor() const
{
	return new TimeCursor();
}

bool Time::getScalar(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	outScalar = evaluator->getTime();
	return true;
}

bool Time::getBlock(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	return false;
}

}
