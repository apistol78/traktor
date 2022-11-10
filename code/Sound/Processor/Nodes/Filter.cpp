/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Sound/IFilter.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/GraphEvaluator.h"
#include "Sound/Processor/Nodes/Filter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_Filter_i[] =
{
	{ L"Input", NptSignal, false },
	{ 0 }
};

const ImmutableNode::OutputPinDesc c_Filter_o[] =
{
	{ L"Output", NptSignal },
	{ 0 }
};

class FilterCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	Ref< IFilterInstance > m_filterInstance;

	virtual void setParameter(handle_t id, float parameter) override final {}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Filter", 0, Filter, ImmutableNode)

Filter::Filter()
:	ImmutableNode(c_Filter_i, c_Filter_o)
{
}

bool Filter::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< ISoundBufferCursor > Filter::createCursor() const
{
	if (!m_filter)
		return nullptr;
	Ref< FilterCursor > fc = new FilterCursor();
	if ((fc->m_filterInstance = m_filter->createInstance()) == nullptr)
		return nullptr;
	return fc;
}

bool Filter::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const
{
	return false;
}

bool Filter::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	FilterCursor* fc = static_cast< FilterCursor* >(cursor);
	if (!evaluator->evaluateBlock(getInputPin(0), mixer, outBlock))
		return false;
	m_filter->apply(fc->m_filterInstance, outBlock);
	return true;
}

void Filter::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);
	s >> MemberRef< const IFilter >(L"filter", m_filter);
}

	}
}
