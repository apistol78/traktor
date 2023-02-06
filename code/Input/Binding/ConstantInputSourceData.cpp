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
#include "Input/Binding/ConstantInputSource.h"
#include "Input/Binding/ConstantInputSourceData.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.ConstantInputSourceData", 0, ConstantInputSourceData, IInputSourceData)

ConstantInputSourceData::ConstantInputSourceData(float value)
:	m_value(value)
{
}

Ref< IInputSource > ConstantInputSourceData::createInstance(DeviceControlManager* deviceControlManager) const
{
	return new ConstantInputSource(m_value);
}

void ConstantInputSourceData::serialize(ISerializer& s)
{
	s >> Member< float >(L"value", m_value);
}

}
