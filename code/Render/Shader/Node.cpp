#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Node", Node, ISerializable)

Node::Node()
:	m_position(0, 0)
{
}

void Node::setComment(const std::wstring& comment)
{
	m_comment = comment;
}

const std::wstring& Node::getComment() const
{
	return m_comment;
}

std::wstring Node::getInformation() const
{
	return L"";
}

void Node::setPosition(const std::pair< int, int >& position)
{
	m_position = position;
}

const std::pair< int, int >& Node::getPosition() const
{
	return m_position;
}

const InputPin* Node::findInputPin(const std::wstring& name) const
{
	int count = getInputPinCount();
	for (int i = 0; i < count; ++i)
	{
		const InputPin* inputPin = getInputPin(i);
		T_ASSERT (inputPin);

		if (inputPin->getName() == name)
			return inputPin;
	}
	return 0;
}

const OutputPin* Node::findOutputPin(const std::wstring& name) const
{
	int count = getOutputPinCount();
	for (int i = 0; i < count; ++i)
	{
		const OutputPin* outputPin = getOutputPin(i);
		T_ASSERT (outputPin);

		if (outputPin->getName() == name)
			return outputPin;
	}
	return 0;
}

void Node::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"comment", m_comment, AttributeMultiLine());
	s >> MemberStlPair< int, int >(L"position", m_position);
}

	}
}
