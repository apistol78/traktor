/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"

namespace traktor
{
	namespace render
	{

InputPin::InputPin(Node* node, const Guid& id, const std::wstring& name, bool optional)
:	m_node(node)
,	m_id(id)
,	m_name(name)
,	m_optional(optional)
{
}

Node* InputPin::getNode() const
{
	return m_node;
}

const Guid& InputPin::getId() const
{
	return m_id;
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
