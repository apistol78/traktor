#include "Sound/Processor/OutputPin.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.OutputPin", OutputPin, Object)

OutputPin::OutputPin(Node* node, const std::wstring& name, NodePinType type)
:	m_node(node)
,	m_name(name)
,	m_type(type)
{
}

Node* OutputPin::getNode() const
{
	return m_node;
}

const std::wstring& OutputPin::getName() const
{
	return m_name;
}

NodePinType OutputPin::getPinType() const
{
	return m_type;
}

	}
}
