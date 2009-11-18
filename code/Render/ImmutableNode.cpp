#include "Render/ImmutableNode.h"

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

int ImmutableNode::getInputPinCount() const
{
	return int(m_inputPins.size());
}

Ref< const InputPin > ImmutableNode::getInputPin(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_inputPins.size()));
	return m_inputPins[index];
}

int ImmutableNode::getOutputPinCount() const
{
	return int(m_outputPins.size());
}

Ref< const OutputPin > ImmutableNode::getOutputPin(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_outputPins.size()));
	return m_outputPins[index];
}

	}
}
