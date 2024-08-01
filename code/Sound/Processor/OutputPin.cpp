/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Processor/OutputPin.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.OutputPin", OutputPin, Object)

OutputPin::OutputPin(Node* node, const std::wstring& name, NodePinType type)
:	m_node(node)
,	m_name(name)
,	m_type(type)
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

NodePinType OutputPin::getPinType() const
{
	return m_type;
}

}
