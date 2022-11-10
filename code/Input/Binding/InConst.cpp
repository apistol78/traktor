/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Input/Binding/InConst.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InConst", 0, InConst, IInputNode)

InConst::InConst()
:	m_value(0.0f)
{
}

InConst::InConst(float value)
:	m_value(value)
{
}

Ref< IInputNode::Instance > InConst::createInstance() const
{
	return 0;
}

float InConst::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	return m_value;
}

void InConst::serialize(ISerializer& s)
{
	s >> Member< float >(L"value", m_value);
}

	}
}
