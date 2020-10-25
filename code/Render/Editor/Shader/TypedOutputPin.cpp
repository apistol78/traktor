#include "Render/Editor/Shader/TypedOutputPin.h"

namespace traktor
{
	namespace render
	{

TypedOutputPin::TypedOutputPin(Node* node, const Guid& id, const std::wstring& name, ParameterType type)
:	OutputPin(node, id, name)
,	m_type(type)
{
}

ParameterType TypedOutputPin::getType() const
{
	return m_type;
}

	}
}
