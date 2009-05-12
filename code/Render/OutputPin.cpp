#include <algorithm>
#include "Render/OutputPin.h"
#include "Render/Node.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.OutputPin", OutputPin, Object)

OutputPin::OutputPin()
{
}

OutputPin::OutputPin(Node* node, const std::wstring& name)
:	m_node(node)
,	m_name(name)
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

	}
}
