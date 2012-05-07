#include "Render/Shader/TypedInputPin.h"

namespace traktor
{
	namespace render
	{

TypedInputPin::TypedInputPin(Node* node, const std::wstring& name, bool optional, ParameterType type)
:	InputPin(node, name, optional)
,	m_type(type)
{
}

ParameterType TypedInputPin::getType() const
{
	return m_type;
}

	}
}
