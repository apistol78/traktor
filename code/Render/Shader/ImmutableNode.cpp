/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader/ImmutableNode.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImmutableNode", ImmutableNode, Node)

ImmutableNode::ImmutableNode(const InputPinDesc* inputPins, const OutputPinDesc* outputPins)
{
	while (inputPins && inputPins->name)
	{
		m_inputPins.push_back(new InputPin(this, inputPins->name, inputPins->optional));
		++inputPins;
	}
	while (outputPins && outputPins->name)
	{
		m_outputPins.push_back(new OutputPin(this, outputPins->name));
		++outputPins;
	}
}

ImmutableNode::~ImmutableNode()
{
	for (std::vector< InputPin* >::iterator i = m_inputPins.begin(); i != m_inputPins.end(); ++i)
		delete *i;
	for (std::vector< OutputPin* >::iterator i = m_outputPins.begin(); i != m_outputPins.end(); ++i)
		delete *i;
}

int ImmutableNode::getInputPinCount() const
{
	return int(m_inputPins.size());
}

const InputPin* ImmutableNode::getInputPin(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_inputPins.size()));
	return m_inputPins[index];
}

int ImmutableNode::getOutputPinCount() const
{
	return int(m_outputPins.size());
}

const OutputPin* ImmutableNode::getOutputPin(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_outputPins.size()));
	return m_outputPins[index];
}

	}
}
