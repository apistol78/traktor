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
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStl.h"
#include "Input/Binding/EnumKeys.h"
#include "Input/Binding/KeyboardInputSource.h"
#include "Input/Binding/KeyboardInputSourceData.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

class MemberInputDefaultControlType : public MemberEnum< InputDefaultControlType >
{
public:
	MemberInputDefaultControlType(const wchar_t* const name, InputDefaultControlType& ref)
	:	MemberEnum< InputDefaultControlType >(name, ref, g_InputDefaultControlType_Keys)
	{
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.KeyboardInputSourceData", 0, KeyboardInputSourceData, IInputSourceData)

void KeyboardInputSourceData::addControlType(InputDefaultControlType controlType)
{
	m_controlTypes.push_back(controlType);
}

const std::vector< InputDefaultControlType >& KeyboardInputSourceData::getControlTypes() const
{
	return m_controlTypes;
}

Ref< IInputSource > KeyboardInputSourceData::createInstance(DeviceControlManager* deviceControlManager) const
{
	return new KeyboardInputSource(m_controlTypes, deviceControlManager);
}

void KeyboardInputSourceData::serialize(ISerializer& s)
{
	s >> MemberStlVector< InputDefaultControlType, MemberInputDefaultControlType >(L"controlTypes", m_controlTypes);
}

	}
}
