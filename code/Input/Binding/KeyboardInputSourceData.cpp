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
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberEnum.h"
#include "Input/Binding/EnumKeys.h"
#include "Input/Binding/KeyboardInputSource.h"
#include "Input/Binding/KeyboardInputSourceData.h"

namespace traktor::input
{
	namespace
	{

class MemberDefaultControlOld : public MemberEnum< DefaultControl >
{
public:
	MemberDefaultControlOld(const wchar_t* const name, DefaultControl& ref)
	:	MemberEnum< DefaultControl >(name, ref, g_DefaultControl_Keys_Old)
	{
	}
};

class MemberDefaultControl : public MemberEnum< DefaultControl >
{
public:
	MemberDefaultControl(const wchar_t* const name, DefaultControl& ref)
	:	MemberEnum< DefaultControl >(name, ref, g_DefaultControl_Keys)
	{
	}
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.KeyboardInputSourceData", 1, KeyboardInputSourceData, IInputSourceData)

void KeyboardInputSourceData::addControlType(DefaultControl controlType)
{
	m_controlTypes.push_back(controlType);
}

const AlignedVector< DefaultControl >& KeyboardInputSourceData::getControlTypes() const
{
	return m_controlTypes;
}

Ref< IInputSource > KeyboardInputSourceData::createInstance(DeviceControlManager* deviceControlManager) const
{
	return new KeyboardInputSource(m_controlTypes, deviceControlManager);
}

void KeyboardInputSourceData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> MemberAlignedVector< DefaultControl, MemberDefaultControl >(L"controlTypes", m_controlTypes);
	else
		s >> MemberAlignedVector< DefaultControl, MemberDefaultControlOld >(L"controlTypes", m_controlTypes);
}

}
