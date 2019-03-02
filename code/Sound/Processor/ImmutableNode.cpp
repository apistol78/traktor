#include "Sound/Processor/ImmutableNode.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.ImmutableNode", ImmutableNode, Node)

ImmutableNode::ImmutableNode(const InputPinDesc* inputPins, const OutputPinDesc* outputPins)
{
	while (inputPins && inputPins->name)
	{
		m_inputPins.push_back(new InputPin(this, inputPins->name, inputPins->type, inputPins->optional));
		++inputPins;
	}
	while (outputPins && outputPins->name)
	{
		m_outputPins.push_back(new OutputPin(this, outputPins->name, outputPins->type));
		++outputPins;
	}
}

size_t ImmutableNode::getInputPinCount() const
{
	return m_inputPins.size();
}

const InputPin* ImmutableNode::getInputPin(size_t index) const
{
	return m_inputPins[index];
}

size_t ImmutableNode::getOutputPinCount() const
{
	return m_outputPins.size();
}

const OutputPin* ImmutableNode::getOutputPin(size_t index) const
{
	return m_outputPins[index];
}

	}
}
