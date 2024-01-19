/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/IAudioBuffer.h"
#include "Sound/Processor/Nodes/Scalar.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::OutputPinDesc c_Scalar_o[] =
{
	{ L"Output", NodePinType::Scalar },
	{ 0 }
};

class ScalarCursor : public RefCountImpl< IAudioBufferCursor >
{
public:
	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Scalar", 0, Scalar, ImmutableNode)

Scalar::Scalar()
:	ImmutableNode(nullptr, c_Scalar_o)
,	m_value(0.0f)
{
}

bool Scalar::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< IAudioBufferCursor > Scalar::createCursor() const
{
	return new ScalarCursor();
}

bool Scalar::getScalar(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	outScalar = m_value;
	return true;
}

bool Scalar::getBlock(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	return false;
}

void Scalar::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< float >(L"value", m_value);
}

	}
}
