/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/ImmutableNode.h"

namespace traktor::render
{
	namespace
	{

const static Guid c_null;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImmutableNode", ImmutableNode, Node)

ImmutableNode::ImmutableNode(const InputPinDesc* inputPins, const OutputPinDesc* outputPins)
{
	if (inputPins)
	{
		int32_t pinCount = 0;
		for (auto pin = inputPins; pin->name; ++pin)
			++pinCount;

		if (pinCount > 0)
		{
			m_inputPins = new InputPin [pinCount];
			m_inputPinCount = pinCount;

			InputPin* ptr = m_inputPins;
			for (auto pin = inputPins; pin->name; ++pin)
				*ptr++ = InputPin(this, Guid(pin->id), pin->name, pin->optional);
		}
	}

	if (outputPins)
	{
		int32_t pinCount = 0;
		for (auto pin = outputPins; pin->name; ++pin)
			++pinCount;

		if (pinCount > 0)
		{
			m_outputPins = new OutputPin [pinCount];
			m_outputPinCount = pinCount;

			OutputPin* ptr = m_outputPins;
			for (auto pin = outputPins; pin->name; ++pin)
				*ptr++ = OutputPin(this, Guid(pin->id), pin->name); 
		}
	}
}

ImmutableNode::~ImmutableNode()
{
	delete[] m_inputPins;
	delete[] m_outputPins;
}

int ImmutableNode::getInputPinCount() const
{
	return m_inputPinCount;
}

const InputPin* ImmutableNode::getInputPin(int index) const
{
	T_ASSERT(index >= 0 && index < m_inputPinCount);
	return &m_inputPins[index];
}

int ImmutableNode::getOutputPinCount() const
{
	return m_outputPinCount;
}

const OutputPin* ImmutableNode::getOutputPin(int index) const
{
	T_ASSERT(index >= 0 && index < m_outputPinCount);
	return &m_outputPins[index];
}

}
