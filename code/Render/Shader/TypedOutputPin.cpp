#include "Render/Shader/TypedOutputPin.h"

namespace traktor
{
	namespace render
	{

TypedOutputPin::TypedOutputPin()
:	OutputPin()
,	m_type(PtScalar)
{
}

TypedOutputPin::TypedOutputPin(Node* node, const std::wstring& name, ParameterType type)
:	OutputPin(node, name)
,	m_type(type)
{
}

ParameterType TypedOutputPin::getType() const
{
	return m_type;
}

	}
}
