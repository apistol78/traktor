/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Processor/InputPin.h"
#include "Sound/Processor/Node.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.InputPin", InputPin, Object)

InputPin::InputPin(Node* node, const std::wstring& name, NodePinType type, bool optional)
:	m_node(node)
,	m_name(name)
,	m_type(type)
,	m_optional(optional)
{
}

Node* InputPin::getNode() const
{
	return m_node;
}

const std::wstring& InputPin::getName() const
{
	return m_name;
}

NodePinType InputPin::getPinType() const
{
	return m_type;
}

bool InputPin::isOptional() const
{
	return m_optional;
}

	}
}
