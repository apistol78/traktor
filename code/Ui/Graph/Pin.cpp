/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Graph/Node.h"
#include "Ui/Graph/Pin.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Pin", Pin, Object)

Pin::Pin(Node* node, const std::wstring& name, const Guid& id, Direction direction, bool mandatory, bool bold)
:	m_node(node)
,	m_name(name)
,	m_id(id)
,	m_direction(direction)
,	m_mandatory(mandatory)
,	m_bold(bold)
{
}

Node* Pin::getNode() const
{
	return m_node;
}

void Pin::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Pin::getName() const
{
	return m_name;
}

const Guid& Pin::getId() const
{
	return m_id;
}

Pin::Direction Pin::getDirection() const
{
	return m_direction;
}

bool Pin::isMandatory() const
{
	return m_mandatory;
}

bool Pin::isBold() const
{
	return m_bold;
}

UnitPoint Pin::getPosition() const
{
	return m_node->getPinPosition(this);
}

}
