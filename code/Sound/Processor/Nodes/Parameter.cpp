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
#include "Sound/Processor/Nodes/Parameter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::OutputPinDesc c_Parameter_o[] =
{
	{ L"Output", NodePinType::Scalar },
	{ nullptr }
};

class ParameterCursor : public RefCountImpl< IAudioBufferCursor >
{
public:
	handle_t m_id;
	float m_value;

	ParameterCursor(handle_t id, float defaultValue)
	:	m_id(id)
	,	m_value(defaultValue)
	{
	}

	virtual void setParameter(handle_t id, float parameter) override final
	{
		if (id == m_id)
			m_value = parameter;
	}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Parameter", 1, Parameter, ImmutableNode)

Parameter::Parameter()
:	ImmutableNode(nullptr, c_Parameter_o)
{
}

bool Parameter::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< IAudioBufferCursor > Parameter::createCursor() const
{
	return new ParameterCursor(getParameterHandle(m_name), m_defaultValue);
}

bool Parameter::getScalar(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, float& outParameter) const
{
	ParameterCursor* parameterCursor = static_cast< ParameterCursor* >(cursor);
	outParameter = parameterCursor->m_value;
	return true;
}

bool Parameter::getBlock(IAudioBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, AudioBlock& outBlock) const
{
	return false;
}

void Parameter::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< std::wstring >(L"name", m_name);

	if (s.getVersion< Parameter >() >= 1)
		s >> Member< float >(L"defaultValue", m_defaultValue);
}

	}
}
