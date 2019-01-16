#include "Ui/Graph/Node.h"
#include "Ui/Graph/Pin.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Pin", Pin, Object)

Pin::Pin(Node* node, const std::wstring& name, const std::wstring& label, Direction direction, bool mandatory)
:	m_node(node)
,	m_name(name)
,	m_label(label)
,	m_direction(direction)
,	m_mandatory(mandatory)
{
}

Node* Pin::getNode() const
{
	return m_node;
}

const std::wstring& Pin::getName() const
{
	return m_name;
}

const std::wstring& Pin::getLabel() const
{
	return m_label;
}

Pin::Direction Pin::getDirection() const
{
	return m_direction;
}

bool Pin::isMandatory() const
{
	return m_mandatory;
}

Point Pin::getPosition() const
{
	return m_node->getPinPosition(this);
}

	}
}
