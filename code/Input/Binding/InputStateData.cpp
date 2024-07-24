/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/IInputNode.h"
#include "Input/Binding/InputStateData.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputStateData", 0, InputStateData, ISerializable)

InputStateData::InputStateData(IInputNode* source)
:	m_source(source)
{
}

void InputStateData::setSource(IInputNode* source)
{
	m_source = source;
}

const IInputNode* InputStateData::getSource() const
{
	return m_source;
}

void InputStateData::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source, AttributePrivate());
}

}
