/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"

namespace traktor
{
	namespace render
	{

OutputPin::OutputPin(Node* node, const Guid& id, const std::wstring& name)
:	m_node(node)
,	m_id(id)
,	m_name(name)
{
}

Node* OutputPin::getNode() const
{
	return m_node;
}

const Guid& OutputPin::getId() const
{
	return m_id;
}

const std::wstring& OutputPin::getName() const
{
	return m_name;
}

	}
}
