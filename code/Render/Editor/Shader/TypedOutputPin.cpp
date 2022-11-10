/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/TypedOutputPin.h"

namespace traktor
{
	namespace render
	{

TypedOutputPin::TypedOutputPin(Node* node, const Guid& id, const std::wstring& name, ParameterType type)
:	OutputPin(node, id, name)
,	m_type(type)
{
}

ParameterType TypedOutputPin::getType() const
{
	return m_type;
}

	}
}
