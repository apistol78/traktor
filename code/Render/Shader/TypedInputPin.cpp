#include "Render/Shader/TypedInputPin.h"

namespace traktor
{
	namespace render
	{

TypedInputPin::TypedInputPin(Node* node, const std::wstring& name, bool optional, ParameterType type, const std::wstring& samplerId)
:	InputPin(node, name, optional)
,	m_type(type)
,	m_samplerId(samplerId)
{
}

ParameterType TypedInputPin::getType() const
{
	return m_type;
}

const std::wstring& TypedInputPin::getSamplerId() const
{
	return m_samplerId;
}

	}
}
