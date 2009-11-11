#include "Render/InputPin.h"
#include "Render/Node.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.InputPin", InputPin, Object)

InputPin::InputPin(Node* node, const std::wstring& name, bool optional)
:	m_node(node)
,	m_name(name)
,	m_optional(optional)
{
}

Ref< Node > InputPin::getNode() const
{
	return m_node;
}

const std::wstring& InputPin::getName() const
{
	return m_name;
}

bool InputPin::isOptional() const
{
	return m_optional;
}

	}
}
