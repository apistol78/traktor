#include "Render/Editor/ImmutableNode.h"

namespace traktor
{
	namespace render
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

		T_ASSERT(pinCount > 0);
		m_inputPins.reserve(pinCount);

		InputPin* pins = new InputPin [pinCount];
		for (auto pin = inputPins; pin->name; ++pin)
		{
			*pins = InputPin(this, c_null, pin->name, pin->optional);
			m_inputPins.push_back(pins);
			++pins;
		}		
	}

	if (outputPins)
	{
		int32_t pinCount = 0;
		for (auto pin = outputPins; pin->name; ++pin)
			++pinCount;

		T_ASSERT(pinCount > 0);
		m_outputPins.reserve(pinCount);

		OutputPin* pins = new OutputPin [pinCount];
		for (auto pin = outputPins; pin->name; ++pin)
		{
			*pins = OutputPin(this, c_null, pin->name);
			m_outputPins.push_back(pins);
			++pins;
		}		
	}
}

ImmutableNode::~ImmutableNode()
{
	if (!m_inputPins.empty())
		delete[] m_inputPins.front();
	if (!m_outputPins.empty())
		delete[] m_outputPins.front();
}

int ImmutableNode::getInputPinCount() const
{
	return (int)m_inputPins.size();
}

const InputPin* ImmutableNode::getInputPin(int index) const
{
	T_ASSERT(index >= 0 && index < (int)m_inputPins.size());
	return m_inputPins[index];
}

int ImmutableNode::getOutputPinCount() const
{
	return (int)m_outputPins.size();
}

const OutputPin* ImmutableNode::getOutputPin(int index) const
{
	T_ASSERT(index >= 0 && index < (int)m_outputPins.size());
	return m_outputPins[index];
}

	}
}
