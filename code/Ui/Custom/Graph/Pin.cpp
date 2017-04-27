/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/Graph/Pin.h"
#include "Ui/Custom/Graph/Node.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Pin", Pin, Object)

Pin::Pin(Node* node, const std::wstring& name, Direction direction, bool mandatory)
:	m_node(node)
,	m_name(name)
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
}
