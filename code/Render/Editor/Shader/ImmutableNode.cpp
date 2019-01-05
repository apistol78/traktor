#include "Render/Editor/Shader/ImmutableNode.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImmutableNode", ImmutableNode, Node)

ImmutableNode::ImmutableNode(const InputPinDesc* inputPins, const OutputPinDesc* outputPins)
{
	const Guid c_null;
	while (inputPins && inputPins->name)
	{
		m_inputPins.push_back(new InputPin(this, c_null, inputPins->name, inputPins->optional));
		++inputPins;
	}
	while (outputPins && outputPins->name)
	{
		m_outputPins.push_back(new OutputPin(this, c_null, outputPins->name));
		++outputPins;
	}
}

ImmutableNode::~ImmutableNode()
{
	for (auto& inputPin : m_inputPins)
		delete inputPin;
	for (auto& outputPin : m_outputPins)
		delete outputPin;
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
